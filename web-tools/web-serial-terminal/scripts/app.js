import { SerialTerminal } from "./serial-terminal.js";
import { WebSerialConnection } from "./web-serial.js";

const elements = {
  terminal: document.querySelector("#terminal"),
  compatibilityBanner: document.querySelector("#compatibilityBanner"),
  connectionStatus: document.querySelector("#connectionStatus"),
  themeSelect: document.querySelector("#themeSelect"),
  fontDecreaseButton: document.querySelector("#fontDecreaseButton"),
  fontIncreaseButton: document.querySelector("#fontIncreaseButton"),
  configButton: document.querySelector("#configButton"),
  configOverlay: document.querySelector("#configOverlay"),
  configPanel: document.querySelector("#configPanel"),
  configCloseButton: document.querySelector("#configCloseButton"),
  baudRateInput: document.querySelector("#baudRateInput"),
  dataBitsSelect: document.querySelector("#dataBitsSelect"),
  stopBitsSelect: document.querySelector("#stopBitsSelect"),
  paritySelect: document.querySelector("#paritySelect"),
  flowControlSelect: document.querySelector("#flowControlSelect"),
  enterModeSelect: document.querySelector("#enterModeSelect"),
  localEchoCheckbox: document.querySelector("#localEchoCheckbox"),
  connectButton: document.querySelector("#connectButton"),
  clearButton: document.querySelector("#clearButton"),
  downloadButton: document.querySelector("#downloadButton")
};

const SERIAL_BAUD_RATE = 115200;
const THEME_STORAGE_KEY = "bit_pirate_webserial_theme";
const FONT_SIZE_STORAGE_KEY = "bit_pirate_webserial_font_size";
const SERIAL_SETTINGS_STORAGE_KEY = "bit_pirate_webserial_serial_settings";
const DEFAULT_SERIAL_SETTINGS = Object.freeze({
  baudRate: SERIAL_BAUD_RATE,
  dataBits: 8,
  stopBits: 1,
  parity: "none",
  flowControl: "none",
  enterMode: "cr",
  localEcho: false
});
const WELCOME_TEXT =
`   ____  _ _     ____  _           _
  | __ )(_) |_  |  _ \\(_)_ __ __ _| |_ ___
  |  _ \\| | __| | |_) | | '__/ _\` | __/ _ \\
  | |_) | | |_  |  __/| | | | (_| | ||  __/
  |____/|_|\\__| |_|   |_|_|  \\__,_|\\__\\___|

      SERIAL ACCESS FROM YOUR BROWSER

  This Web Serial terminal connects
  to USB serial devices from Chrome,
  Edge, Firefox 151+ desktop, Opera
  and compatible Chromium browsers.

  Use the settings button to change
  baud rate, data bits, parity,
  stop bits, flow control, and Enter.

  1. Plug in your device by USB.
  2. Click Connect at the top right.
  3. Select the USB serial port.
  4. Click here and start typing.

  Web Serial requires HTTPS or localhost.
  The OS must already expose the device
  as a serial port.

  Serial data is processed locally in
  your browser and is never uploaded
  by this application.

  ESP32 Bit Pirate - Hardware hacking tool:
  https://github.com/geo-tp/ESP32-Bit-Pirate
`;

const serial = new WebSerialConnection();
const terminal = new SerialTerminal(elements.terminal);

let hasReceivedSerialData = false;
let serialLogByteLength = 0;

function updateDownloadButton(byteLength = serialLogByteLength) {
  serialLogByteLength = byteLength;
  const canDownload = hasReceivedSerialData && byteLength > 0;

  elements.downloadButton.disabled = !canDownload;
  elements.downloadButton.setAttribute("aria-disabled", String(!canDownload));
  elements.downloadButton.title = canDownload
    ? `Download serial log (${byteLength} bytes)`
    : "No serial log to download";
}

