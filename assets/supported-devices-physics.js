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
  const sceneInset = 24;
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

    if (body.bounds.min.y < sceneInset) {
      translateY = sceneInset - body.bounds.min.y;
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

  const resizePhysics = () => {
    if (reducedMotion.matches) {
      return;
    }

    const width = scene.clientWidth;
    const height = scene.clientHeight;
    if (!width || !height) {
      return;
    }

    devices.forEach((device) => {
      const nextWidth = device.element.offsetWidth;
      const nextHeight = device.element.offsetHeight;
      if (nextWidth !== device.width || nextHeight !== device.height) {
        Body.scale(device.body, nextWidth / device.width, nextHeight / device.height);
        device.width = nextWidth;
        device.height = nextHeight;
      }
      confineDeviceToScene(device, width, height);
      wakeDevice(device.body);
    });

    replaceBoundaries(width, height);
    renderDevices();
    startLoop();
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
      const maxThrowSpeed = 18;
      const speed = Vector.magnitude(throwVelocity);
      const scale = speed > maxThrowSpeed ? maxThrowSpeed / speed : 1;
      Body.setVelocity(device.body, Vector.mult(throwVelocity, scale));
    }

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

  const createDevices = () => {
    const sceneWidth = scene.clientWidth;
    const sceneHeight = scene.clientHeight;
    const measurements = deviceElements.map((element) => ({
      element,
      width: element.offsetWidth,
      height: element.offsetHeight,
    }));
    const widest = Math.max(...measurements.map(({ width }) => width));
    const columns = Math.max(2, Math.floor((sceneWidth - 24) / (widest + 10)));
    const cellWidth = (sceneWidth - 24) / columns;

    measurements.forEach((measurement, index) => {
      const column = index % columns;
      const row = Math.floor(index / columns);
      const itemsInColumn = Math.floor((measurements.length - 1 - column) / columns) + 1;
      const x = 12 + cellWidth * (column + 0.5);
      const firstY = measurement.height / 2 + sceneInset;
      const lastY = Math.max(firstY, sceneHeight - measurement.height / 2 - sceneInset);
      const availableStep = itemsInColumn > 1 ? (lastY - firstY) / (itemsInColumn - 1) : 0;
      const step = Math.min(measurement.height + 14, availableStep);
      const y = Math.min(firstY + row * step, lastY);
      const angle = (((index * 37) % 11) - 5) * 0.035;
      const body = Bodies.rectangle(x, y, measurement.width, measurement.height, {
        angle,
        chamfer: { radius: 8 },
        density: 0.001,
        friction: 0.08,
        frictionAir: 0.018,
        restitution: 0.34,
        sleepThreshold: 55,
        label: measurement.element.textContent.trim(),
      });
      const device = { ...measurement, body, keyboardLocked: false };

      devices.push(device);
      Composite.add(world, body);
      addDeviceInteraction(device);
    });
  };

  const initialize = () => {
    if (initialized) {
      return;
    }

    initialized = true;
    section.classList.add("physics-enabled");
    replaceBoundaries(scene.clientWidth, scene.clientHeight);
    createDevices();
    renderDevices();

    if ("ResizeObserver" in window) {
      const resizeObserver = new ResizeObserver(queueResize);
      resizeObserver.observe(scene);
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
