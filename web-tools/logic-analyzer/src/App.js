import { downloadBytes } from "../../shared/files/download.js";
import { SumpClient, calculateDivider, calculateEffectiveSampleRate, SUMP_COMPAT_PROFILE } from "./SumpClient.js";
import { captureToSessionJson, createDefaultChannels, samplesToCsv } from "./CaptureModel.js";
import {
  alignCaptureToTrigger,
  calculateTriggerSearchSampleCount,
  findTriggerIndex,
  formatTriggerSummary,
  getTriggerConditions,
} from "./TriggerEngine.js";
import { IframeVcdViewerAdapter } from "./WaveformViewerAdapter.js";
import { writeVcd } from "./VcdWriter.js";

const elements = {
  serialUnsupported: document.querySelector("#serialUnsupported"),
  connectButton: document.querySelector("#connectButton"),
  disconnectButton: document.querySelector("#disconnectButton"),
  captureButton: document.querySelector("#captureButton"),
  cancelButton: document.querySelector("#cancelButton"),
  exportVcdButton: document.querySelector("#exportVcdButton"),
  exportCsvButton: document.querySelector("#exportCsvButton"),
  exportRawButton: document.querySelector("#exportRawButton"),
  exportJsonButton: document.querySelector("#exportJsonButton"),
  connectionStatus: document.querySelector("#connectionStatus"),
  deviceName: document.querySelector("#deviceName"),
  firmwareVersion: document.querySelector("#firmwareVersion"),
  probeCount: document.querySelector("#probeCount"),
  sampleMemory: document.querySelector("#sampleMemory"),
  maxSampleRate: document.querySelector("#maxSampleRate"),
  effectiveRate: document.querySelector("#effectiveRate"),
  sampleRate: document.querySelector("#sampleRate"),
  sampleDepth: document.querySelector("#sampleDepth"),
  channelList: document.querySelector("#channelList"),
  durationEstimate: document.querySelector("#durationEstimate"),
  progress: document.querySelector("#progress"),
  technicalLog: document.querySelector("#technicalLog"),
  viewerFrame: document.querySelector("#viewerFrame"),
};

const sampleRates = [100000, 500000, 1000000, 2000000, 5000000, 10000000, 20000000, 50000000, 100000000, 200000000];

let client = null;
let viewer = null;
let metadata = { ...SUMP_COMPAT_PROFILE };
let channels = createDefaultChannels(8);
let lastCapture = null;
let lastVcd = "";
let busy = false;
let captureCancellationRequested = false;

init();

async function init() {
  setConnectionButtons(false);
  renderOptions();
  renderChannels();
  renderEstimate();

  if (!("serial" in navigator)) {
    elements.serialUnsupported.hidden = false;
    elements.connectButton.disabled = true;
    setStatus("Unsupported browser");
  }

  viewer = new IframeVcdViewerAdapter(elements.viewerFrame);
  try {
    await viewer.initialize();
    syncViewerTheme();
    const demo = makeDemoCapture();
    await viewer.loadCapture(demo);
    lastVcd = writeVcd(demo);
    log("Loaded test capture into local viewer harness.");
  } catch (error) {
    log(`ERROR: ${error.message}`);
  }

  elements.connectButton.addEventListener("click", connect);
  elements.disconnectButton.addEventListener("click", disconnect);
  elements.captureButton.addEventListener("click", capture);
  elements.cancelButton.addEventListener("click", cancelCapture);
  elements.exportVcdButton.addEventListener("click", () => downloadText(lastVcd, "sump-capture.vcd", "text/plain"));
  elements.exportCsvButton.addEventListener("click", () => downloadText(samplesToCsv(lastCapture), "sump-capture.csv", "text/csv"));
  elements.exportRawButton.addEventListener("click", () => downloadBytes(lastCapture.packedSamples, "sump-capture.bin"));
  elements.exportJsonButton.addEventListener("click", () => downloadText(captureToSessionJson(lastCapture), "sump-session.json", "application/json"));
  elements.sampleRate.addEventListener("change", renderEstimate);
  elements.sampleDepth.addEventListener("change", renderEstimate);
  elements.channelList.addEventListener("change", handleChannelControlChange);
  elements.channelList.addEventListener("input", handleChannelControlChange);

  new MutationObserver(syncViewerTheme).observe(document.documentElement, {
    attributes: true,
    attributeFilter: ["data-theme"]
  });
}