function resetDownloadState() {
  hasReceivedSerialData = false;
  serialLogByteLength = 0;
  updateDownloadButton(0);
}

terminal.onLogChange((byteLength) => {
  updateDownloadButton(byteLength);
});

let connecting = false;
let connected = false;
let lastFocusedElement = null;
let statusMessage = "";

function loadPreference(key, fallback) {
  try {
    return localStorage.getItem(key) || fallback;
  } catch {
    return fallback;
  }
}

function savePreference(key, value) {
  try {
    localStorage.setItem(key, value);
  } catch {
    // Preferences are optional; the terminal should keep working without them.
  }
}

function loadSerialSettings() {
  try {
    const settings = {
      ...DEFAULT_SERIAL_SETTINGS,
      ...JSON.parse(localStorage.getItem(SERIAL_SETTINGS_STORAGE_KEY) || "{}")
    };
    return normalizeSerialSettings(settings);
  } catch {
    return { ...DEFAULT_SERIAL_SETTINGS };
  }
}

function saveSerialSettings(settings) {
  savePreference(SERIAL_SETTINGS_STORAGE_KEY, JSON.stringify(normalizeSerialSettings(settings)));
}

function normalizeSerialSettings(settings) {
  const dataBits = [7, 8];
  const stopBits = [1, 2];
  const parities = ["none", "even", "odd"];
  const flowControls = ["none", "hardware"];
  const enterModes = ["cr", "lf", "crlf"];

  const baudRate = Number(settings.baudRate);

  return {
    baudRate: Number.isInteger(baudRate) && baudRate > 0 ? baudRate : DEFAULT_SERIAL_SETTINGS.baudRate,
    dataBits: dataBits.includes(Number(settings.dataBits)) ? Number(settings.dataBits) : DEFAULT_SERIAL_SETTINGS.dataBits,
    stopBits: stopBits.includes(Number(settings.stopBits)) ? Number(settings.stopBits) : DEFAULT_SERIAL_SETTINGS.stopBits,
    parity: parities.includes(settings.parity) ? settings.parity : DEFAULT_SERIAL_SETTINGS.parity,
    flowControl: flowControls.includes(settings.flowControl) ? settings.flowControl : DEFAULT_SERIAL_SETTINGS.flowControl,
    enterMode: enterModes.includes(settings.enterMode) ? settings.enterMode : DEFAULT_SERIAL_SETTINGS.enterMode,
    localEcho: Boolean(settings.localEcho)
  };
}

function readSerialSettingsForm() {
  return {
    baudRate: Number(elements.baudRateInput.value),
    dataBits: Number(elements.dataBitsSelect.value),
    stopBits: Number(elements.stopBitsSelect.value),
    parity: elements.paritySelect.value,
    flowControl: elements.flowControlSelect.value,
    enterMode: elements.enterModeSelect.value,
    localEcho: elements.localEchoCheckbox.checked
  };
}

function applySerialSettings(settings) {
  settings = normalizeSerialSettings(settings);
  elements.baudRateInput.value = String(settings.baudRate);
  elements.dataBitsSelect.value = String(settings.dataBits);
  elements.stopBitsSelect.value = String(settings.stopBits);
  elements.paritySelect.value = settings.parity;
  elements.flowControlSelect.value = settings.flowControl;
  elements.enterModeSelect.value = settings.enterMode;
  elements.localEchoCheckbox.checked = settings.localEcho;
  terminal.setEnterMode(settings.enterMode);
  terminal.setLocalEcho(settings.localEcho);
}

function saveCurrentSerialSettings() {
  const settings = normalizeSerialSettings(readSerialSettingsForm());
  saveSerialSettings(settings);
  terminal.setEnterMode(settings.enterMode);
  terminal.setLocalEcho(settings.localEcho);
  return settings;
}

function updateThemeSelectColor(theme) {
  elements.themeSelect.classList.remove("theme-soft", "theme-green", "theme-contrast");
  elements.themeSelect.classList.add(`theme-${theme}`);
}

