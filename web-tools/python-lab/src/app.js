import { WebSerialConnection } from "../../web-serial-terminal/scripts/web-serial.js";
import { Bpio2Client as WebBpio2Client } from "../../bpio2/src/Bpio2Client.js";

const PYODIDE_VERSION = "0.26.4";
const PYODIDE_INDEX_URL = `https://cdn.jsdelivr.net/pyodide/v${PYODIDE_VERSION}/full/`;
const MONACO_VERSION = "0.49.0";
const MONACO_BASE_URL = `https://cdn.jsdelivr.net/npm/monaco-editor@${MONACO_VERSION}/min/vs`;
const SCRIPT_REPO_API_URL = "https://api.github.com/repos/geo-tp/ESP32-Bit-Pirate-Scripts/contents?ref=main";
const SCRIPT_REPO_RAW_BASE = "https://raw.githubusercontent.com/geo-tp/ESP32-Bit-Pirate-Scripts/main/";
const SPLIT_STORAGE_KEY = "python-lab-split-ratio";
const SPLIT_MIN_PANEL_WIDTH = 360;
const SPLIT_DESKTOP_MIN_WIDTH = 920;
const SPLIT_DEFAULT_RATIO = 0.65;
const SPLIT_DIVIDER_WIDTH = 6;

const STARTER_SCRIPT = `# Interactive I2C quick start for ESP32 Bit Pirate.
# Before running a script, put the Bit Pirate in the mode expected
# by that script: serial terminal mode for CLI scripts, WiFi mode for
# WiFi workflows, or USB adapter/BPIO2 mode for BPIO2 scripts.
# 1. Click Connect and choose the Bit Pirate serial CLI port.
# 2. Click Run. Python wakes the CLI, enters I2C mode, accepts
#    the default pin/config prompts, then scans the bus.
# 3. Open Output to inspect the responses or save the session.

from bitpirate_web import bp

bp.connect()
bp.start()

print("Switching to I2C mode...")
bp.change_mode("i2c")

print("Scanning I2C bus...")
print(bp.cmd("scan", timeout=2500))
`;

const PYTHON_MODULE = `
import bitpirate_web_js
import asyncio
import json
import re

class _ConnectDescriptor:
    def __get__(self, instance, owner):
        if instance is None:
            async def connect_class():
                connected = owner()
                await connected.connect()
                return connected
            return connect_class

        async def connect_instance():
            await bitpirate_web_js.connect()
            return instance
        return connect_instance

class BitPirate:
    connect = _ConnectDescriptor()

    @classmethod
    async def auto_connect(cls):
        return await cls.connect()

    async def send(self, command):
        await bitpirate_web_js.send(str(command))

    async def wait(self, delay=0.3, quiet=0.12, timeout=None):
        if timeout is not None:
            delay = float(timeout) / 1000
        await bitpirate_web_js.wait_seconds(float(delay), float(quiet))

    def receive(self, skip=1):
        return list(bitpirate_web_js.receive_lines(int(skip)))

    async def receive_wait(self, skip=1, timeout=0.5):
        return list(await bitpirate_web_js.receive_lines_async(int(skip), float(timeout)))

    async def receive_all(self, silence_timeout=0.5):
        return list(await bitpirate_web_js.receive_lines_async(0, float(silence_timeout)))

    async def clear_echoes(self, lines=1):
        await bitpirate_web_js.clear_echoes(int(lines))

    async def flush(self):
        await bitpirate_web_js.flush()

    async def sleep(self, seconds):
        await asyncio.sleep(float(seconds))

    async def start(self, wake_attempts=10):
        await self.flush()
        await self.send("n")
        await self.wait()
        await self.send("1")
        await self.wait()
        await bitpirate_web_js.send_newlines(int(wake_attempts))
        await self.wait()
        await self.flush()

    async def change_mode(self, mode):
        await self.send("m " + str(mode).lower())
        await bitpirate_web_js.send_newlines(10)
        await self.wait()
        await self.flush()

    async def cmd(self, command, timeout=1000, quiet=120):
        return str(await bitpirate_web_js.cmd(str(command), float(timeout), float(quiet)))

    async def stop(self):
        await bitpirate_web_js.stop()

class Helper:
    @staticmethod
    def extractHexFromList(lines):
        values = set()
        for line in lines:
            values.update(re.findall(r"0x[0-9A-Fa-f]{2}", str(line)))
        return sorted(values)

    @staticmethod
    def extractSsidsFromList(lines):
        ssids = []
        for line in lines:
            if "SSID:" not in str(line):
                continue
            for part in str(line).split("|"):
                part = part.strip()
                if part.startswith("SSID:"):
                    ssid = part.replace("SSID:", "").strip()
                    if ssid:
                        ssids.append(ssid)
        return ssids

class BPIOClient:
    def __init__(self, port=None, timeout=5):
        self.port = port
        self.timeout = timeout

    @classmethod
    async def connect(cls, port=None, timeout=5):
        instance = cls(port, timeout)
        await bitpirate_web_js.bpio_connect(float(timeout))
        return instance

    async def status_request(self):
        return json.loads(str(await bitpirate_web_js.bpio_status()))

    async def configuration_request(self, **kwargs):
        return bool(await bitpirate_web_js.bpio_configure(json.dumps(kwargs)))

    async def close(self):
        await bitpirate_web_js.bpio_disconnect()

    async def stop(self):
        await self.close()

class BPIOI2C:
    def __init__(self, client):
        self.client = client

    async def configure(self, speed=100000, clock_stretch=False, **kwargs):
        return bool(await bitpirate_web_js.bpio_configure_i2c(int(speed), bool(clock_stretch)))

    async def transfer(self, data, read_bytes=0):
        result = await bitpirate_web_js.bpio_transfer(list(data), int(read_bytes))
        if result is False:
            return False
        return list(result)

class BPIOSPI:
    def __init__(self, client):
        self.client = client

    async def configure(self, speed=1000000, clock_polarity=False, clock_phase=False, chip_select_idle=True, mode_bitorder_msb=True, **kwargs):
        return bool(await bitpirate_web_js.bpio_configure_spi(
            int(speed),
            bool(clock_polarity),
            bool(clock_phase),
            bool(chip_select_idle),
            bool(mode_bitorder_msb),
        ))

    async def transfer(self, data, read_bytes=0):
        result = await bitpirate_web_js.bpio_transfer(list(data), int(read_bytes))
        if result is False:
            return False
        return list(result)

bp = BitPirate()
`;