function syncViewerTheme() {
  viewer?.setTheme(document.documentElement.dataset.theme === "light" ? "light" : "dark");
}

async function connect() {
  await runOperation("Connecting", async () => {
    const pendingClient = new SumpClient({ log });
    const info = await pendingClient.connect();
    client = pendingClient;
    metadata = info.metadata;
    channels = createDefaultChannels(Math.min(metadata.probeCount ?? 8, 8));
    renderMetadata(metadata);
    renderOptions();
    renderChannels();
    renderEstimate();
    setConnectionButtons(true);
    setStatus("Connected");
  });
}

async function disconnect() {
  await runOperation("Disconnecting", async () => {
    await client?.disconnect();
    client = null;
    lastCapture = null;
    lastVcd = "";
    setConnectionButtons(false);
    setExportButtons(false);
    setStatus("Disconnected");
  });
}

async function capture() {
  await runOperation("Capturing", async () => {
    if (!client) {
      throw new Error("Connect to a SUMP logic analyzer first.");
    }

    const sampleRateHz = Number(elements.sampleRate.value);
    const sampleCount = Number(elements.sampleDepth.value);
    const activeChannels = readChannels();
    const triggerConditions = getTriggerConditions(activeChannels);
    captureCancellationRequested = false;
    elements.cancelButton.disabled = false;
    elements.progress.hidden = false;
    elements.progress.value = 0;

    try {
      const captureResult = triggerConditions.length === 0
        ? await captureSingleBuffer(sampleRateHz, sampleCount, activeChannels)
        : await captureUntilTriggered(sampleRateHz, sampleCount, activeChannels, triggerConditions);

      if (!captureResult) {
        return;
      }
      if (captureCancellationRequested) {
        throw createAbortError();
      }

      setStatus("Processing");
      lastCapture = captureResult;
      elements.effectiveRate.textContent = formatHz(captureResult.sampleRateHz);
      lastVcd = await generateVcd(captureResult);
      await viewer.loadCapture(cloneCaptureForViewer(captureResult));
      setExportButtons(true);
      setStatus("Ready");

      const triggerText = captureResult.triggerIndex == null
        ? ""
        : `, trigger at sample ${captureResult.triggerIndex}`;
      log(`Capture ready: ${captureResult.sampleCount} samples${triggerText}, ${lastVcd.length} VCD chars.`);
    } catch (error) {
      if (error?.name === "AbortError" || captureCancellationRequested) {
        setStatus("Cancelled");
        log("Capture cancelled.");
        return;
      }
      throw error;
    } finally {
      elements.cancelButton.disabled = true;
    }
  });
}

async function captureSingleBuffer(sampleRateHz, sampleCount, activeChannels) {
  setStatus("Configuring");
  return client.capture({
    sampleRateHz,
    sampleCount,
    channels: activeChannels,
    trigger: { type: "none" },
    onProgress(received, expected) {
      setStatus("Receiving samples");
      elements.progress.value = received / expected;
    },
  });
}