function applyStoredPreferences() {
  const storedTheme = loadPreference(THEME_STORAGE_KEY, "soft");
  const storedFontSize = Number(loadPreference(FONT_SIZE_STORAGE_KEY, "14"));
  const theme = ["soft", "green", "contrast"].includes(storedTheme) ? storedTheme : "soft";

  elements.themeSelect.value = theme;
  updateThemeSelectColor(theme);
  terminal.setTheme(theme);
  terminal.setFontSize(storedFontSize);
}

function syncTerminalWithPageTheme() {
  terminal.setPageTheme(document.documentElement.dataset.theme);
}

function showWelcome() {
  terminal.clear();
  terminal.write(WELCOME_TEXT.replace(/\n/g, "\r\n"), { log: false });
}

function setConnectedUi(connected) {
  const supported = serial.isSupported();
  const connectLabel = !supported ? "Unsupported" : connected ? "Disconnect" : connecting ? "Connecting..." : "Connect";
  const statusLabel = statusMessage || (!supported ? "Unsupported browser" : connected ? "Connected" : connecting ? "Connecting" : "Ready");

  elements.connectButton.disabled = connecting || !supported;
  elements.connectButton.textContent = connectLabel;
  elements.connectButton.classList.toggle("primary", !connected);
  elements.configButton.disabled = connecting || connected;
  elements.configButton.classList.toggle("is-suggested", supported && !connecting && !connected);
  elements.configButton.setAttribute("aria-expanded", String(!elements.configPanel.hidden));
  elements.connectionStatus.textContent = statusLabel;
}

async function connect() {
  if (connecting || connected) {
    return;
  }

  connecting = true;
  statusMessage = "";
  setConnectedUi(false);

  try {
    const settings = saveCurrentSerialSettings();
    closeConfig({ restoreFocus: false });
    terminal.clear();
    terminal.clearLog();
    resetDownloadState();
    await serial.connect({
      baudRate: settings.baudRate,
      dataBits: settings.dataBits,
      stopBits: settings.stopBits,
      parity: settings.parity,
      flowControl: settings.flowControl
    });
    connected = true;
    statusMessage = "";
    terminal.focus();
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    terminal.write(`\r\n[Connection failed: ${message}]\r\n`, { log: false });
    statusMessage = "Connection failed";
    connected = false;
  } finally {
    connecting = false;
    setConnectedUi(connected);
  }
}

function openConfig() {
  lastFocusedElement = document.activeElement;
  elements.configOverlay.hidden = false;
  elements.configPanel.hidden = false;
  elements.configButton.setAttribute("aria-expanded", "true");
  elements.configCloseButton.focus();
}

function closeConfig(options = {}) {
  const { restoreFocus = true } = options;
  if (elements.configPanel.hidden) {
    return;
  }

  saveCurrentSerialSettings();
  elements.configOverlay.hidden = true;
  elements.configPanel.hidden = true;
  elements.configButton.setAttribute("aria-expanded", "false");

  if (!restoreFocus) {
    return;
  }

  if (lastFocusedElement && typeof lastFocusedElement.focus === "function") {
    lastFocusedElement.focus();
  } else {
    terminal.focus();
  }
}

async function disconnect() {
  if (connecting) {
    return;
  }

  await serial.disconnect("Disconnected");
  connected = false;
  statusMessage = "";
  setConnectedUi(connected);
}

function initializeCompatibility() {
  if (serial.isSupported()) {
    return;
  }

  elements.compatibilityBanner.hidden = false;
  terminal.write("\r\n[Web Serial is not available.]\r\n", { log: false });
  terminal.write("[Use Chrome 89+, Edge 89+, Firefox 151+ desktop, Opera or a compatible Chromium browser on HTTPS or localhost.]\r\n", { log: false });
  setConnectedUi(false);
}