const elements = {
  compatibilityBanner: document.querySelector("#compatibilityBanner"),
  exampleSelect: document.querySelector("#exampleSelect"),
  importCodeButton: document.querySelector("#importCodeButton"),
  importCodeInput: document.querySelector("#importCodeInput"),
  saveCodeButton: document.querySelector("#saveCodeButton"),
  connectButton: document.querySelector("#connectButton"),
  runButton: document.querySelector("#runButton"),
  stopButton: document.querySelector("#stopButton"),
  clearButton: document.querySelector("#clearButton"),
  saveOutputButton: document.querySelector("#saveOutputButton"),
  rawSerialToggle: document.querySelector("#rawSerialToggle"),
  editor: document.querySelector("#editor"),
  editorLoading: document.querySelector("#editorLoading"),
  fallbackEditor: document.querySelector("#fallbackEditor"),
  generatedFilesPanel: document.querySelector("#generatedFilesPanel"),
  generatedFilesList: document.querySelector("#generatedFilesList"),
  workspace: document.querySelector(".workspace"),
  splitDivider: document.querySelector("#splitDivider"),
  consoleTerminal: document.querySelector("#consoleTerminal")
};

const serial = new WebSerialConnection();
const consoleTerm = createTerminal(elements.consoleTerminal);

let pyodide = null;
let editorView = null;
let running = false;
let interruptBuffer = null;
let serialBuffer = "";
let serialVersion = 0;
let connected = false;
let connecting = false;
let showRawSerial = false;
let remoteScripts = [];
let outputLog = "";
let bpioClient = null;
let generatedFiles = new Map();
let stopRequested = false;
let currentScriptName = "python-lab-script.py";

elements.fallbackEditor.value = STARTER_SCRIPT;

function createTerminal(container) {
  const theme = getTerminalTheme();
  const term = new window.Terminal({
    convertEol: true,
    cursorBlink: false,
    disableStdin: true,
    fontFamily: 'Menlo, "Courier New", Courier, "Liberation Mono", monospace',
    fontSize: 13,
    lineHeight: 1.15,
    scrollback: 5000,
    theme
  });
  const fit = new window.FitAddon.FitAddon();
  term.loadAddon(fit);
  term.open(container);
  fit.fit();
  term.fitAddon = fit;
  return term;
}

function getTerminalTheme() {
  if (document.documentElement.dataset.theme === "light") {
    return {
      background: "#f8fcf9",
      foreground: "#183128",
      cursor: "#007f7a",
      selectionBackground: "#c9e8dc",
      black: "#183128",
      red: "#a63d45",
      green: "#087345",
      yellow: "#8a6200",
      blue: "#2359a5",
      magenta: "#6d3f91",
      cyan: "#007f7a",
      white: "#ffffff"
    };
  }

  return {
    background: "#101412",
    foreground: "#d8ddd8",
    cursor: "#00ffcc",
    selectionBackground: "#245447",
    black: "#101412",
    red: "#ff6b6b",
    green: "#00ff00",
    yellow: "#ffdc7d",
    blue: "#70a7ff",
    magenta: "#c792ea",
    cyan: "#00ffcc",
    white: "#e0e0e0"
  };
}

function syncTerminalTheme() {
  consoleTerm.options.theme = getTerminalTheme();
}

function syncEditorTheme() {
  if (window.monaco?.editor) {
    window.monaco.editor.setTheme(getMonacoThemeName());
  }
}

function fitTerminals() {
  consoleTerm.fitAddon.fit();
  editorView?.layout?.();
}

function isSplitEnabled() {
  return window.matchMedia(`(min-width: ${SPLIT_DESKTOP_MIN_WIDTH}px)`).matches;
}

function getSplitMetrics() {
  const rect = elements.workspace.getBoundingClientRect();
  const styles = window.getComputedStyle(elements.workspace);
  const columnGap = Number.parseFloat(styles.columnGap) || 0;
  const dividerWidth = elements.splitDivider.getBoundingClientRect().width || SPLIT_DIVIDER_WIDTH;
  const usableWidth = rect.width - dividerWidth - (columnGap * 2);
  const minLeft = SPLIT_MIN_PANEL_WIDTH;
  const maxLeft = usableWidth - SPLIT_MIN_PANEL_WIDTH;
  return { rect, usableWidth, minLeft, maxLeft };
}

function applySplitRatio(ratio) {
  if (!isSplitEnabled()) {
    elements.workspace.style.gridTemplateColumns = "";
    return;
  }

  const { usableWidth, minLeft, maxLeft } = getSplitMetrics();
  if (maxLeft <= minLeft) {
    elements.workspace.style.gridTemplateColumns = "";
    return;
  }

  const safeRatio = Math.min(0.76, Math.max(0.34, Number(ratio) || SPLIT_DEFAULT_RATIO));
  const leftWidth = Math.min(maxLeft, Math.max(minLeft, usableWidth * safeRatio));
  elements.workspace.style.gridTemplateColumns = `${leftWidth}px ${SPLIT_DIVIDER_WIDTH}px minmax(${SPLIT_MIN_PANEL_WIDTH}px, 1fr)`;
  window.requestAnimationFrame(fitTerminals);
}

function readSplitRatio() {
  try {
    return Number.parseFloat(window.localStorage.getItem(SPLIT_STORAGE_KEY) || String(SPLIT_DEFAULT_RATIO));
  } catch {
    return SPLIT_DEFAULT_RATIO;
  }
}