async function captureUntilTriggered(sampleRateHz, sampleCount, activeChannels, triggerConditions) {
  let attempt = 0;
  const searchSampleCount = calculateTriggerSearchSampleCount(
    sampleCount,
    metadata.sampleMemoryBytes ?? SUMP_COMPAT_PROFILE.sampleMemoryBytes,
  );
  const latestUsableTriggerIndex = searchSampleCount - sampleCount;

  log(
    `Software trigger armed: ${formatTriggerSummary(triggerConditions)}. `
      + `Searching ${formatCount(searchSampleCount)} samples per pass; output starts at the trigger.`,
  );

  while (!captureCancellationRequested) {
    attempt += 1;
    elements.progress.value = 0;
    setStatus("Waiting trigger");

    const searchCapture = await client.capture({
      sampleRateHz,
      sampleCount: searchSampleCount,
      channels: activeChannels,
      trigger: { type: "none" },
      onProgress(received, expected) {
        setStatus("Waiting trigger");
        elements.progress.value = received / expected;
      },
    });

    const triggerIndex = findTriggerIndex(searchCapture.packedSamples, triggerConditions, {
      maxIndex: latestUsableTriggerIndex,
    });

    if (triggerIndex >= 0) {
      const alignedCapture = alignCaptureToTrigger(searchCapture, triggerIndex, sampleCount);
      log(
        `Trigger matched on capture pass ${attempt} at search sample ${triggerIndex}; `
          + `displayed capture starts at T=0.`,
      );
      return {
        ...alignedCapture,
        config: {
          ...alignedCapture.config,
          trigger: {
            mode: "software",
            alignment: "trigger-at-start",
            conditions: triggerConditions,
            attempts: attempt,
            searchSampleCount,
            outputSampleCount: sampleCount,
            sourceTriggerIndex: triggerIndex,
          },
        },
      };
    }

    if (attempt === 1 || attempt % 10 === 0) {
      log(`Trigger not found after ${attempt} capture pass${attempt === 1 ? "" : "es"}; re-arming.`);
    }
    await delay(0);
  }

  throw createAbortError();
}

async function cancelCapture() {
  if (!busy || elements.cancelButton.disabled) {
    return;
  }

  captureCancellationRequested = true;
  elements.cancelButton.disabled = true;
  setStatus("Cancelling");
  try {
    await client?.cancelCapture();
  } catch (error) {
    log(`WARNING: Could not immediately stop the active capture: ${error.message}`);
  }
}

async function generateVcd(captureResult) {
  if (!window.Worker) {
    return writeVcd(captureResult);
  }

  return new Promise((resolve, reject) => {
    const worker = new Worker("workers/vcd-worker.js", { type: "module" });
    worker.addEventListener("message", (event) => {
      worker.terminate();
      if (event.data.type === "vcd-ready") {
        log(`VCD generated in ${event.data.elapsedMs.toFixed(1)} ms, ${event.data.bytes} bytes.`);
        resolve(event.data.vcd);
      } else {
        reject(new Error(event.data.message));
      }
    });
    worker.addEventListener("error", (event) => {
      worker.terminate();
      reject(new Error(event.message));
    });
    worker.postMessage({ capture: cloneCaptureForWorker(captureResult) });
  });
}

function renderOptions() {
  const maxRate = Math.min(
    metadata.maxSampleRateHz ?? SUMP_COMPAT_PROFILE.maxSampleRateHz,
    metadata.clockHz ?? SUMP_COMPAT_PROFILE.clockHz,
  );
  elements.sampleRate.innerHTML = "";
  for (const rate of sampleRates.filter((rate) => rate <= maxRate)) {
    elements.sampleRate.append(new Option(formatHz(rate), String(rate), rate === 1000000, rate === 1000000));
  }

  const maxDepth = metadata.sampleMemoryBytes ?? SUMP_COMPAT_PROFILE.sampleMemoryBytes;
  const maximumDepth = Math.floor(maxDepth / 4) * 4;
  const maximumTriggeredDepth = getMaximumTriggeredDepth(maximumDepth);
  const depthOptions = new Set(buildSampleDepthOptions(maximumDepth));
  depthOptions.add(maximumTriggeredDepth);

  elements.sampleDepth.innerHTML = "";
  for (const depth of [...depthOptions].sort((left, right) => left - right)) {
    const label = depth === maximumTriggeredDepth
      ? `Trigger max (${formatCount(depth)})`
      : formatCount(depth);
    elements.sampleDepth.append(new Option(label, String(depth)));
  }
  elements.sampleDepth.append(new Option(`Maximum (${formatCount(maximumDepth)})`, String(maximumDepth), true, true));
  updateDepthAvailability();
}

