(() => {
  "use strict";

  const section = document.querySelector("[data-supported-devices]");
  const scene = section?.querySelector("[data-supported-devices-scene]");
  const copy = section?.querySelector("[data-supported-devices-copy]");
  const deviceElements = section
    ? Array.from(section.querySelectorAll("[data-physics-device]"))
    : [];
  const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)");

  // The untouched HTML/CSS grid is the intentional fallback for either case.
  if (!section || !scene || !copy || !deviceElements.length || reducedMotion.matches || !window.Matter) {
    return;
  }

  const {
    Bodies,
    Body,
    Composite,
    Constraint,
    Engine,
    Sleeping,
    Vector,
  } = window.Matter;

  const engine = Engine.create({ enableSleeping: true });
  engine.gravity.y = 0.9;

  const world = engine.world;
  let sceneInset = 24;
  let copyBounds = { right: 0, bottom: 0 };
  let layoutKey = "";
  const devices = [];
  let boundaries = [];
  let activeDrag = null;
  let animationFrame = 0;
  let resizeFrame = 0;
  let lastFrameTime = 0;
  let isNearViewport = false;
  let initialized = false;
  let keyboardNavigation = false;

  const suppressNextClick = new WeakSet();

  const renderDevices = () => {
    devices.forEach(({ body, element, width, height }) => {
      const x = body.position.x - width / 2;
      const y = body.position.y - height / 2;
      element.style.transform = `translate3d(${x}px, ${y}px, 0) rotate(${body.angle}rad)`;
    });
  };

  const confineDeviceToScene = (device, width = scene.clientWidth, height = scene.clientHeight) => {
    const { body } = device;
    const maxSpeed = 24;
    const speed = Vector.magnitude(body.velocity);

    if (speed > maxSpeed) {
      Body.setVelocity(body, Vector.mult(body.velocity, maxSpeed / speed));
    }

    let translateX = 0;
    let translateY = 0;

    if (body.bounds.min.x < sceneInset) {
      translateX = sceneInset - body.bounds.min.x;
    } else if (body.bounds.max.x > width - sceneInset) {
      translateX = width - sceneInset - body.bounds.max.x;
    }

    const top = body.bounds.min.x < copyBounds.right + 12
      ? Math.max(sceneInset, copyBounds.bottom + 12)
      : sceneInset;
    if (body.bounds.min.y < top) {
      translateY = top - body.bounds.min.y;
    } else if (body.bounds.max.y > height - sceneInset) {
      translateY = height - sceneInset - body.bounds.max.y;
    }

    if (!translateX && !translateY) {
      return;
    }

    Body.translate(body, { x: translateX, y: translateY });
    Body.setVelocity(body, {
      x: translateX ? 0 : body.velocity.x,
      y: translateY ? 0 : body.velocity.y,
    });
  };

  // Let each card fall, then gently settle it around its own landing position.
  // Neighbours still collide and can be dragged, but labels never finish sideways.
  const guideDevice = (device) => {
    const { body, home } = device;
    if (body.isStatic || activeDrag?.device === device) return;

    const dx = home.x - body.position.x;
    const dy = home.y - body.position.y;
    const da = home.angle - body.angle;
    if (body.isSleeping) {
      if (Math.hypot(dx, dy) < 1 && Math.abs(da) < 0.01) return;
      Sleeping.set(body, false);
    }
    if (body.position.y >= home.y - 12) device.landing = true;

    if (device.landing && Math.hypot(dx, dy) < 0.6 && body.speed < 0.15 && Math.abs(da) < 0.008) {
      Body.setPosition(body, { x: home.x, y: home.y });
      Body.setAngle(body, home.angle);
      Body.setVelocity(body, { x: 0, y: 0 });
      Body.setAngularVelocity(body, 0);
      Sleeping.set(body, true);
      return;
    }

    const mass = body.mass;
    Body.applyForce(body, body.position, {
      x: mass * (dx * 0.00012 - body.velocity.x * 0.00055),
      y: device.landing
        ? mass * (dy * 0.00012 - body.velocity.y * 0.00055 - engine.gravity.y * engine.gravity.scale)
        : 0,
    });
    Body.setAngularVelocity(body, body.angularVelocity * 0.55 + da * 0.13);
  };

  const stopLoop = () => {
    if (animationFrame) {
      window.cancelAnimationFrame(animationFrame);
      animationFrame = 0;
    }
    lastFrameTime = 0;
  };

  const runFrame = (time) => {
    if (!isNearViewport || document.hidden) {
      stopLoop();
      return;
    }

    const elapsed = lastFrameTime ? Math.min(time - lastFrameTime, 1000 / 30) : 1000 / 60;
    lastFrameTime = time;
    devices.forEach(guideDevice);
    Engine.update(engine, elapsed);
    devices.forEach((device) => confineDeviceToScene(device));
    renderDevices();

    const settled = !activeDrag && devices.every(({ body }) => body.isSleeping || body.isStatic);
    if (settled) {
      stopLoop();
      return;
    }

    animationFrame = window.requestAnimationFrame(runFrame);
  };

  const startLoop = () => {
    if (!initialized || !isNearViewport || document.hidden || reducedMotion.matches || animationFrame) {
      return;
    }
    lastFrameTime = 0;
    animationFrame = window.requestAnimationFrame(runFrame);
  };

  const wakeDevice = (body) => {
    if (!body.isStatic) {
      Sleeping.set(body, false);
    }
    startLoop();
  };

  const replaceBoundaries = (width, height) => {
    if (boundaries.length) {
      Composite.remove(world, boundaries);
    }

    const thickness = 160;
    const boundaryOptions = {
      isStatic: true,
      friction: 0.1,
      restitution: 0.25,
      label: "supported-devices-boundary",
    };

    boundaries = [
      Bodies.rectangle(width / 2, height - sceneInset + thickness / 2, width + thickness * 2, thickness, boundaryOptions),
      Bodies.rectangle(sceneInset - thickness / 2, height / 2, thickness, height + thickness * 2, boundaryOptions),
      Bodies.rectangle(width - sceneInset + thickness / 2, height / 2, thickness, height + thickness * 2, boundaryOptions),
      Bodies.rectangle(width / 2, sceneInset - thickness / 2, width + thickness * 2, thickness, boundaryOptions),
    ];
    Composite.add(world, boundaries);
  };

  const measureLayout = () => {
    const width = scene.clientWidth;
    const compact = width <= 700;
    sceneInset = compact ? 14 : 24;
    const gap = compact ? 12 : 16;
    const availableWidth = width - sceneInset * 2;
    scene.style.removeProperty("--physics-cell-width");
    const widest = Math.max(...deviceElements.map((element) => element.offsetWidth));
    // Three columns keep the mobile gallery compact without shrinking its labels.
    const columns = compact ? 3 : Math.max(3, Math.floor((availableWidth + gap) / (widest + gap)));
    const cellWidth = (availableWidth - gap * (columns - 1)) / columns;
    scene.style.setProperty("--physics-cell-width", `${cellWidth}px`);
    const measurements = deviceElements.map((element) => ({
      element,
      width: element.offsetWidth,
      height: element.offsetHeight,
    }));
    const sceneRect = scene.getBoundingClientRect();
    const copyRect = copy.getBoundingClientRect();
    copyBounds = {
      right: copyRect.right - sceneRect.left,
      bottom: copyRect.bottom - sceneRect.top,
    };
    const firstRowCount = measurements.length % columns || columns;
    const firstRowLeft = sceneInset + (columns - firstRowCount) * (cellWidth + gap);
    const besideCopy = firstRowLeft >= copyBounds.right + 20;
    let rowTop = besideCopy ? sceneInset + 16 : copyBounds.bottom + 56;
    let index = 0;
    let row = 0;
    while (index < measurements.length) {
      const count = row === 0 ? firstRowCount : Math.min(columns, measurements.length - index);
      const rowHeight = Math.max(...measurements.slice(index, index + count).map(({ height }) => height));
      for (let item = 0; item < count; item++, index++) {
        const measurement = measurements[index];
        const column = columns - count + item;
        const angle = (((index * 13) % 7) - 3) * (compact ? 0.009 : 0.013);
        const halfWidth = (measurement.width * Math.cos(angle) + measurement.height * Math.abs(Math.sin(angle))) / 2;
        const x = sceneInset + column * (cellWidth + gap) + cellWidth / 2 + ((index * 7) % 5 - 2);
        measurement.home = {
          x: Math.max(sceneInset + halfWidth + 2, Math.min(width - sceneInset - halfWidth - 2, x)),
          y: rowTop + rowHeight / 2 + ((index * 11) % 7 - 3),
          angle,
        };
      }
      rowTop += rowHeight + gap;
      row++;
    }
    const height = Math.ceil(rowTop - gap + sceneInset + 8);
    scene.style.setProperty("--physics-scene-height", `${height}px`);
    const key = JSON.stringify([width, height, copyBounds, measurements.map(({ width, height }) => [width, height])]);
    return { width, height, measurements, key };
  };

  const resizePhysics = () => {
    if (reducedMotion.matches) return;
    const layout = measureLayout();
    if (layout.key === layoutKey) {
      renderDevices();
      return;
    }
    layoutKey = layout.key;
    finishDrag();

    devices.forEach((device, index) => {
      const next = layout.measurements[index];
      Body.setAngle(device.body, 0);
      if (next.width !== device.width || next.height !== device.height) {
        Body.scale(device.body, next.width / device.width, next.height / device.height);
      }
      Object.assign(device, next, { landing: true });
      Body.setPosition(device.body, device.home);
      Body.setAngle(device.body, device.home.angle);
      Body.setVelocity(device.body, { x: 0, y: 0 });
      Body.setAngularVelocity(device.body, 0);
      Sleeping.set(device.body, true);
    });
    replaceBoundaries(layout.width, layout.height);
    renderDevices();
  };

  const queueResize = () => {
    if (resizeFrame) {
      window.cancelAnimationFrame(resizeFrame);
    }
    resizeFrame = window.requestAnimationFrame(() => {
      resizeFrame = 0;
      resizePhysics();
    });
  };

  const pointInScene = (event) => {
    const rect = scene.getBoundingClientRect();
    return {
      x: Math.min(Math.max(event.clientX - rect.left, sceneInset), rect.width - sceneInset),
      y: Math.min(Math.max(event.clientY - rect.top, sceneInset), rect.height - sceneInset),
    };
  };

  const finishDrag = (event) => {
    if (!activeDrag || (event && event.pointerId !== activeDrag.pointerId)) {
      return;
    }

    const { constraint, device, dragged, element, pointerId } = activeDrag;
    const velocityAge = event ? event.timeStamp - activeDrag.lastMoveTime : Infinity;
    const throwVelocity = activeDrag.throwVelocity;
    activeDrag = null;
    Composite.remove(world, constraint);
    element.classList.remove("is-dragging");

    if (dragged && event?.type !== "pointercancel" && velocityAge < 80) {
      const maxThrowSpeed = 10;
      const speed = Vector.magnitude(throwVelocity);
      const scale = speed > maxThrowSpeed ? maxThrowSpeed / speed : 1;
      Body.setVelocity(device.body, Vector.mult(throwVelocity, scale));
    }

    device.landing = true;
    confineDeviceToScene(device);

    if (element.hasPointerCapture?.(pointerId)) {
      element.releasePointerCapture(pointerId);
    }

    if (dragged) {
      suppressNextClick.add(element);
      window.setTimeout(() => suppressNextClick.delete(element), 500);
    }

    wakeDevice(device.body);
  };

  const beginDrag = (event, device) => {
    if (reducedMotion.matches || !event.isPrimary || event.button !== 0 || activeDrag || device.body.isStatic) {
      return;
    }

    keyboardNavigation = false;
    const pointer = pointInScene(event);
    const localGrabPoint = Vector.rotate(Vector.sub(pointer, device.body.position), -device.body.angle);
    const constraint = Constraint.create({
      pointA: pointer,
      bodyB: device.body,
      pointB: localGrabPoint,
      stiffness: 0.18,
      damping: 0.12,
      length: 0,
      render: { visible: false },
      label: "supported-devices-drag",
    });

    activeDrag = {
      constraint,
      device,
      element: device.element,
      pointerId: event.pointerId,
      startX: event.clientX,
      startY: event.clientY,
      lastPointer: pointer,
      lastMoveTime: event.timeStamp,
      throwVelocity: { x: 0, y: 0 },
      dragged: false,
    };

    Composite.add(world, constraint);
    Sleeping.set(device.body, false);
    device.element.setPointerCapture?.(event.pointerId);
    startLoop();
  };

  const moveDrag = (event) => {
    if (!activeDrag || event.pointerId !== activeDrag.pointerId) {
      return;
    }

    const distance = Math.hypot(event.clientX - activeDrag.startX, event.clientY - activeDrag.startY);
    if (distance > 7 && !activeDrag.dragged) {
      activeDrag.dragged = true;
      activeDrag.element.classList.add("is-dragging");
    }

    const pointer = pointInScene(event);
    const elapsed = Math.max(event.timeStamp - activeDrag.lastMoveTime, 1);
    const frameScale = (1000 / 60) / elapsed;
    const pointerVelocity = Vector.mult(Vector.sub(pointer, activeDrag.lastPointer), frameScale);
    activeDrag.throwVelocity = Vector.add(
      Vector.mult(activeDrag.throwVelocity, 0.35),
      Vector.mult(pointerVelocity, 0.65),
    );
    activeDrag.lastPointer = pointer;
    activeDrag.lastMoveTime = event.timeStamp;
    activeDrag.constraint.pointA.x = pointer.x;
    activeDrag.constraint.pointA.y = pointer.y;
    Sleeping.set(activeDrag.device.body, false);

    if (activeDrag.dragged && event.cancelable) {
      event.preventDefault();
    }
    startLoop();
  };

  const addDeviceInteraction = (device) => {
    const { element, body } = device;

    element.addEventListener("pointerdown", (event) => beginDrag(event, device));
    element.addEventListener("pointermove", moveDrag);
    element.addEventListener("pointerup", finishDrag);
    element.addEventListener("pointercancel", finishDrag);
    element.addEventListener("lostpointercapture", finishDrag);
    element.addEventListener("dragstart", (event) => event.preventDefault());

    element.addEventListener("click", (event) => {
      if (suppressNextClick.has(element)) {
        suppressNextClick.delete(element);
        event.preventDefault();
        event.stopPropagation();
      }
    });

    element.addEventListener("focus", () => {
      if (!keyboardNavigation || activeDrag?.element === element) {
        return;
      }
      device.keyboardLocked = true;
      Body.setStatic(body, true);
      renderDevices();
    });

    element.addEventListener("blur", () => {
      if (!device.keyboardLocked) {
        return;
      }
      device.keyboardLocked = false;
      Body.setStatic(body, false);
      Body.setVelocity(body, { x: 0, y: 0 });
      wakeDevice(body);
    });
  };

  const createDevices = (layout) => {
    layout.measurements.forEach((measurement, index) => {
      const { home, width, height } = measurement;
      const body = Bodies.rectangle(home.x, home.y - 52 - (index % 3) * 12, width, height, {
        angle: home.angle + ((index % 3) - 1) * 0.055,
        chamfer: { radius: 8 },
        density: 0.001,
        friction: 0.18,
        frictionAir: 0.035,
        restitution: 0.2,
        sleepThreshold: 55,
        label: measurement.element.textContent.trim(),
      });
      const device = { ...measurement, body, landing: false, keyboardLocked: false };
      devices.push(device);
      Composite.add(world, body);
      confineDeviceToScene(device, layout.width, layout.height);
      addDeviceInteraction(device);
    });
  };

  const initialize = () => {
    if (initialized) {
      return;
    }

    initialized = true;
    section.classList.add("physics-enabled");
    const layout = measureLayout();
    layoutKey = layout.key;
    replaceBoundaries(layout.width, layout.height);
    createDevices(layout);
    renderDevices();

    if ("ResizeObserver" in window) {
      const resizeObserver = new ResizeObserver(queueResize);
      resizeObserver.observe(scene);
      resizeObserver.observe(copy);
    } else {
      window.addEventListener("resize", queueResize, { passive: true });
    }

    startLoop();
  };

  document.addEventListener("keydown", (event) => {
    if (event.key === "Tab") {
      keyboardNavigation = true;
    }
  }, true);

  document.addEventListener("pointerdown", () => {
    keyboardNavigation = false;
  }, true);

  document.addEventListener("visibilitychange", () => {
    if (document.hidden) {
      stopLoop();
    } else {
      startLoop();
    }
  });

  reducedMotion.addEventListener?.("change", ({ matches }) => {
    if (matches) {
      finishDrag();
      stopLoop();
      section.classList.remove("physics-enabled");
      deviceElements.forEach((element) => element.style.removeProperty("transform"));
      return;
    }

    section.classList.add("physics-enabled");
    queueResize();
    startLoop();
  });

  if ("IntersectionObserver" in window) {
    const viewportObserver = new IntersectionObserver(([entry]) => {
      isNearViewport = entry.isIntersecting;
      if (isNearViewport) {
        initialize();
        startLoop();
      } else {
        stopLoop();
      }
    }, {
      rootMargin: "0px",
      threshold: 0.3,
    });
    viewportObserver.observe(section);
    initialize();
  } else {
    isNearViewport = true;
    initialize();
  }
})();