function saveSplitRatio(leftWidth) {
  const { usableWidth } = getSplitMetrics();
  if (usableWidth > 0) {
    try {
      window.localStorage.setItem(SPLIT_STORAGE_KEY, String(leftWidth / usableWidth));
    } catch {
      // Split persistence is optional.
    }
  }
}

function resetSplitIfNeeded() {
  applySplitRatio(readSplitRatio());
}

function setupSplitDivider() {
  let dragging = false;

  const moveTo = (clientX) => {
    if (!isSplitEnabled()) {
      return;
    }

    const { rect, minLeft, maxLeft } = getSplitMetrics();
    if (maxLeft <= minLeft) {
      return;
    }

    const leftWidth = Math.min(maxLeft, Math.max(minLeft, clientX - rect.left));
    elements.workspace.style.gridTemplateColumns = `${leftWidth}px ${SPLIT_DIVIDER_WIDTH}px minmax(${SPLIT_MIN_PANEL_WIDTH}px, 1fr)`;
    saveSplitRatio(leftWidth);
    fitTerminals();
  };

  elements.splitDivider.addEventListener("pointerdown", (event) => {
    if (!isSplitEnabled()) {
      return;
    }
    dragging = true;
    elements.splitDivider.setPointerCapture(event.pointerId);
    document.body.classList.add("is-resizing-split");
    moveTo(event.clientX);
  });

  elements.splitDivider.addEventListener("pointermove", (event) => {
    if (dragging) {
      moveTo(event.clientX);
    }
  });

  const stopDragging = (event = null) => {
    if (!dragging) {
      return;
    }
    dragging = false;
    if (event?.pointerId !== undefined && elements.splitDivider.hasPointerCapture(event.pointerId)) {
      elements.splitDivider.releasePointerCapture(event.pointerId);
    }
    document.body.classList.remove("is-resizing-split");
  };

  elements.splitDivider.addEventListener("pointerup", stopDragging);
  elements.splitDivider.addEventListener("pointercancel", stopDragging);
  window.addEventListener("pointerup", stopDragging);
  window.addEventListener("pointercancel", stopDragging);
  window.addEventListener("blur", () => stopDragging());

  elements.splitDivider.addEventListener("keydown", (event) => {
    if (!isSplitEnabled() || !["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) {
      return;
    }
    event.preventDefault();
    const { usableWidth } = getSplitMetrics();
    const currentRatio = readSplitRatio();
    const step = event.shiftKey ? 0.08 : 0.03;
    const nextRatio = event.key === "Home"
      ? 0.34
      : event.key === "End"
        ? 0.76
        : currentRatio + (event.key === "ArrowRight" ? step : -step);
    const nextLeft = usableWidth * Math.min(0.76, Math.max(0.34, nextRatio));
    saveSplitRatio(nextLeft);
    applySplitRatio(readSplitRatio());
  });

  applySplitRatio(readSplitRatio());
}

function writeConsole(text, tone = "normal") {
  const prefix = tone === "error" ? "\x1b[31m" : tone === "system" ? "\x1b[36m" : "";
  const suffix = prefix ? "\x1b[0m" : "";
  const value = String(text);
  outputLog += value;
  elements.saveOutputButton.disabled = outputLog.length === 0;
  consoleTerm.write(`${prefix}${value.replace(/\n/g, "\r\n")}${suffix}`);
}

function updateUi() {
  const supported = serial.isSupported();
  const hasOpenConnection = connected || Boolean(bpioClient?.connected);
  elements.compatibilityBanner.hidden = supported;
  elements.connectButton.disabled = connecting || !supported;
  elements.connectButton.textContent = hasOpenConnection ? "Disconnect" : connecting ? "Connecting..." : "Connect";
  elements.connectButton.classList.toggle("primary", !hasOpenConnection);
  elements.runButton.disabled = running || !pyodide;
  elements.stopButton.disabled = !running;
}

async function loadEditor() {
  try {
    const monaco = await loadMonaco();
    defineMonacoSurfaceThemes(monaco);
    editorView = monaco.editor.create(elements.editor, {
      value: STARTER_SCRIPT,
      language: "python",
      theme: getMonacoThemeName(),
      automaticLayout: true,
      minimap: { enabled: false },
      scrollBeyondLastLine: false,
      fontFamily: 'Menlo, "Courier New", Courier, "Liberation Mono", monospace',
      fontSize: 14,
      lineHeight: 20,
      tabSize: 4,
      insertSpaces: true,
      wordWrap: "off",
      renderWhitespace: "selection",
      smoothScrolling: false,
      contextmenu: true,
      fixedOverflowWidgets: true,
      overviewRulerLanes: 0
    });
    elements.editor.addEventListener("pointerdown", () => {
      document.body.classList.remove("is-resizing-split");
    }, true);
    elements.editorLoading?.remove();
    elements.editor.classList.add("is-ready");
  } catch (error) {
    elements.editorLoading?.remove();
    elements.editor.classList.add("is-fallback");
    writeConsole(`[Monaco unavailable, using textarea: ${error.message || error}]\n`, "system");
  }
}

function getCode() {
  return editorView ? editorView.getValue() : elements.fallbackEditor.value;
}

function setCode(code) {
  if (editorView) {
    editorView.setValue(code);
  }
  elements.fallbackEditor.value = code;
}

function loadMonaco() {
  if (window.monaco?.editor) {
    return Promise.resolve(window.monaco);
  }

  return new Promise((resolve, reject) => {
    const existingLoader = document.querySelector('script[data-monaco-loader="true"]');
    const configure = () => {
      window.require.config({ paths: { vs: MONACO_BASE_URL } });
      window.MonacoEnvironment = {
        getWorkerUrl: () => `data:text/javascript;charset=utf-8,${encodeURIComponent(`
          self.MonacoEnvironment = { baseUrl: "${MONACO_BASE_URL}/" };
          importScripts("${MONACO_BASE_URL}/base/worker/workerMain.js");
        `)}`
      };
      window.require(["vs/editor/editor.main"], () => resolve(window.monaco), reject);
    };

    if (window.require) {
      configure();
      return;
    }

    const loader = existingLoader || document.createElement("script");
    loader.dataset.monacoLoader = "true";
    loader.src = `${MONACO_BASE_URL}/loader.js`;
    loader.onload = configure;
    loader.onerror = () => reject(new Error("Monaco loader failed"));
    if (!existingLoader) {
      document.head.append(loader);
    }
  });
}

function getMonacoThemeName() {
  return document.documentElement.dataset.theme === "light" ? "bitpirate-surface-light" : "bitpirate-surface-dark";
}

function defineMonacoSurfaceThemes(monaco) {
  monaco.editor.defineTheme("bitpirate-surface-dark", {
    base: "vs-dark",
    inherit: true,
    rules: [],
    colors: {
      "editor.background": "#0f1513",
      "editorGutter.background": "#0b100e",
      "editor.lineHighlightBackground": "#18231f",
      "editorLineNumber.foreground": "#6f837a",
      "editorCursor.foreground": "#d8ddd8"
    }
  });

  monaco.editor.defineTheme("bitpirate-surface-light", {
    base: "vs",
    inherit: true,
    rules: [],
    colors: {
      "editor.background": "#f7fbf8",
      "editorGutter.background": "#edf4ef",
      "editor.lineHighlightBackground": "#eaf4ee",
      "editorLineNumber.foreground": "#52665c",
      "editorCursor.foreground": "#08705b"
    }
  });
}

async function waitForPyodideLoader() {
  for (let index = 0; index < 200; index += 1) {
    if (typeof window.loadPyodide === "function") {
      return;
    }
    await sleep(50);
  }
  throw new Error("Pyodide loader did not become available.");
}

async function loadPython() {
  writeConsole("Loading Python...\n", "system");
  await waitForPyodideLoader();
  pyodide = await window.loadPyodide({
    indexURL: PYODIDE_INDEX_URL,
    stdout: (text) => writeConsole(`${text}\n`),
    stderr: (text) => writeConsole(`${text}\n`, "error")
  });

  pyodide.registerJsModule("bitpirate_web_js", createPythonBridge());
  pyodide.runPython(`
import sys
import types

module = types.ModuleType("bitpirate_web")
exec(${JSON.stringify(PYTHON_MODULE)}, module.__dict__)
sys.modules["bitpirate_web"] = module
`);
  writeConsole("Python ready. Connect Bit Pirate, then run a script.\n", "system");
}

async function loadRemoteScriptList() {
  const placeholder = document.createElement("option");
  placeholder.value = "";
  placeholder.textContent = "Loading repo scripts...";
  placeholder.disabled = true;
  elements.exampleSelect.append(placeholder);

  try {
    const response = await fetch(SCRIPT_REPO_API_URL, { headers: { Accept: "application/vnd.github+json" } });
    if (!response.ok) {
      throw new Error(`GitHub returned ${response.status}`);
    }

    const entries = await response.json();
    remoteScripts = entries
      .filter((entry) => entry.type === "file" && entry.name.endsWith(".py"))
      .sort((left, right) => left.name.localeCompare(right.name));

    placeholder.remove();
    if (remoteScripts.length === 0) {
      return;
    }

    const group = document.createElement("optgroup");
    group.label = "ESP32-Bit-Pirate-Scripts";
    for (const script of remoteScripts) {
      const option = document.createElement("option");
      option.value = `remote:${script.path}`;
      option.textContent = script.name.replace(/_/g, " ").replace(/\.py$/, "");
      group.append(option);
    }
    elements.exampleSelect.append(group);
  } catch (error) {
    placeholder.textContent = "Repo scripts unavailable";
    writeConsole(`[Script list unavailable: ${error.message || error}]\n`, "system");
  }
}

async function loadRemoteScript(path) {
  const url = `${SCRIPT_REPO_RAW_BASE}${path}`;
  writeConsole(`[Fetching ${path}]\n`, "system");

  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`GitHub returned ${response.status}`);
  }

  const source = await response.text();
  currentScriptName = path.split("/").pop() || "python-lab-script.py";
  setCode(convertScriptForWeb(source, path));
  selectRemoteScript(path);
}