terminal.onInput(async (data) => {
  try {
    await serial.write(data);
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    terminal.write(`\r\n[Write failed: ${message}]\r\n`, { log: false });
  }
});

serial.addEventListener("data", (event) => {
  if (event.detail.length > 0) {
    hasReceivedSerialData = true;
  }
  terminal.write(event.detail);
});

serial.addEventListener("status", (event) => {
  const { state, message } = event.detail;
  connected = state === "connected";
  statusMessage = "";
  setConnectedUi(connected);

  if (state === "disconnected" && message !== "Disconnected") {
    terminal.write(`\r\n[${message}]\r\n`, { log: false });
    statusMessage = message;
    setConnectedUi(false);
  }
});

serial.addEventListener("error", (event) => {
  terminal.write(`\r\n[Serial error: ${event.detail.message}]\r\n`, { log: false });
  statusMessage = "Serial error";
  setConnectedUi(connected);
});

elements.connectButton.addEventListener("click", () => {
  if (connected) {
    void disconnect();
  } else {
    void connect();
  }
});
elements.clearButton.addEventListener("click", () => {
  terminal.clearLog();
  resetDownloadState();

  if (connected) {
    terminal.clear();
  } else {
    showWelcome();
  }

  terminal.focus();
});
elements.downloadButton.addEventListener("click", () => {
  if (!hasReceivedSerialData || serialLogByteLength === 0) {
    return;
  }
  terminal.downloadLog();
});
elements.baudRateInput.addEventListener("input", saveCurrentSerialSettings);
elements.themeSelect.addEventListener("change", () => {
  terminal.setTheme(elements.themeSelect.value);
  updateThemeSelectColor(elements.themeSelect.value);
  savePreference(THEME_STORAGE_KEY, elements.themeSelect.value);
  terminal.focus();
});
elements.fontDecreaseButton.addEventListener("click", () => {
  const fontSize = terminal.adjustFontSize(-1);
  savePreference(FONT_SIZE_STORAGE_KEY, String(fontSize));
  terminal.focus();
});
elements.fontIncreaseButton.addEventListener("click", () => {
  const fontSize = terminal.adjustFontSize(1);
  savePreference(FONT_SIZE_STORAGE_KEY, String(fontSize));
  terminal.focus();
});
elements.configButton.addEventListener("click", openConfig);
elements.configCloseButton.addEventListener("click", closeConfig);
elements.configOverlay.addEventListener("click", closeConfig);
elements.configPanel.addEventListener("change", saveCurrentSerialSettings);
document.addEventListener("keydown", (event) => {
  if (elements.configPanel.hidden) {
    return;
  }

  if (event.key === "Escape") {
    closeConfig();
    return;
  }

  if (event.key === "Tab") {
    const focusable = Array.from(elements.configPanel.querySelectorAll("button, input, select, textarea, a[href]"))
      .filter((element) => !element.disabled && element.offsetParent !== null);

    if (focusable.length === 0) {
      return;
    }

    const first = focusable[0];
    const last = focusable[focusable.length - 1];

    if (event.shiftKey && document.activeElement === first) {
      event.preventDefault();
      last.focus();
    } else if (!event.shiftKey && document.activeElement === last) {
      event.preventDefault();
      first.focus();
    }
  }
});

document.addEventListener("visibilitychange", () => {
  if (!document.hidden) {
    terminal.fit();
  }
});

window.addEventListener("beforeunload", () => {
  if (connected) {
    void serial.disconnect("Page closed");
  }
});

setConnectedUi(false);
terminal.setAutoScroll(true);
applyStoredPreferences();
syncTerminalWithPageTheme();

new MutationObserver(syncTerminalWithPageTheme).observe(document.documentElement, {
  attributes: true,
  attributeFilter: ["data-theme"]
});
applySerialSettings(loadSerialSettings());
showWelcome();
resetDownloadState();
initializeCompatibility();
