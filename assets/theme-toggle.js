(() => {
  const storageKey = "esp32-bit-pirate-theme";
  const root = document.documentElement;

  const applyTheme = (theme) => {
    root.dataset.theme = theme;
    try {
      localStorage.setItem(storageKey, theme);
    } catch (_) {
      // The visual switch still works when storage is unavailable.
    }
  };

  let theme = "dark";
  try {
    theme = localStorage.getItem(storageKey) || "dark";
  } catch (_) {
    // Dark is the default for private or restricted browser contexts.
  }
  applyTheme(theme === "light" ? "light" : "dark");

  const style = document.createElement("style");
  style.textContent = `
    .theme-toggle { display:inline-flex; align-items:center; gap:5px; min-height:34px; padding:5px; border:1px solid #2f3a2f; border-radius:999px; background:#111; color:#d8fff2; cursor:pointer; font:700 .72rem/1 Menlo,"Courier New",monospace; }
    .theme-toggle:hover, .theme-toggle:focus-visible { border-color:#00ffcc88; color:#b8ffb8; }
    .theme-toggle-icon { display:grid; width:22px; height:22px; place-items:center; border-radius:50%; color:currentColor; }
    .theme-toggle-icon svg { width:13px; height:13px; fill:none; stroke:currentColor; stroke-linecap:round; stroke-linejoin:round; stroke-width:1.8; }
    .theme-toggle[data-theme="light"] .theme-toggle-icon--sun, .theme-toggle[data-theme="dark"] .theme-toggle-icon--moon { background:#1f3a32; color:#b8ffb8; }
    .theme-toolbar { display:flex; justify-content:flex-end; margin:0 0 14px; }
    .theme-nav-cluster { display:flex; align-items:center; gap:8px; margin-left:auto; }
    html[data-theme="light"] { color-scheme:light; --page-bg:#f4f7f5; --panel-bg:#ffffff; --panel-bg-deep:#e7eee9; --border:#b9c9c0; --text:#183128; --muted:#52665c; --green:#078653; --green-dim:#087345; --cyan:#007f7a; }
    html[data-theme="light"] body { background:#f4f7f5 !important; color:#183128 !important; }
    html[data-theme="light"] :is(.hero-copy,.content-card,.system-card,.recipe-card,.tool-card,.protocol-card,.faq-grid article,.faq-item,.support-card,.card,.tip-card,.notice,.modal,.config-panel,.serial-panel,.settings-panel,.recipe-search,.recipe-toolbar,.hero-terminal,.brand-mark,.site-nav a,.page-top-nav a,.article-top-nav a,.theme-toggle,.recipe-pill-list li,.tag-list li,.article-hero-tags li,.filter-button,.recipe-section,.callout,.empty-state,.wiring-mobile-row,.wiring-node,.link-card-list a,.hardware-list li,.wiring-list li,.setup-list li,.audience-list li,.external-links-card,.external-link-grid a,.capability-grid article,.compatibility-card,.link-list a,.authority-card) { background:#fff !important; border-color:#b9c9c0 !important; box-shadow:inset 0 0 5px rgba(28,57,43,.08) !important; }
    html[data-theme="light"] :is(h1,h2,h3,h4,h5,h6,.brand,.card-arrow,.button,.eyebrow,.card-eyebrow,.article-hero-tags li) { color:#087345 !important; }
    html[data-theme="light"] :is(p,li,dd,dt,label,summary,.lead,.section-intro,.muted,.recipe-date,.wiring-pin,.wiring-mobile-row span,.audience-list strong,.external-links-card p,.external-link-grid a,.link-list a) { color:#355448; }
    html[data-theme="light"] :is(pre,code,.cli-window,.terminal-body,.ascii-title) { color:#123e2e; }
    html[data-theme="light"] .cli-window, html[data-theme="light"] .terminal-body { background:#ecf4ef !important; border-color:#b9c9c0 !important; }
    html[data-theme="light"] .cli-window { box-shadow:0 8px 22px rgba(31,72,53,.08), inset 0 0 7px rgba(28,57,43,.06) !important; }
    html[data-theme="light"] .cli-window-chrome { background:#e3eee7 !important; border-color:#b9c9c0 !important; }
    html[data-theme="light"] .cli-window-body, html[data-theme="light"] .cli-window-body code { background:#ecf4ef !important; color:#123e2e !important; }
    html[data-theme="light"] .cli-copy { background:#fff !important; border-color:#9db9aa !important; color:#17463a !important; box-shadow:none !important; }
    html[data-theme="light"] .recipe-section.troubleshooting-section { background:linear-gradient(145deg,#fff7f7,#fff) !important; border-color:#d8aaad !important; box-shadow:0 8px 22px rgba(116,47,51,.08), inset 0 0 5px rgba(116,47,51,.04) !important; }
    html[data-theme="light"] .recipe-section.troubleshooting-section h2 { color:#a63d45 !important; }
    html[data-theme="light"] .recipe-section.troubleshooting-section code { color:#7a272d !important; }
    html[data-theme="light"] .mistake-list li { background:#fff !important; border-color:#d8aaad !important; color:#63383b !important; box-shadow:inset 0 0 4px rgba(116,47,51,.04) !important; }
    html[data-theme="light"] .mistake-list li::before, html[data-theme="light"] .recipe-section.troubleshooting-section .section-title-icon { background:#fff0f0 !important; border-color:#d8aaad !important; color:#a63d45 !important; box-shadow:none !important; }
    html[data-theme="light"] .next-steps-section > :is(ol,ul) > li { background:#f8fcf9 !important; border-color:#b9c9c0 !important; color:#355448 !important; box-shadow:inset 0 0 5px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .next-steps-section > :is(ol,ul) > li::before { background:#e6f1ea !important; border-color:#a8c5b7 !important; color:#087345 !important; box-shadow:none !important; }
    html[data-theme="light"] .next-steps-section li code { background:#edf7f1 !important; border-color:#a8c5b7 !important; color:#075f3b !important; text-shadow:none !important; }
    html[data-theme="light"] .aside-card, html[data-theme="light"] .aside-card.recipe-side-panel { background:linear-gradient(145deg,#fff,#edf4ef) !important; border-color:#b9c9c0 !important; box-shadow:0 10px 26px rgba(31,72,53,.09), inset 0 0 5px rgba(28,57,43,.06) !important; }
    html[data-theme="light"] .aside-card dt { color:#087345 !important; }
    html[data-theme="light"] .aside-card dt::before { background:#e6f1ea !important; border-color:#a8c5b7 !important; color:#087345 !important; box-shadow:none !important; }
    html[data-theme="light"] .aside-card dd, html[data-theme="light"] .aside-card dd.aside-setup-dd { background:#f8fcf9 !important; border-color:#c6d5cd !important; color:#355448 !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .aside-card :is(.aside-setup-list li,dd.aside-setup-dd ul li) { background:#fff !important; border-color:#c6d5cd !important; color:#355448 !important; box-shadow:none !important; }
    html[data-theme="light"] .aside-card dd.aside-setup-dd p { color:#355448 !important; }
    html[data-theme="light"] .aside-card dd code { background:#edf7f1 !important; border-color:#a8c5b7 !important; color:#075f3b !important; }
    html[data-theme="light"] .aside-card dd a { color:#075f3b !important; }
    html[data-theme="light"] .wiring-visual-section { background:radial-gradient(circle at 10% 0%, rgba(0,127,122,.09), transparent 18rem), linear-gradient(145deg,#fff,#edf4ef) !important; border-color:#a8c5b7 !important; box-shadow:0 10px 26px rgba(31,72,53,.08), inset 0 0 5px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .wiring-diagram { background:linear-gradient(90deg, rgba(0,127,122,.10) 1px, transparent 1px), linear-gradient(rgba(0,127,122,.08) 1px, transparent 1px), linear-gradient(145deg,#f9fcfa,#e7f0ea) !important; background-size:28px 28px,28px 28px,auto !important; border-color:#a8c5b7 !important; box-shadow:inset 0 0 14px rgba(28,57,43,.08) !important; }
    html[data-theme="light"] .wiring-node { fill:#fff !important; stroke:#82a997 !important; filter:drop-shadow(0 6px 12px rgba(31,72,53,.12)) !important; }
    html[data-theme="light"] .wiring-pin { fill:#24493b !important; }
    html[data-theme="light"] .wiring-line.wire-4 { stroke:#356a5a !important; }
    html[data-theme="light"] .wiring-diagram figcaption { background:#f8fcf9 !important; border-color:#b9c9c0 !important; color:#52665c !important; }
    html[data-theme="light"] .wiring-mobile-row { background:#fff !important; border-color:#b9c9c0 !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .wiring-mobile-row span { color:#24493b !important; }
    html[data-theme="light"] .link-card-list a :is(span,strong) { color:#24493b !important; }
    html[data-theme="light"] .link-card-list a span { color:#087345 !important; }
    html[data-theme="light"] :is(.recipe-banner,.recipe-image-break) { background:#fff !important; border-color:#b9c9c0 !important; box-shadow:0 12px 28px rgba(31,72,53,.11), inset 0 0 5px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.recipe-banner,.recipe-image-break) figcaption { background:#f8fcf9 !important; border-color:#b9c9c0 !important; color:#52665c !important; }
    html[data-theme="light"] :is(input,select,textarea,.recipe-search-bar,.recipe-search-clear,.cli-window-chrome,.wiring-diagram,.recipe-banner) { background:#f8fbf9 !important; border-color:#b9c9c0 !important; color:#183128 !important; }
    html[data-theme="light"] .recipe-search-input { background:linear-gradient(145deg,#fff,#f1f7f3) !important; border-color:#a8c5b7 !important; box-shadow:inset 0 0 8px rgba(28,57,43,.06), 0 8px 22px rgba(31,72,53,.08) !important; }
    html[data-theme="light"] .recipe-search-input::placeholder { color:#6b8176 !important; }
    html[data-theme="light"] .recipe-search-input:focus { border-color:#5b9c7d !important; box-shadow:inset 0 0 8px rgba(28,57,43,.06), 0 0 0 3px rgba(7,134,83,.13) !important; }
    html[data-theme="light"] .wiring-node { fill:#fff !important; stroke:#b9c9c0 !important; }
    html[data-theme="light"] .audience-list li::before { background:#e6f1ea !important; border-color:#a8c5b7 !important; color:#087345 !important; box-shadow:none !important; }
    html[data-theme="light"] :is(.protocols li,.pill-list li,.protocol-list li,.brand-signature-keywords li,.site-footer a,.inline-links a) { background:#fff !important; border-color:#b9c9c0 !important; color:#17463a !important; box-shadow:inset 0 0 5px rgba(28,57,43,.08) !important; }
    html[data-theme="light"] :is(.site-nav a,.page-top-nav a,.article-top-nav a) { color:#17463a !important; }
    html[data-theme="light"] :is(.button,.button.primary,.button.secondary-button) { background:#eef6f1 !important; border-color:#9db9aa !important; color:#17463a !important; box-shadow:inset 0 0 5px rgba(28,57,43,.08) !important; }
    html[data-theme="light"] .brand-signature-card { background:radial-gradient(circle at 14% 50%, rgba(0,127,122,.10), transparent 16rem), linear-gradient(145deg,#fff,#e8f0eb) !important; border-color:#a8c5b7 !important; box-shadow:inset 0 0 5px rgba(28,57,43,.08) !important; }
    html[data-theme="light"] :is(.hero-copy,.hero-terminal,.content-card,.system-card,.recipe-card,.tool-card,.protocol-card,.faq-grid article,.faq-item,.support-card,.card,.tip-card,.notice,.compatibility-card,.external-links-card,.authority-card) { box-shadow:0 10px 26px rgba(31,72,53,.09), inset 0 0 5px rgba(28,57,43,.06) !important; }
    html[data-theme="light"] :is([class*="panel"],[class*="card"],[class*="modal"],[class*="dialog"],[class*="console"],[class*="output"],[class*="terminal"],[class*="log"],[class*="status"],[class*="inspector"],[class*="workspace"],[class*="sidebar"],[class*="table-wrap"],[class*="settings"],[class*="controls"]) { background-color:#fff !important; border-color:#b9c9c0 !important; color:#183128 !important; box-shadow:0 8px 22px rgba(31,72,53,.08), inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is([class*="console"],[class*="output"],[class*="terminal"],[class*="log"]) { background-color:#f3f8f5 !important; }
    html[data-theme="light"] :is([class*="warning"],[class*="caution"]) { background:#fff7e6 !important; border-color:#dcc38b !important; color:#634a10 !important; }
    html[data-theme="light"] :is([class*="error"],[class*="danger"]) { background:#fff0f0 !important; border-color:#d7a1a1 !important; color:#7a2727 !important; }
    html[data-theme="light"] :is(.mode-tabs,.operation-tabs,.tabs,.tab-list,[role="tablist"]) { background:#edf4ef !important; border-color:#b9c9c0 !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.mode-tab,.operation-tab,.tab-button,[role="tab"]) { background:#fff !important; border-color:#b9c9c0 !important; color:#24493b !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.mode-tab,.operation-tab,.tab-button,[role="tab"]):hover,
    html[data-theme="light"] :is(.mode-tab,.operation-tab,.tab-button,[role="tab"]):focus-visible { background:#f3faf6 !important; border-color:#7eb79d !important; color:#075f3b !important; }
    html[data-theme="light"] :is(.mode-tab.is-active,.operation-tab.is-active,.tab-button.is-active,[role="tab"][aria-selected="true"]) { background:#def0e5 !important; border-color:#78ad91 !important; color:#087345 !important; box-shadow:0 8px 18px rgba(7,115,69,.10), inset 0 0 4px rgba(255,255,255,.55) !important; }
    html[data-theme="light"] :is(.mode-tab.danger-tab.is-active,.operation-tab.danger-tab.is-active,[role="tab"].danger-tab[aria-selected="true"]) { background:#ffe8e8 !important; border-color:#d49a9a !important; color:#8a2424 !important; }
    html[data-theme="light"] :is(.mode-tab,.operation-tab,.tab-button,[role="tab"]) :is(svg,.tab-icon) { color:currentColor !important; stroke:currentColor !important; }
    html[data-theme="light"] :is(.toolbar,.toolbar-brand,.toolbar-actions,.drop-zone,.file-picker,.memory-operation-info,.memory-operation-info > div,.progress-row,.boot-help,.boot-mode-select,.toolbar-field,#bootModeHint) { background:#f7fbf8 !important; border-color:#b9c9c0 !important; color:#24493b !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.drop-zone,.file-picker) { background:#eef7f1 !important; border-style:dashed !important; color:#17463a !important; }
    html[data-theme="light"] progress { background:#e3eee7 !important; color:#087345 !important; }
    html[data-theme="light"] progress::-webkit-progress-bar { background:#e3eee7 !important; }
    html[data-theme="light"] progress::-webkit-progress-value { background:linear-gradient(90deg,#087345,#007f7a) !important; }
    html[data-theme="light"] progress::-moz-progress-bar { background:linear-gradient(90deg,#087345,#007f7a) !important; }
    html[data-theme="light"] :is(.toolbar-actions button,.config-button,.terminal-controls button,.terminal-controls select,.help-icon-button,.info-list > div,.target-help,.hex-preview,.estimate-cell strong,.memory-visual,.file-table,.file-table th,.file-table td,.file-table-wrap,.remove-row,.remove-file-row,.remove-step) { background:#f7fbf8 !important; border-color:#b9c9c0 !important; color:#24493b !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.file-table th,.file-table thead,.hex-preview) { background:#e7f1eb !important; color:#17463a !important; }
    html[data-theme="light"] :is([class*="auto-detect"],[class*="detect-card"],[class*="gpio"],[class*="pin-"],[class*="bus-config"],[class*="speed-control"],[class*="inner-panel"],[class*="device-list"],[class*="sequence-builder"],[class*="sequence-editor"],[class*="sequence-list"],[class*="sequence-item"]) { background:#f7fbf8 !important; border-color:#b9c9c0 !important; color:#24493b !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.segment-list,.memory-map,.memory-track,.memory-visual,.metric-grid > div,.info-dashboard article,.gpio-logic-trace,.gpio-logic-trace > div,.result-output,.log-output) { background:#f7fbf8 !important; border-color:#b9c9c0 !important; color:#24493b !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.file-label,.icon-button,.device-status-grid > div,.memory-operation-info > div,.progress-track,.sequence-step,.step-number) { background:#f7fbf8 !important; border-color:#b9c9c0 !important; color:#24493b !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] :is(.file-label,.icon-button) { color:#17463a !important; }
    html[data-theme="light"] :is(.device-status-grid strong,.memory-operation-info strong,.sequence-step strong) { color:#087345 !important; }
    html[data-theme="light"] :is(.device-status-grid span,.memory-operation-info span,.sequence-step small) { color:#52665c !important; }
    html[data-theme="light"] .progress-track { background:#e3eee7 !important; }
    html[data-theme="light"] .gpio-logic-trace svg { background:#f7fbf8 !important; border-color:#b9c9c0 !important; }
    html[data-theme="light"] .gpio-logic-trace :is(.logic-guide,.logic-wave) { stroke:#28725c !important; }
    html[data-theme="light"] .operation-note { background:#f8fcf9 !important; border-color:#b9c9c0 !important; color:#355448 !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .erase-info { background:#fff7f7 !important; border-color:#d8aaad !important; box-shadow:inset 0 0 4px rgba(116,47,51,.04) !important; }
    html[data-theme="light"] .erase-info > div { background:#fff !important; border-color:#d8aaad !important; color:#63383b !important; box-shadow:none !important; }
    html[data-theme="light"] .erase-info :is(dt,dd) { color:#63383b !important; }
    html[data-theme="light"] .danger-message { background:#fff0f0 !important; border-color:#d49a9a !important; color:#6d262b !important; box-shadow:inset 0 0 4px rgba(116,47,51,.05) !important; }
    html[data-theme="light"] .danger-message strong { color:#7a2027 !important; }
    html[data-theme="light"] .danger-message span { color:#6d3c40 !important; }
    html[data-theme="light"] .step-label { background:#e6f1ea !important; border-color:#a8c5b7 !important; color:#087345 !important; box-shadow:inset 0 0 4px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .recipe-section > ol > li,
    html[data-theme="light"] .recipe-section > ul:not(.hardware-list):not(.wiring-list):not(.mistake-list):not(.recipe-badge-list):not(.tag-list):not(.recipe-pill-list) > li { background:linear-gradient(90deg,rgba(0,127,122,.07),transparent 38%),#f8fcf9 !important; border-color:#b9c9c0 !important; color:#355448 !important; box-shadow:inset 0 0 6px rgba(28,57,43,.05) !important; }
    html[data-theme="light"] .recipe-section > ol > li::before,
    html[data-theme="light"] .recipe-section > ul:not(.hardware-list):not(.wiring-list):not(.mistake-list):not(.recipe-badge-list):not(.tag-list):not(.recipe-pill-list) > li::before { background:#e6f1ea !important; border-color:#a8c5b7 !important; color:#087345 !important; box-shadow:none !important; }
    html[data-theme="light"] :is(.segment-list.empty,.memory-map,.memory-track) { background:#edf5f0 !important; }
    html[data-theme="light"] :is(.metric-grid strong,.info-dashboard strong,.gpio-logic-trace strong) { color:#087345 !important; }
    html[data-theme="light"] .modal-overlay { background:rgba(24,57,43,.28) !important; }
    html[data-theme="light"] :is(.brand-signature-copy p,.site-footer-title,.terminal-topbar,.prompt-list li) { color:#24493b !important; }
    html[data-theme="light"] .terminal-topbar { background:#e3eee7 !important; border-color:#b9c9c0 !important; }

    /* Light artificial veil for image-heavy cards in light theme.
       The illustrations stay adapted to white cards without being washed out. */
    html[data-theme="light"] :is(.card-cover,.recipe-cover,.tool-image,.authority-image,.brand-signature-logo,.recipe-banner img,.recipe-image-break img) {
      background:#fff !important;
      opacity:.88 !important;
      filter:saturate(.94) contrast(.97) brightness(1.04) !important;
      box-shadow:none !important;
    }
    html[data-theme="light"] :is(.system-card-link,.content-card-link,.recipe-card-link,.tool-card > a,.authority-card,.brand-signature-card,.recipe-banner,.recipe-image-break) {
      background:#fff !important;
    }
    html[data-theme="light"] :is(.system-card-link,.content-card-link,.recipe-card-link,.tool-card > a):hover :is(.card-cover,.recipe-cover,.tool-image),
    html[data-theme="light"] :is(.system-card-link,.content-card-link,.recipe-card-link,.tool-card > a):focus-visible :is(.card-cover,.recipe-cover,.tool-image) {
      opacity:.94 !important;
      filter:saturate(.96) contrast(.98) brightness(1.02) !important;
    }
    html[data-theme="light"] .theme-toggle { color:#17463a; }
  `;
  document.head.appendChild(style);

  const button = document.createElement("button");
  button.className = "theme-toggle";
  button.type = "button";
  button.setAttribute("aria-label", "Toggle light theme");
  button.innerHTML = '<span class="theme-toggle-icon theme-toggle-icon--moon" aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M20 14.5A8.5 8.5 0 0 1 9.5 4 8.5 8.5 0 1 0 20 14.5z"></path></svg></span><span class="theme-toggle-icon theme-toggle-icon--sun" aria-hidden="true"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="3.5"></circle><path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4"></path></svg></span>';

  const syncButton = () => {
    const current = root.dataset.theme || "dark";
    button.dataset.theme = current;
    button.setAttribute("aria-pressed", String(current === "light"));
    button.setAttribute("aria-label", current === "light" ? "Switch to dark theme" : "Switch to light theme");
  };
  syncButton();

  button.addEventListener("click", () => {
    applyTheme(root.dataset.theme === "light" ? "dark" : "light");
    syncButton();
  });

  const toolToolbarActions = document.querySelector(".toolbar-actions");
  if (toolToolbarActions) {
    toolToolbarActions.appendChild(button);
    return;
  }

  const contextualNav = document.querySelector(".article-top-nav, .page-top-nav");
  if (contextualNav) {
    const cluster = document.createElement("div");
    cluster.className = "theme-nav-cluster";
    contextualNav.parentNode.insertBefore(cluster, contextualNav);
    cluster.append(contextualNav, button);
    return;
  }

  const mount = document.querySelector(".site-nav");

  if (mount) {
    mount.appendChild(button);
    return;
  }

  const toolbar = document.createElement("div");
  toolbar.className = "theme-toolbar";
  toolbar.appendChild(button);
  (document.querySelector(".hero") || document.querySelector(".wrap") || document.body).prepend(toolbar);
})();