function selectRemoteScript(path) {
  const value = `remote:${path}`;
  const label = path.split("/").pop()?.replace(/_/g, " ").replace(/\.py$/, "") || "Selected script";
  let option = Array.from(elements.exampleSelect.options).find((item) => item.value === value);

  if (!option) {
    option = document.createElement("option");
    option.value = value;
    option.textContent = label;
    elements.exampleSelect.append(option);
  }

  option.textContent = label;
  elements.exampleSelect.value = value;
  elements.exampleSelect.title = label;
}

function convertScriptForWeb(source, path = "remote-script.py") {
  const warnings = [];
  let code = source.replace(/\r\n?/g, "\n");

  if (/bitpirate\.bpio2|BPIOClient|BPIOGPIO|BPIOI2C|BPIOSPI/.test(code)) {
    warnings.push("BPIO2 scripts require the Bit Pirate BPIO2 USB adapter mode and will open a Web Serial BPIO2 session.");
  }

  if (/serial|serial\.tools|pyserial/i.test(code)) {
    warnings.push("Local pyserial/COM/tty access was removed; Python Scripting Lab uses the browser Web Serial bridge.");
  }

  code = code
    .replace(/^\s*from\s+bitpirate\.bpio2\s+import\s+.*$/gm, "from bitpirate_web import BPIOClient, BPIOI2C, BPIOSPI")
    .replace(/^\s*from\s+bitpirate(?:\.\w+)?\s+import\s+.*$/gm, "from bitpirate_web import BitPirate, Helper, bp")
    .replace(/^\s*import\s+serial(?:\..*)?\s*$/gm, "")
    .replace(/^\s*import\s+serial\.tools\.list_ports\s*$/gm, "")
    .replace(/^\s*from\s+serial(?:\..*)?\s+import\s+.*$/gm, "")
    .replace(/\bbp\.receive\s*\(/g, "bp.receive_wait(")
    .replace(/BitPirate\.auto_connect\s*\([^)]*\)/g, "BitPirate.connect()")
    .replace(/BitPirate\s*\([^)]*\)/g, "BitPirate.connect()")
    .replace(/time\.sleep\s*\(/g, "bp.sleep(");

  code = code
    .replace(/^(\s*)with\s+BPIOClient\([^)]*timeout\s*=\s*([^,)]+)[^)]*\)\s+as\s+(\w+):/gm, "$1$3 = BPIOClient.connect(timeout=$2)\n$1if True:")
    .replace(/^(\s*)with\s+BPIOClient\([^)]*\)\s+as\s+(\w+):/gm, "$1$2 = BPIOClient.connect()\n$1if True:")
    .replace(/BPIOClient\s*\([^)]*timeout\s*=\s*([^,)]+)[^)]*\)/g, "BPIOClient.connect(timeout=$1)")
    .replace(/BPIOClient\s*\([^)]*\)/g, "BPIOClient.connect()");

  code = stripDisplayAwaits(code);

  const header = [
    `# Converted from ESP32-Bit-Pirate-Scripts/${path}`,
    "# Transport: browser Web Serial via bitpirate_web.",
    "# Conversion is best-effort for CLI scripts; BPIO2/pyserial/local files may need manual edits.",
    ...warnings.map((warning) => `# Warning: ${warning}`),
    ""
  ].join("\n");

  if (!/(?:await\s+)?(?:BitPirate|bp)\.connect\(\)/.test(code)) {
    code = `from bitpirate_web import bp\n\nbp.connect()\nbp.start()\n\n${code}`;
  }

  return `${header}${code}`.replace(/\n{3,}/g, "\n\n");
}

