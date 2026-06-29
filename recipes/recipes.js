(function () {
  const recipes = Array.isArray(window.BIT_PIRATE_RECIPES) ? window.BIT_PIRATE_RECIPES : [];
  const grid = document.querySelector("[data-recipe-grid]");
  const staticGrid = document.querySelector("[data-static-recipe-grid]");
  const empty = document.querySelector("[data-empty-state]");
  const selects = Array.from(document.querySelectorAll("[data-recipe-select]"));
  const count = document.querySelector("[data-recipe-count]");
  const searchInput = document.querySelector("[data-recipe-search]");
  const clearSearch = document.querySelector("[data-search-clear]");
  const activeFilters = {
    protocol: "All",
    task: "All",
    difficulty: "All"
  };
  let activeQuery = "";

  if (!grid) {
    return;
  }

  const normalize = (value) => String(value || "")
    .normalize("NFD")
    .replace(/[\u0300-\u036f]/g, "")
    .replace(/²/g, "2")
    .replace(/[-_/]/g, " ")
    .replace(/\s+/g, " ")
    .trim()
    .toLowerCase();

  const SEARCH_CONCEPTS = [
    { name: "i2c", kind: "protocol", aliases: ["i2c", "i2c bus", "twi"] },
    { name: "spi", kind: "protocol", aliases: ["spi", "serial peripheral interface"] },
    { name: "uart", kind: "protocol", aliases: ["uart", "serial", "console", "rs232"] },
    { name: "gpio", kind: "protocol", aliases: ["gpio", "dio", "pin", "pins"] },
    { name: "usb", kind: "protocol", aliases: ["usb", "web serial", "cdc", "hid"] },
    { name: "scan", kind: "action", aliases: ["scan", "discover", "detect", "identify", "find", "probe", "enumerate"] },
    { name: "read", kind: "action", aliases: ["read", "dump", "inspect", "view", "monitor", "trace"] },
    { name: "debug", kind: "action", aliases: ["debug", "diagnose", "diagnostic", "troubleshoot", "recover", "health"] },
    { name: "flash", kind: "action", aliases: ["flash", "firmware", "program", "write", "burn"] }
  ];

  const textIncludes = (text, value) => text.includes(normalize(value));
  const conceptMatches = (text, concept) => concept.aliases.some((alias) => textIncludes(text, alias));

  const getQueryConcepts = (query) => SEARCH_CONCEPTS.filter((concept) => (
    concept.aliases.some((alias) => textIncludes(query, alias))
  ));

  const getQueryWords = (query, concepts) => {
    let remainder = normalize(query);
    concepts.forEach((concept) => {
      concept.aliases.forEach((alias) => {
        remainder = remainder.replaceAll(normalize(alias), " ");
      });
    });
    return remainder.split(/\s+/).filter((word) => word.length > 1);
  };

  const TASK_FILTERS = {
    "find & diagnose": ["scan", "discover", "detect", "identify", "find", "probe", "check", "diagnos", "health", "recover", "status"],
    "read & inspect": ["read", "dump", "inspect", "monitor", "trace", "analyze", "view"],
    "flash & write": ["flash", "write", "program", "load", "configure", "set "],
    "capture & analyze": ["capture", "sniff", "record", "trace", "analyze", "sweep"],
    "connect & control": ["connect", "open", "bridge", "send", "receive", "transfer", "emulate", "test", "use "]
  };

  const PROTOCOL_FILTERS = {
    "i2c": ["i2c"],
    "spi": ["spi"],
    "uart": ["uart"],
    "hd uart": ["hduart", "half duplex uart"],
    "gpio dio": ["gpio", "dio"],
    "pwm servo": ["pwm", "servo"],
    "1 wire": ["1 wire", "one wire", "ibutton"],
    "2 wire sle4442": ["2wire", "smartcard", "sle4442"],
    "3 wire": ["3wire", "microwire"],
    "i2s": ["i2s"],
    "jtag swd": ["jtag", "swd"],
    "can": ["can", "mcp2515"],
    "usb": ["usb", "hid", "cdc", "msc"],
    "infrared": ["infrared", "ir toy", "lirc"],
    "sub ghz": ["subghz", "cc1101"],
    "wi fi": ["wi fi", "wifi"],
    "ble": ["ble", "bluetooth"],
    "rf24": ["rf24", "nrf24"],
    "fm rds": ["fm", "rds", "si4713"],
    "ethernet": ["ethernet", "w5500"],
    "cellular at": ["cell", "cellular", "gsm", "lte", "ussd", "modem"],
    "rfid": ["rfid", "pn532", "mifare", "ntag"],
    "led": ["led", "fastled"]
  };

  const recipeMatchesValue = (recipe, value, filterName) => {
    const selected = normalize(value);

    if (selected === "all") {
      return true;
    }

    const fields = [
      recipe.difficulty,
      recipe.type,
      ...(recipe.protocols || []),
      ...(recipe.targets || []),
      ...(recipe.workflows || []),
      ...(recipe.tools || []),
      ...(recipe.tags || [])
    ].map(normalize);

    if (filterName === "protocol") {
      const aliases = PROTOCOL_FILTERS[selected];
      return aliases
        ? aliases.some((alias) => fields.some((field) => field.includes(alias)))
        : fields.includes(selected);
    }

    if (filterName === "task") {
      return (TASK_FILTERS[selected] || []).some((term) => getSearchText(recipe).includes(term));
    }

    if (filterName === "difficulty") {
      return normalize(recipe.difficulty) === selected;
    }

    if (selected === "web tools") {
      return fields.some((field) => field.includes("web"));
    }

    if (selected === "cli") {
      return fields.some((field) => field.includes("cli"));
    }

    return fields.includes(selected);
  };

  const recipeMatchesFilters = (recipe) => (
    recipeMatchesValue(recipe, activeFilters.protocol, "protocol")
    && recipeMatchesValue(recipe, activeFilters.task, "task")
    && recipeMatchesValue(recipe, activeFilters.difficulty, "difficulty")
  );

  const getSearchText = (recipe) => normalize([
    recipe.title,
    recipe.description,
    recipe.difficulty,
    recipe.type,
    recipe.time,
    recipe.hardware,
    recipe.takeaway,
    ...(recipe.protocols || []),
    ...(recipe.targets || []),
    ...(recipe.workflows || []),
    ...(recipe.tools || []),
    ...(recipe.tags || [])
  ].join(" "));

  const getRecipeSearchScore = (recipe, query) => {
    const normalizedQuery = normalize(query);

    if (!normalizedQuery) {
      return 0;
    }

    const haystack = getSearchText(recipe);
    const title = normalize(recipe.title);
    const concepts = getQueryConcepts(normalizedQuery);
    const protocolConcepts = concepts.filter((concept) => concept.kind === "protocol");
    const actionConcepts = concepts.filter((concept) => concept.kind === "action");
    const words = getQueryWords(normalizedQuery, concepts);

    // A named protocol is the user's main scope. Keep every matching recipe,
    // then rank direct actions (scan, recover, read…) first within that scope.
    if (protocolConcepts.some((concept) => !conceptMatches(haystack, concept))) {
      return null;
    }

    if (words.some((word) => !haystack.includes(word))) {
      return null;
    }

    // With no protocol or free-text constraint, an action such as "scan" is
    // itself the search intent and must be present through one of its aliases.
    if (!protocolConcepts.length && !words.length && actionConcepts.length
      && actionConcepts.some((concept) => !conceptMatches(haystack, concept))) {
      return null;
    }

    let score = title.includes(normalizedQuery) ? 100 : 0;
    score += protocolConcepts.filter((concept) => conceptMatches(title, concept)).length * 24;
    score += actionConcepts.filter((concept) => conceptMatches(title, concept)).length * 18;
    score += actionConcepts.filter((concept) => conceptMatches(haystack, concept)).length * 8;
    score += words.filter((word) => title.includes(word)).length * 12;
    score += words.filter((word) => haystack.includes(word)).length * 4;
    return score;
  };

  const createPillList = (items, className) => {
    const list = document.createElement("ul");
    list.className = className;

    (items || []).forEach((item) => {
      const pill = document.createElement("li");
      pill.textContent = item;
      list.appendChild(pill);
    });

    return list;
  };

  const staticCardCount = staticGrid ? staticGrid.querySelectorAll(".recipe-card").length : 0;
  const canEnhanceIndex = !staticGrid || recipes.length >= staticCardCount;

  if (!canEnhanceIndex) {
    // Keep the static, crawlable cards visible if an older cached recipes-list.js
    // is loaded by the browser. This avoids hiding newly added recipes while
    // preserving basic search/filter behavior instead of making the toolbar inert.
    if (grid) {
      grid.hidden = true;
    }

    const staticCards = staticGrid ? Array.from(staticGrid.querySelectorAll(".recipe-card")) : [];
    const getStaticCardText = (card) => normalize(card.textContent || "");
    const staticMatchesProtocol = (text) => {
      const selected = normalize(activeFilters.protocol);
      if (selected === "all") {
        return true;
      }
      const aliases = PROTOCOL_FILTERS[selected];
      return aliases ? aliases.some((alias) => text.includes(alias)) : text.includes(selected);
    };
    const staticMatchesTask = (text) => {
      const selected = normalize(activeFilters.task);
      if (selected === "all") {
        return true;
      }
      return (TASK_FILTERS[selected] || []).some((term) => text.includes(term));
    };
    const staticMatchesDifficulty = (text) => {
      const selected = normalize(activeFilters.difficulty);
      return selected === "all" || text.includes(selected);
    };
    const staticMatchesSearch = (text) => {
      const words = normalize(activeQuery).split(/\s+/).filter(Boolean);
      return !words.length || words.every((word) => text.includes(word));
    };
    const renderStaticFallback = () => {
      let visibleCount = 0;
      staticCards.forEach((card) => {
        const text = getStaticCardText(card);
        const visible = staticMatchesSearch(text)
          && staticMatchesProtocol(text)
          && staticMatchesTask(text)
          && staticMatchesDifficulty(text);
        card.hidden = !visible;
        if (visible) {
          visibleCount += 1;
        }
      });
      if (empty) {
        empty.hidden = visibleCount > 0;
      }
      if (count) {
        count.textContent = `${visibleCount} recipe${visibleCount === 1 ? "" : "s"}`;
      }
      if (clearSearch) {
        clearSearch.hidden = activeQuery.length === 0;
      }
    };

    selects.forEach((select) => {
      select.addEventListener("change", () => {
        activeFilters[select.dataset.recipeSelect] = select.value;
        renderStaticFallback();
      });
    });

    if (searchInput) {
      searchInput.addEventListener("input", () => {
        activeQuery = searchInput.value;
        renderStaticFallback();
      });
    }

    if (clearSearch && searchInput) {
      clearSearch.addEventListener("click", () => {
        searchInput.value = "";
        activeQuery = "";
        searchInput.focus();
        renderStaticFallback();
      });
    }

    renderStaticFallback();
    return;
  }

  if (staticGrid) {
    staticGrid.hidden = true;
  }

  const createCard = (recipe) => {
    const article = document.createElement("article");
    article.className = "recipe-card";

    const link = document.createElement("a");
    link.className = "recipe-card-link";
    link.href = `${recipe.slug}/`;
    link.setAttribute("aria-label", recipe.title);

    if (recipe.cover) {
      const image = document.createElement("img");
      image.className = "recipe-cover";
      image.src = recipe.cover;
      image.alt = "";
      image.width = 520;
      image.height = 300;
      image.loading = "lazy";
      link.appendChild(image);
    }

    const eyebrow = document.createElement("p");
    eyebrow.className = "eyebrow";
    const protocol = (recipe.protocols || [recipe.type])[0] || recipe.type;
    const mark = recipe.mark || ">_";
    const markSpan = document.createElement("span");
    markSpan.className = "eyebrow-text-mark";
    markSpan.setAttribute("aria-hidden", "true");
    markSpan.textContent = mark;
    eyebrow.appendChild(markSpan);
    eyebrow.appendChild(document.createTextNode(recipe.difficulty || ""));
    link.appendChild(eyebrow);

    const title = document.createElement("h2");
    title.textContent = recipe.title;
    link.appendChild(title);

    const description = document.createElement("p");
    description.className = "recipe-description";
    description.textContent = recipe.takeaway || recipe.description;
    link.appendChild(description);

    const meta = document.createElement("div");
    meta.className = "recipe-meta";
    const primaryTool = (recipe.tools || [])[0];
    const quickFacts = [
      primaryTool === "Serial CLI" ? "Any CLI" : primaryTool,
      recipe.time,
    ].filter(Boolean);

    if (quickFacts.length) {
      meta.appendChild(createPillList(quickFacts, "recipe-pill-list recipe-facts-list"));
    }

    link.appendChild(meta);

    const footerParts = [recipe.date, recipe.author || "Geo"].filter(Boolean);
    if (footerParts.length) {
      const footer = document.createElement("p");
      footer.className = "recipe-date";
      footer.textContent = footerParts.join(" · ");
      link.appendChild(footer);
    }

    article.appendChild(link);
    return article;
  };

  const render = () => {
    const visible = recipes
      .map((recipe) => ({ recipe, score: getRecipeSearchScore(recipe, activeQuery) }))
      .filter(({ recipe, score }) => recipeMatchesFilters(recipe) && score !== null)
      .sort((a, b) => b.score - a.score || a.recipe.title.localeCompare(b.recipe.title))
      .map(({ recipe }) => recipe);
    grid.replaceChildren(...visible.map(createCard));

    if (empty) {
      empty.hidden = visible.length > 0;
    }

    if (count) {
      count.textContent = `${visible.length} recipe${visible.length === 1 ? "" : "s"}`;
    }

    if (clearSearch) {
      clearSearch.hidden = activeQuery.length === 0;
    }
  };

  selects.forEach((select) => {
    select.addEventListener("change", () => {
      activeFilters[select.dataset.recipeSelect] = select.value;
      render();
    });
  });

  if (searchInput) {
    searchInput.addEventListener("input", () => {
      activeQuery = searchInput.value;
      render();
    });
  }

  if (clearSearch && searchInput) {
    clearSearch.addEventListener("click", () => {
      searchInput.value = "";
      activeQuery = "";
      searchInput.focus();
      render();
    });
  }

  render();

  if (searchInput && !searchInput.value) {
    window.setTimeout(() => {
      try {
        searchInput.focus({ preventScroll: true });
      } catch (error) {
        searchInput.focus();
      }
    }, 0);
  }
})();