function renderChannels() {
  elements.channelList.innerHTML = "";
  for (const channel of channels) {
    const row = document.createElement("div");
    row.className = "channel-row";
    row.dataset.channelRow = String(channel.index);
    row.innerHTML = `
      <input type="checkbox" data-channel-enabled="${channel.index}" checked aria-label="Enable channel ${channel.index}">
      <input type="text" data-channel-name="${channel.index}" value="${channel.name}" aria-label="Channel ${channel.index} name">
      <select data-channel-role="${channel.index}" aria-label="Channel ${channel.index} role" title="Role">
        <option value="">R</option>
        <option>SDA</option><option>SCL</option><option>RX</option><option>TX</option>
        <option>CLK</option><option>MOSI</option><option>MISO</option><option>CS</option>
      </select>
      <select class="channel-trigger" data-channel-trigger="${channel.index}" aria-label="Channel ${channel.index} trigger" title="Trigger condition">
        <option value="none">— None</option>
        <option value="low">0 Low</option>
        <option value="high">1 High</option>
        <option value="rising">↑ Rising</option>
        <option value="falling">↓ Falling</option>
        <option value="edge">↕ Any edge</option>
      </select>
    `;
    elements.channelList.append(row);
  }
}

function handleChannelControlChange(event) {
  const target = event.target;
  const channelIndex = target.dataset.channelEnabled ?? target.dataset.channelTrigger ?? target.dataset.channelName;
  if (channelIndex == null) {
    return;
  }

  const enabled = document.querySelector(`[data-channel-enabled="${channelIndex}"]`);
  const trigger = document.querySelector(`[data-channel-trigger="${channelIndex}"]`);
  const row = document.querySelector(`[data-channel-row="${channelIndex}"]`);

  if (target.matches("[data-channel-trigger]") && target.value !== "none") {
    enabled.checked = true;
  }

  if (target.matches("[data-channel-enabled]") && !enabled.checked) {
    trigger.value = "none";
  }

  trigger.disabled = !enabled.checked;
  row?.classList.toggle("has-trigger", trigger.value !== "none");
  updateDepthAvailability();
  renderEstimate();
}

function updateDepthAvailability() {
  if (!elements.sampleDepth.options.length) {
    return;
  }

  const triggerActive = [...elements.channelList.querySelectorAll("[data-channel-trigger]")]
    .some((select) => !select.disabled && select.value !== "none");
  const maximumTriggeredDepth = getMaximumTriggeredDepth(
    metadata.sampleMemoryBytes ?? SUMP_COMPAT_PROFILE.sampleMemoryBytes,
  );

  for (const option of elements.sampleDepth.options) {
    option.disabled = triggerActive && Number(option.value) > maximumTriggeredDepth;
  }

  if (triggerActive && Number(elements.sampleDepth.value) > maximumTriggeredDepth) {
    elements.sampleDepth.value = String(maximumTriggeredDepth);
    log(`Depth adjusted to ${formatCount(maximumTriggeredDepth)} samples for trigger-aligned capture.`);
  }
}

function getMaximumTriggeredDepth(sampleMemoryBytes) {
  const memoryCount = Math.max(8, Math.floor(Number(sampleMemoryBytes) / 4) * 4);
  return Math.max(4, Math.floor(memoryCount / 8) * 4);
}

function readChannels() {
  return channels.map((channel) => ({
    ...channel,
    enabled: document.querySelector(`[data-channel-enabled="${channel.index}"]`).checked,
    name: document.querySelector(`[data-channel-name="${channel.index}"]`).value.trim() || `D${channel.index}`,
    role: document.querySelector(`[data-channel-role="${channel.index}"]`).value,
    trigger: document.querySelector(`[data-channel-trigger="${channel.index}"]`).value,
  }));
}

function renderMetadata(info) {
  elements.deviceName.textContent = info.deviceName ?? "-";
  elements.firmwareVersion.textContent = info.firmwareVersion ?? "-";
  elements.probeCount.textContent = String(info.probeCount ?? "-");
  elements.sampleMemory.textContent = formatCount(info.sampleMemoryBytes ?? 0);
  elements.maxSampleRate.textContent = formatHz(info.maxSampleRateHz ?? 0);
}

function renderEstimate() {
  const sampleRateHz = Number(elements.sampleRate.value || 1000000);
  const sampleCount = Number(elements.sampleDepth.value || Math.floor((metadata.sampleMemoryBytes ?? SUMP_COMPAT_PROFILE.sampleMemoryBytes) / 4) * 4);
  const divider = calculateDivider(sampleRateHz, metadata.clockHz);
  const effective = calculateEffectiveSampleRate(metadata.clockHz, divider);
  const ms = (sampleCount / effective) * 1000;
  elements.durationEstimate.textContent = formatDuration(ms);
  elements.durationEstimate.title = `${formatHz(effective)} x ${formatCount(sampleCount)} samples`;
  elements.effectiveRate.textContent = formatHz(effective);
}