function stripDisplayAwaits(source) {
  const knownAsyncCallPatterns = [
    /\bawait\s+(bp\.(?:connect|start|flush|change_mode|cmd|send|wait|stop|sleep|receive_wait|receive_all|clear_echoes)\s*\()/g,
    /\bawait\s+(BitPirate\.(?:connect|auto_connect)\s*\()/g,
    /\bawait\s+(BPIOClient\.connect\s*\()/g,
    /\bawait\s+((?:client|i2c|spi)\.(?:connect|status_request|configuration_request|configure|transfer|close|stop)\s*\()/g
  ];

  return knownAsyncCallPatterns.reduce((nextSource, pattern) => {
    return nextSource.replace(pattern, "$1");
  }, source);
}

function addImplicitAwaits(source) {
  const bpMethods = [
    "connect",
    "start",
    "flush",
    "change_mode",
    "cmd",
    "send",
    "wait",
    "stop",
    "sleep",
    "receive_wait",
    "receive_all",
    "clear_echoes"
  ];
  const bpioMethods = [
    "connect",
    "status_request",
    "configuration_request",
    "configure",
    "transfer",
    "close",
    "stop"
  ];
  const asyncCallPatterns = [
    new RegExp(`\\bbp\\.(?:${bpMethods.join("|")})\\s*\\(`, "g"),
    /\bBitPirate\.(?:connect|auto_connect)\s*\(/g,
    /\bBPIOClient\.connect\s*\(/g,
    new RegExp(`\\b(?:client|i2c|spi)\\.(?:${bpioMethods.join("|")})\\s*\\(`, "g")
  ];

  return source
    .split("\n")
    .map((line) => {
      const trimmed = line.trim();
      if (!trimmed || trimmed.startsWith("#") || /^(?:from|import)\s+/.test(trimmed)) {
        return line;
      }

      let nextLine = line;
      for (const pattern of asyncCallPatterns) {
        nextLine = nextLine.replace(pattern, (match, offset, fullLine) => {
          const before = fullLine.slice(Math.max(0, offset - 10), offset);
          if (/\bawait\s*$/.test(before)) {
            return match;
          }
          return `await ${match}`;
        });
      }
      return nextLine;
    })
    .join("\n");
}

function createPythonBridge() {
  return {
    connect,
    send: async (command) => {
      checkStopRequested();
      await ensureConnected();
      checkStopRequested();
      await serial.write(formatCommand(command));
    },
    wait: async (timeout = 1000, quiet = 120) => {
      checkStopRequested();
      await waitForSerial(timeout, quiet);
    },
    wait_seconds: async (delay = 0.3, quiet = 0.12) => {
      checkStopRequested();
      const timeoutMs = Math.max(0, Number(delay) || 0) * 1000;
      const quietMs = Math.max(0, Number(quiet) || 0) * 1000;
      await waitForSerial(timeoutMs, quietMs);
    },
    receive: (skip = 1) => {
      const output = serialBuffer;
      serialBuffer = "";
      return cleanBitPirateOutput(output, skip);
    },
    receive_lines: (skip = 1) => {
      const output = serialBuffer;
      serialBuffer = "";
      return cleanBitPirateLines(output, skip);
    },
    receive_lines_async: async (skip = 1, timeout = 0.5) => {
      checkStopRequested();
      await waitForSerial(Math.max(0, Number(timeout) || 0) * 1000, 120);
      const output = serialBuffer;
      serialBuffer = "";
      return cleanBitPirateLines(output, skip);
    },
    clear_echoes: async (lineCount = 1) => {
      checkStopRequested();
      await waitForSerial(250, 80);
      serialBuffer = cleanBitPirateLines(serialBuffer, lineCount).join("\n");
    },
    flush: async () => {
      checkStopRequested();
      serialBuffer = "";
      await waitForSerial(120, 60);
      serialBuffer = "";
    },
    send_newlines: async (count = 1) => {
      checkStopRequested();
      await ensureConnected();
      checkStopRequested();
      await serial.write("\n".repeat(Math.max(0, Number(count) || 0)));
    },
    cmd: async (command, timeout = 1000, quiet = 120) => {
      checkStopRequested();
      await ensureConnected();
      checkStopRequested();
      serialBuffer = "";
      await serial.write(formatCommand(command));
      await waitForSerial(timeout, quiet);
      const output = serialBuffer;
      serialBuffer = "";
      return cleanBitPirateOutput(output, 1);
    },
    stop: async () => {
      checkStopRequested();
      await waitForSerial(120, 60);
      return true;
    },
    bpio_connect: async (timeoutSeconds = 5) => {
      checkStopRequested();
      if (connected) {
        await disconnect();
      }
      if (bpioClient?.connected) {
        return true;
      }
      bpioClient = new WebBpio2Client({
        timeoutMs: Math.max(500, Number(timeoutSeconds) * 1000),
        log: (message) => writeConsole(`[BPIO2] ${message}\n`, "system")
      });
      await bpioClient.connect();
      writeConsole("[BPIO2 connected]\n", "system");
      updateUi();
      return true;
    },
    bpio_disconnect: async () => {
      checkStopRequested();
      if (!bpioClient) {
        return;
      }
      await bpioClient.disconnect({ returnToHiZ: true });
      bpioClient = null;
      writeConsole("[BPIO2 disconnected]\n", "system");
      updateUi();
    },
    bpio_status: async () => {
      checkStopRequested();
      const status = await ensureBpioClient().getStatus();
      return JSON.stringify(toSnakeStatus(status));
    },
    bpio_configure: async (jsonOptions) => {
      checkStopRequested();
      const options = normalizeBpioOptions(JSON.parse(String(jsonOptions || "{}")));
      await ensureBpioClient().configure(options);
      return true;
    },
    bpio_configure_i2c: async (speed = 100000, clockStretch = false) => {
      checkStopRequested();
      await ensureBpioClient().configureI2c({ speed: Number(speed), clockStretch: Boolean(clockStretch) });
      return true;
    },
    bpio_configure_spi: async (speed = 1000000, clockPolarity = false, clockPhase = false, chipSelectIdle = true, msbFirst = true) => {
      checkStopRequested();
      await ensureBpioClient().configureSpi({
        speed: Number(speed),
        mode: (clockPolarity ? 2 : 0) | (clockPhase ? 1 : 0),
        msbFirst: Boolean(msbFirst),
        csIdleHigh: Boolean(chipSelectIdle)
      });
      return true;
    },
    bpio_transfer: async (data, readBytes = 0) => {
      checkStopRequested();
      const result = await ensureBpioClient().dataRequest({
        startMain: true,
        dataWrite: Uint8Array.from(Array.from(data || []).map((byte) => Number(byte) & 0xff)),
        bytesRead: Math.max(0, Number(readBytes) || 0),
        stopMain: true
      }, { allowError: true });
      if (!result.ok) {
        return false;
      }
      checkStopRequested();
      return Array.from(result.data || []);
    }
  };
}

function ensureBpioClient() {
  if (!bpioClient?.connected) {
    throw new Error("BPIO2 is not connected. Select a BPIO2 script and choose the BPIO2 serial adapter port.");
  }
  return bpioClient;
}

function normalizeBpioOptions(options = {}) {
  const normalized = {};
  const map = {
    mode_configuration: "modeConfiguration",
    modeConfiguration: "modeConfiguration",
    io_value_mask: "ioValueMask",
    ioValueMask: "ioValueMask",
    io_value: "ioValue",
    ioValue: "ioValue",
    io_direction_mask: "ioDirectionMask",
    ioDirectionMask: "ioDirectionMask",
    io_direction: "ioDirection",
    ioDirection: "ioDirection"
  };

  for (const [key, value] of Object.entries(options || {})) {
    normalized[map[key] || key] = value;
  }
  return normalized;
}

function toSnakeStatus(status = {}) {
  return {
    version_flatbuffers_major: status.versionFlatbuffersMajor,
    version_flatbuffers_minor: status.versionFlatbuffersMinor,
    version_hardware_major: status.versionHardwareMajor,
    version_hardware_minor: status.versionHardwareMinor,
    version_firmware_major: status.versionFirmwareMajor,
    version_firmware_minor: status.versionFirmwareMinor,
    version_firmware_git_hash: status.versionFirmwareGitHash,
    version_firmware_date: status.versionFirmwareDate,
    modes_available: status.modesAvailable || [],
    mode_current: status.modeCurrent,
    mode_pin_labels: status.modePinLabels || [],
    mode_max_packet_size: status.modeMaxPacketSize,
    mode_max_write: status.modeMaxWrite,
    mode_max_read: status.modeMaxRead,
    io_direction: status.ioDirection,
    io_value: status.ioValue
  };
}

function formatCommand(command) {
  const text = String(command);
  return /[\r\n]$/.test(text) ? text : `${text}\n`;
}

function cleanBitPirateOutput(text, skip = 1) {
  return cleanBitPirateLines(text, skip).join("\n");
}

function cleanBitPirateLines(text, skip = 1) {
  const skipLines = Math.max(0, Number(skip) || 0);
  const lines = String(text)
    .replace(/\r/g, "")
    .split("\n")
    .map((line) => line.trimEnd())
    .filter((line) => line.trim().length > 0);

  const withoutEchoes = lines.slice(skipLines);
  return withoutEchoes
    .filter((line, index, all) => {
      const trimmed = line.trim();
      if (trimmed === ">") {
        return false;
      }
      return !(index === all.length - 1 && trimmed.endsWith(">"));
    });
}

async function ensureConnected() {
  if (!connected) {
    await connect();
  }
}

async function connect() {
  if (connected || connecting) {
    if (connected) {
      await wakeBitPirateCli({ announce: false });
    }
    return;
  }
  connecting = true;
  updateUi();

  try {
    await serial.connect({ baudRate: 115200 });
    connected = true;
    await wakeBitPirateCli({ announce: true });
    writeConsole("[Serial connected]\n", "system");
  } catch (error) {
    writeConsole(`[Connection failed: ${error.message || error}]\n`, "error");
    throw error;
  } finally {
    connecting = false;
    updateUi();
  }
}

async function wakeBitPirateCli({ announce = false } = {}) {
  if (!serial.connected) {
    return false;
  }

  serialBuffer = "";
  const before = serialVersion;
  await serial.write("\n\n\n");
  const hasResponse = await waitForSerial(1000, 140);

  if (announce) {
    writeConsole(hasResponse || serialVersion !== before
      ? "[Bit Pirate CLI responded]\n"
      : "[Serial connected; no CLI response yet]\n", "system");
  }

  return hasResponse;
}

async function disconnect() {
  if (!connected || connecting) {
    return;
  }
  await serial.disconnect("Disconnected");
  connected = false;
  writeConsole("[Serial disconnected]\n", "system");
  updateUi();
}

async function waitForSerial(timeout = 1000, quiet = 120) {
  checkStopRequested();
  const timeoutMs = Math.max(0, Number(timeout) || 0);
  const quietMs = Math.max(0, Number(quiet) || 0);
  const startVersion = serialVersion;
  let lastVersion = serialVersion;
  let lastChange = performance.now();
  const started = performance.now();

  while (performance.now() - started < timeoutMs) {
    checkStopRequested();
    if (serialVersion !== lastVersion) {
      lastVersion = serialVersion;
      lastChange = performance.now();
    }
    if (lastVersion !== startVersion && performance.now() - lastChange >= quietMs) {
      return true;
    }
    await sleep(25);
  }

  return lastVersion !== startVersion;
}

async function runPython() {
  if (!pyodide || running) {
    return;
  }

  running = true;
  stopRequested = false;
  const filesBeforeRun = snapshotPyodideFiles();
  if (typeof SharedArrayBuffer === "function" && typeof pyodide.setInterruptBuffer === "function") {
    interruptBuffer = new Uint8Array(new SharedArrayBuffer(1));
    pyodide.setInterruptBuffer(interruptBuffer);
  } else {
    interruptBuffer = null;
  }
  updateUi();

  try {
    writeConsole("\n>>> run\n", "system");
    await pyodide.runPythonAsync(addImplicitAwaits(getCode()));
    writeConsole("\n[done]\n", "system");
  } catch (error) {
    if (isStopError(error)) {
      writeConsole("\n[stopped]\n", "system");
    } else {
      writeConsole(`\n${error.message || error}\n`, "error");
    }
  } finally {
    if (typeof pyodide.setInterruptBuffer === "function") {
      pyodide.setInterruptBuffer();
    }
    collectGeneratedFiles(filesBeforeRun);
    interruptBuffer = null;
    stopRequested = false;
    running = false;
    updateUi();
  }
}

function stopPython() {
  if (!running) {
    return;
  }
  stopRequested = true;
  if (interruptBuffer) {
    interruptBuffer[0] = 2;
  }
  writeConsole("\n[stop requested]\n", "system");
}

function clearOutputs() {
  consoleTerm.clear();
  serialBuffer = "";
  outputLog = "";
  generatedFiles.clear();
  renderGeneratedFiles();
  elements.saveOutputButton.disabled = true;
}

function saveOutput() {
  if (!outputLog) {
    return;
  }
  const timestamp = new Date().toISOString().replace(/[:.]/g, "-");
  const blob = new Blob([outputLog], { type: "text/plain;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = `python-lab-session-${timestamp}.txt`;
  document.body.append(link);
  link.click();
  link.remove();
  URL.revokeObjectURL(url);
}

function importCode() {
  elements.importCodeInput.value = "";
  elements.importCodeInput.click();
}

async function handleCodeFileImport() {
  const file = elements.importCodeInput.files?.[0];
  if (!file) {
    return;
  }

  try {
    const text = await file.text();
    currentScriptName = sanitizeFilename(file.name || "python-lab-script.py");
    setCode(text);
    elements.exampleSelect.value = "";
    elements.exampleSelect.title = "Scripts";
    writeConsole(`[Imported ${currentScriptName}]\n`, "system");
  } catch (error) {
    writeConsole(`[Import failed: ${error.message || error}]\n`, "error");
  }
}

function saveCode() {
  const code = getCode();
  const name = sanitizeFilename(currentScriptName || "python-lab-script.py");
  const filename = name.endsWith(".py") ? name : `${name}.py`;
  const blob = new Blob([code], { type: "text/x-python;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = filename;
  document.body.append(link);
  link.click();
  link.remove();
  URL.revokeObjectURL(url);
}

function sanitizeFilename(name) {
  return String(name)
    .trim()
    .replace(/[/\\?%*:|"<>]/g, "-")
    .replace(/\s+/g, "-")
    .replace(/^-+|-+$/g, "")
    || `python-lab-script-${new Date().toISOString().slice(0, 10)}.py`;
}

function snapshotPyodideFiles() {
  if (!pyodide?.FS) {
    return new Map();
  }

  return new Map(listPyodideFiles().map((file) => [file.path, file.signature]));
}

function listPyodideFiles() {
  const fs = pyodide?.FS;
  if (!fs) {
    return [];
  }

  const root = "/home/pyodide";
  const files = [];
  const visit = (dir) => {
    let entries = [];
    try {
      entries = fs.readdir(dir);
    } catch {
      return;
    }

    for (const entry of entries) {
      if (entry === "." || entry === "..") {
        continue;
      }

      const path = `${dir}/${entry}`.replace(/\/+/g, "/");
      let stat = null;
      try {
        stat = fs.stat(path);
      } catch {
        continue;
      }

      if (fs.isDir(stat.mode)) {
        visit(path);
      } else if (fs.isFile(stat.mode)) {
        const relativePath = path.replace(`${root}/`, "");
        const mtime = stat.mtime instanceof Date ? stat.mtime.getTime() : Number(stat.mtime) || 0;
        files.push({
          path,
          name: relativePath,
          size: Number(stat.size) || 0,
          signature: `${Number(stat.size) || 0}:${mtime}`
        });
      }
    }
  };

  visit(root);
  return files;
}

function collectGeneratedFiles(filesBeforeRun) {
  for (const file of listPyodideFiles()) {
    if (filesBeforeRun.get(file.path) !== file.signature) {
      generatedFiles.set(file.path, file);
    }
  }
  renderGeneratedFiles();
}

function renderGeneratedFiles() {
  elements.generatedFilesList.replaceChildren();
  const files = Array.from(generatedFiles.values()).sort((a, b) => a.name.localeCompare(b.name));
  elements.generatedFilesPanel.hidden = files.length === 0;
  if (files.length === 0) {
    fitTerminals();
    return;
  }

  for (const file of files) {
    const item = document.createElement("div");
    item.className = "generated-file";

    const name = document.createElement("span");
    name.className = "generated-file-name";
    name.textContent = file.name;

    const meta = document.createElement("span");
    meta.className = "generated-file-meta";
    meta.textContent = formatBytes(file.size);

    const button = document.createElement("button");
    button.type = "button";
    button.textContent = "Download";
    button.addEventListener("click", () => downloadPyodideFile(file.path, file.name));

    item.append(name, meta, button);
    elements.generatedFilesList.append(item);
  }

  fitTerminals();
}

function downloadPyodideFile(path, name) {
  if (!pyodide?.FS) {
    return;
  }

  let data = null;
  try {
    data = pyodide.FS.readFile(path);
  } catch (error) {
    writeConsole(`\n[Unable to read ${name}: ${error.message || error}]\n`, "error");
    return;
  }

  const blob = new Blob([data], { type: "application/octet-stream" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = name.split("/").pop() || "python-lab-file";
  document.body.append(link);
  link.click();
  link.remove();
  URL.revokeObjectURL(url);
}

function formatBytes(size) {
  if (size < 1024) {
    return `${size} B`;
  }
  if (size < 1024 * 1024) {
    return `${(size / 1024).toFixed(1)} KB`;
  }
  return `${(size / (1024 * 1024)).toFixed(1)} MB`;
}

function checkStopRequested() {
  if (stopRequested) {
    throw new Error("Python run stopped");
  }
}

function isStopError(error) {
  const message = String(error?.message || error || "");
  return stopRequested || message.includes("Python run stopped") || message.includes("KeyboardInterrupt");
}

serial.addEventListener("data", (event) => {
  const text = event.detail || "";
  serialBuffer += text;
  serialVersion += 1;
  if (showRawSerial) {
    writeConsole(text);
  }
});

serial.addEventListener("status", (event) => {
  const { state } = event.detail;
  connected = state === "connected";
  updateUi();
});

serial.addEventListener("error", (event) => {
  writeConsole(`[Serial error: ${event.detail.message}]\n`, "error");
});

elements.connectButton.addEventListener("click", () => {
  if (bpioClient?.connected) {
    void bpioClient.disconnect({ returnToHiZ: true }).finally(() => {
      bpioClient = null;
      updateUi();
      writeConsole("[BPIO2 disconnected]\n", "system");
    });
  } else if (connected) {
    void disconnect();
  } else {
    void connect().catch(() => {});
  }
});
elements.runButton.addEventListener("click", () => void runPython());
elements.stopButton.addEventListener("click", stopPython);
elements.clearButton.addEventListener("click", clearOutputs);
elements.saveOutputButton.addEventListener("click", saveOutput);
elements.importCodeButton.addEventListener("click", importCode);
elements.importCodeInput.addEventListener("change", () => void handleCodeFileImport());
elements.saveCodeButton.addEventListener("click", saveCode);
elements.rawSerialToggle.addEventListener("change", () => {
  showRawSerial = elements.rawSerialToggle.checked;
});
elements.exampleSelect.addEventListener("change", () => {
  const value = elements.exampleSelect.value;
  if (!value) {
    currentScriptName = "python-lab-script.py";
    setCode(STARTER_SCRIPT);
    elements.exampleSelect.title = "Scripts";
    return;
  }
  if (value.startsWith("remote:")) {
    const path = value.slice("remote:".length);
    selectRemoteScript(path);
    void loadRemoteScript(path).catch((error) => {
      writeConsole(`[Script fetch failed: ${error.message || error}]\n`, "error");
      elements.exampleSelect.value = "";
      elements.exampleSelect.title = "Scripts";
    });
    return;
  }
});

window.addEventListener("resize", () => {
  resetSplitIfNeeded();
  fitTerminals();
});
new MutationObserver(() => {
  syncTerminalTheme();
  syncEditorTheme();
}).observe(document.documentElement, {
  attributes: true,
  attributeFilter: ["data-theme"]
});
window.addEventListener("beforeunload", () => {
  if (connected) {
    void serial.disconnect("Page closed");
  }
});

function sleep(ms) {
  return new Promise((resolve, reject) => {
    const deadline = performance.now() + Math.max(0, Number(ms) || 0);
    const tick = () => {
      try {
        checkStopRequested();
      } catch (error) {
        reject(error);
        return;
      }

      const remaining = deadline - performance.now();
      if (remaining <= 0) {
        resolve();
        return;
      }

      window.setTimeout(tick, Math.min(remaining, 50));
    };
    tick();
  });
}

updateUi();
setupSplitDivider();
void loadEditor();
void loadRemoteScriptList();
void loadPython().catch((error) => {
  writeConsole(`[Pyodide failed: ${error.message || error}]\n`, "error");
  updateUi();
});