(function () {
  const icons = {
    "Recipes": '<svg class="article-nav-icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M5 4.5h11a3 3 0 0 1 3 3v12H8a3 3 0 0 0-3 3z"></path><path d="M5 4.5v18M8 9h7M8 13h5"></path></svg>',
    "Web Tools": '<svg class="article-nav-icon" viewBox="0 0 24 24" aria-hidden="true"><rect x="3" y="4" width="18" height="15" rx="2"></rect><path d="M3 8h18M7 6h.01M10 6h.01"></path></svg>',
    "Wiki": '<svg class="article-nav-icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M5 5.5A2.5 2.5 0 0 1 7.5 3H19v16H7.5A2.5 2.5 0 0 0 5 21.5z"></path><path d="M5 5.5v16M9 7h6M9 11h6"></path></svg>'
  };

  document.querySelectorAll(".article-top-nav a").forEach((link) => {
    const icon = icons[link.textContent.trim()];
    if (icon && !link.querySelector(".article-nav-icon")) {
      link.insertAdjacentHTML("afterbegin", icon);
    }
  });
})();

(function () {
  const path = window.location.pathname;
  const isButtonExitBridge = [
    "/recipes/bridge-half-duplex-uart/",
    "/recipes/open-hduart-bridge-session/"
  ].some((suffix) => path.endsWith(suffix));

  if (!isButtonExitBridge) {
    return;
  }

  const list = document.querySelector(".troubleshooting-section .mistake-list");
  if (!list) {
    return;
  }

  const item = document.createElement("li");
  item.textContent = "Terminal input is forwarded to the target while bridging. Press any physical button on the ESP32 to leave bridge mode.";
  list.appendChild(item);
})();