async function runOperation(status, task) {
  if (busy) {
    log("Operation already running.");
    return;
  }

  busy = true;
  setBusy(true);
  setStatus(status);
  try {
    await task();
  } catch (error) {
    setStatus("Error");
    log(`ERROR: ${error instanceof Error ? error.message : String(error)}`);
  } finally {
    busy = false;
    setBusy(false);
    elements.progress.hidden = true;
  }
}

function setBusy(isBusy) {
  elements.connectButton.classList.toggle("is-busy", isBusy);
  elements.disconnectButton.classList.toggle("is-busy", isBusy);
  elements.captureButton.classList.toggle("is-busy", isBusy);
  elements.connectButton.disabled = isBusy || Boolean(client);
  elements.disconnectButton.disabled = isBusy || !client;
  elements.captureButton.disabled = isBusy || !client;
}

function setConnectionButtons(connected) {
  elements.connectButton.hidden = connected;
  elements.disconnectButton.hidden = !connected;
  elements.connectButton.disabled = connected;
  elements.disconnectButton.disabled = !connected;
  elements.captureButton.disabled = !connected;
}

function setExportButtons(enabled) {
  for (const button of [elements.exportVcdButton, elements.exportCsvButton, elements.exportRawButton, elements.exportJsonButton]) {
    button.disabled = !enabled;
  }
}

function setStatus(status) {
  elements.connectionStatus.textContent = status;
}

function log(message) {
  const line = `[${new Date().toLocaleTimeString()}] ${message}`;
  elements.technicalLog.textContent += `${line}\n`;
  elements.technicalLog.scrollTop = elements.technicalLog.scrollHeight;
}

function makeDemoCapture() {
  const sampleCount = 256;
  const packedSamples = new Uint8Array(sampleCount);
  for (let index = 0; index < sampleCount; index += 1) {
    packedSamples[index] = ((index >> 2) & 1) | (((index >> 4) & 1) << 1) | (((index >> 5) & 1) << 2);
  }
  return {
    sampleRateHz: 1000000,
    requestedSampleRateHz: 1000000,
    sampleCount,
    triggerIndex: null,
    channels: createDefaultChannels(4),
    packedSamples,
    startedAt: new Date().toISOString(),
    duration: 0,
    deviceMetadata: { deviceName: "Demo capture" },
    config: {},
  };
}

function cloneCaptureForViewer(captureResult) {
  return {
    ...captureResult,
    packedSamples: new Uint8Array(captureResult.packedSamples),
  };
}

function cloneCaptureForWorker(captureResult) {
  return {
    ...captureResult,
    packedSamples: new Uint8Array(captureResult.packedSamples),
  };
}

function downloadText(text, filename, mimeType) {
  downloadBytes(new TextEncoder().encode(text), filename, mimeType);
}

function formatHz(value) {
  if (!value) return "-";
  if (value >= 1000000) return `${trim(value / 1000000)} MHz`;
  if (value >= 1000) return `${trim(value / 1000)} kHz`;
  return `${value} Hz`;
}

function formatCount(value) {
  if (!value) return "-";
  if (value >= 1024) return `${trim(value / 1024)} K`;
  return String(value);
}

function formatDuration(ms) {
  if (!Number.isFinite(ms)) return "-";
  if (ms >= 1000) return `${trim(ms / 1000)} s`;
  if (ms >= 1) return `${trim(ms)} ms`;
  return `${trim(ms * 1000)} us`;
}

function buildSampleDepthOptions(maximumDepth) {
  const depths = [];
  for (let depth = 1024; depth < maximumDepth; depth *= 2) {
    depths.push(depth);
  }
  return depths;
}

function trim(value) {
  return Number.isInteger(value) ? String(value) : value.toFixed(3).replace(/0+$/, "").replace(/\.$/, "");
}

function createAbortError() {
  const error = new Error("Capture cancelled.");
  error.name = "AbortError";
  return error;
}

function delay(ms) {
  return new Promise((resolve) => window.setTimeout(resolve, ms));
}