(function () {
  const wiringLists = Array.from(document.querySelectorAll(".wiring-list"));
  const article = document.querySelector(".recipe-article");
  const svgNS = "http://www.w3.org/2000/svg";

  if (!wiringLists.length || !article || document.querySelector(".wiring-visual-section")) {
    return;
  }

  const splitWire = (text) => {
    const normalized = String(text || "").replace(/\s+/g, " ").trim();
    const arrow = normalized.includes("→") ? "→" : "->";
    const parts = normalized.split(arrow).map((part) => part.trim());

    if (parts.length < 2 || !parts[0] || !parts[1]) {
      return null;
    }

    return {
      from: parts[0],
      to: parts.slice(1).join(` ${arrow} `)
    };
  };

  const shortLabel = (label) => {
    const cleaned = String(label || "")
      .replace(/\bconfigured\b/gi, "")
      .replace(/\bselected\b/gi, "")
      .replace(/\bESP32 Bit Pirate\b/gi, "Bit Pirate")
      .replace(/\bBit Pirate\b/gi, "BP")
      .replace(/\bTarget board\b/gi, "Target")
      .replace(/\bTarget\b/gi, "")
      .replace(/\bin the order selected\b/gi, "")
      .replace(/\bshown as\b/gi, "as")
      .replace(/\s+/g, " ")
      .trim();

    return cleaned.length > 18 ? `${cleaned.slice(0, 15)}...` : cleaned;
  };

  const makeSvgEl = (tag, attrs = {}) => {
    const element = document.createElementNS(svgNS, tag);
    Object.entries(attrs).forEach(([name, value]) => {
      element.setAttribute(name, value);
    });
    return element;
  };

  const hasPhysicalPins = (text) => /\b(CS|CS#|CLK|SCK|MISO|MOSI|SDA|SCL|TX|RX|IO|GPIO|GND|VCC|VIN|3\.3V|5V|CANH|CANL|DIN|DOUT|DO|DI|SK|RST|RESET|EN|BOOT|IRQ|INT|DATA|SIG|PWM|LED|IR|GDO|CE|CSN|MOSI|MISO|SO|SI|SCLK|TCK|TMS|TDI|TDO|SWDIO|SWCLK)\b/i.test(text);

  const isUsefulWiring = (list, wires) => {
    if (wires.length < 2) {
      return false;
    }

    const text = [
      list.dataset.wiringLeft,
      list.dataset.wiringRight,
      ...wires.flatMap((wire) => [wire.from, wire.to])
    ].filter(Boolean).join(" ");

    if (/\b(Network|Endpoint|URL|API|HTTP|WebSocket|Wi-Fi|Ethernet|LAN|Host)\b/i.test(text) && !hasPhysicalPins(text)) {
      return false;
    }

    if (/Signal pins/i.test(text) && /configured Bit Pirate pins/i.test(text) && wires.length <= 2) {
      return false;
    }

    return hasPhysicalPins(text);
  };

  const getWires = (list) => Array.from(list.querySelectorAll("li"))
    .map((item) => splitWire(item.textContent))
    .filter(Boolean)
    .slice(0, 8);

  const selected = wiringLists
    .map((list) => ({ list, wires: getWires(list) }))
    .find(({ list, wires }) => isUsefulWiring(list, wires));

  if (!selected) {
    return;
  }

  const makeDiagram = (list, wires) => {
    const diagram = document.createElement("figure");
    diagram.className = "wiring-diagram wiring-diagram-main";

    const svg = makeSvgEl("svg", {
      viewBox: "0 0 920 320",
      role: "img",
      "aria-label": "Generated wiring diagram"
    });

    svg.appendChild(makeSvgEl("rect", {
      class: "wiring-node wiring-node-left",
      x: "34",
      y: "48",
      width: "220",
      height: "224",
      rx: "8"
    }));
    svg.appendChild(makeSvgEl("rect", {
      class: "wiring-node wiring-node-right",
      x: "666",
      y: "48",
      width: "220",
      height: "224",
      rx: "8"
    }));

    const leftTitle = makeSvgEl("text", { class: "wiring-title", x: "144", y: "86", "text-anchor": "middle" });
    leftTitle.textContent = shortLabel(list.dataset.wiringLeft || "Target");
    svg.appendChild(leftTitle);

    const rightTitle = makeSvgEl("text", { class: "wiring-title", x: "776", y: "86", "text-anchor": "middle" });
    rightTitle.textContent = shortLabel(list.dataset.wiringRight || "BP");
    svg.appendChild(rightTitle);

    wires.forEach((wire, index) => {
      const y = 122 + (index * 22);
      const hueClass = `wire-${index % 5}`;

      svg.appendChild(makeSvgEl("line", {
        class: `wiring-line ${hueClass}`,
        x1: "254",
        y1: String(y),
        x2: "666",
        y2: String(y)
      }));

      const leftPin = makeSvgEl("text", { class: "wiring-pin", x: "232", y: String(y + 4), "text-anchor": "end" });
      leftPin.textContent = shortLabel(wire.from);
      svg.appendChild(leftPin);

      const rightPin = makeSvgEl("text", { class: "wiring-pin", x: "688", y: String(y + 4) });
      rightPin.textContent = shortLabel(wire.to);
      svg.appendChild(rightPin);
    });

    const mobileList = document.createElement("div");
    mobileList.className = "wiring-mobile-list";
    mobileList.setAttribute("aria-hidden", "true");
    wires.forEach((wire, index) => {
      const row = document.createElement("div");
      row.className = `wiring-mobile-row wire-${index % 5}`;

      const from = document.createElement("span");
      from.textContent = shortLabel(wire.from);

      const arrow = document.createElement("span");
      arrow.className = "wiring-mobile-arrow";
      arrow.textContent = "->";

      const to = document.createElement("span");
      to.textContent = shortLabel(wire.to);

      row.appendChild(from);
      row.appendChild(arrow);
      row.appendChild(to);
      mobileList.appendChild(row);
    });

    const caption = document.createElement("figcaption");
    caption.textContent = `Generated from the wiring summary: ${shortLabel(list.dataset.wiringLeft || "Target")} to ${shortLabel(list.dataset.wiringRight || "BP")}.`;

    diagram.appendChild(svg);
    diagram.appendChild(mobileList);
    diagram.appendChild(caption);
    return diagram;
  };

  const section = document.createElement("section");
  section.className = "recipe-section wiring-visual-section";
  section.setAttribute("aria-label", "Generated wiring diagram");
  section.appendChild(makeDiagram(selected.list, selected.wires));

  article.insertAdjacentElement("afterbegin", section);
})();

(function () {
  const main = document.querySelector("main.wrap");

  if (!main || document.querySelector(".site-footer")) {
    return;
  }

  const footer = document.createElement("footer");
  footer.className = "site-footer";

  const title = document.createElement("p");
  title.className = "site-footer-title";
  title.textContent = "ESP32 Bit Pirate";

  const links = document.createElement("nav");
  links.setAttribute("aria-label", "Project links");

  const isRecipeArticle = /\/recipes\/[^/]+\/?$/.test(window.location.pathname);
  const webToolsHref = isRecipeArticle ? "../../web-tools/" : "../web-tools/";

  [
    ["GitHub", "https://github.com/geo-tp/ESP32-Bit-Pirate"],
    ["Wiki", "https://github.com/geo-tp/ESP32-Bit-Pirate/wiki"],
    ["Web Tools", webToolsHref]
  ].forEach(([label, href]) => {
    const link = document.createElement("a");
    link.href = href;
    link.textContent = label;

    if (href.startsWith("https://")) {
      link.target = "_blank";
      link.rel = "noopener";
    }

    links.appendChild(link);
  });

  footer.appendChild(title);
  footer.appendChild(links);
  main.appendChild(footer);
})();

(function () {
  const article = document.querySelector(".recipe-article");

  if (!article) {
    return;
  }

  const base = "../../";
  const toolLinks = [
    ["Web Serial Terminal", "web-tools/web-serial-terminal/"],
    ["Web Logic Analyzer", "web-tools/logic-analyzer/"],
    ["SPI Flash Programmer", "web-tools/spi-flash-programmer/"],
    ["AVR Programmer", "web-tools/avr-programmer/"],
    ["STM32 Tool", "web-tools/stm32-tool/"],
    ["ESP Tool", "web-tools/esp-tool/"],
    ["Web Flasher", "webflasher/"],
    ["BPIO2", "web-tools/bpio2/"]
  ];
  const pattern = new RegExp(`(${toolLinks.map(([name]) => name.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")).join("|")})`, "gi");
  const walker = document.createTreeWalker(article, NodeFilter.SHOW_TEXT);
  const textNodes = [];

  while (walker.nextNode()) {
    const node = walker.currentNode;
    const parent = node.parentElement;

    if (node.nodeValue.trim() && parent && !parent.closest("a, code, script, style, h1, h2, h3, h4, h5, h6, summary, button")) {
      textNodes.push(node);
    }
  }

  textNodes.forEach((node) => {
    const value = node.nodeValue;
    pattern.lastIndex = 0;

    if (!pattern.test(value)) {
      return;
    }

    const fragment = document.createDocumentFragment();
    let lastIndex = 0;
    pattern.lastIndex = 0;
    let match;

    while ((match = pattern.exec(value))) {
      fragment.append(value.slice(lastIndex, match.index));
      const link = document.createElement("a");
      const tool = toolLinks.find(([name]) => name.toLowerCase() === match[0].toLowerCase());
      link.href = `${base}${tool[1]}`;
      link.textContent = match[0];
      link.setAttribute("aria-label", `Open ${match[0]}`);
      fragment.append(link);
      lastIndex = match.index + match[0].length;
    }

    fragment.append(value.slice(lastIndex));
    node.replaceWith(fragment);
  });
})();

(function () {
  const article = document.querySelector(".recipe-article");

  if (article && article.querySelectorAll(".step-label").length === 1) {
    article.classList.add("has-single-step");
  }
})();

(function () {
  const sessions = Array.from(document.querySelectorAll("[data-cli-session]"));

  if (!sessions.length) {
    return;
  }

  const splitLines = (value) => String(value || "")
    .split("\n")
    .map((line) => line.trim())
    .filter(Boolean);

  const promptAfterMode = (command) => {
    const match = String(command || "").match(/^mode\s+(.+)$/i);

    if (!match) {
      return null;
    }

    return match[1].trim().toUpperCase();
  };

  const commandPromptLines = (mode, commands, explicitPrompts) => {
    if (explicitPrompts.length) {
      return commands.map((command, index) => `${explicitPrompts[index] || explicitPrompts[explicitPrompts.length - 1]}> ${command}`);
    }

    if (mode === "BitPirate") {
      let prompt = "HiZ";
      return commands.map((command) => {
        const line = `${prompt}> ${command}`;
        const nextPrompt = promptAfterMode(command);

        if (nextPrompt) {
          prompt = nextPrompt;
        }

        return line;
      });
    }

    return commands.map((command) => `${mode}> ${command}`);
  };

  sessions.forEach((session) => {
    const mode = session.dataset.cliMode || "BitPirate";
    const commands = splitLines(session.dataset.cliCommands);
    const result = splitLines(session.dataset.cliResult);
    const explicitPrompts = splitLines(session.dataset.cliPrompts);

    const windowEl = document.createElement("div");
    windowEl.className = "cli-window";

    const chrome = document.createElement("div");
    chrome.className = "cli-window-chrome";
    chrome.innerHTML = '<span></span><span></span><span></span>';
    const copyButton = document.createElement("button");
    copyButton.className = "cli-copy";
    copyButton.type = "button";
    copyButton.textContent = "Copy";
    copyButton.setAttribute("aria-label", "Copy commands");
    chrome.appendChild(copyButton);
    windowEl.appendChild(chrome);

    const body = document.createElement("pre");
    body.className = "cli-window-body";
    const code = document.createElement("code");

    const lines = commandPromptLines(mode, commands, explicitPrompts);
    result.forEach((line) => {
      lines.push(`  ${line}`);
    });

    code.textContent = lines.join("\n");
    body.appendChild(code);
    windowEl.appendChild(body);
    session.replaceChildren(windowEl);

    copyButton.addEventListener("click", async () => {
      const text = commands.join("\n");
      try {
        await navigator.clipboard.writeText(text);
        copyButton.textContent = "Copied";
        window.setTimeout(() => {
          copyButton.textContent = "Copy";
        }, 1400);
      } catch (error) {
        copyButton.textContent = "Select";
        code.focus();
      }
    });
  });
})();
