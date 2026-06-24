export class WaveformViewerAdapter {
  async initialize() {}
  async loadCapture(_capture) {}
  async loadVcd(_data) {}
  reset() {}
  setTheme(_theme) {}
  setChannelNames(_channels) {}
  setTriggerMarker(_sampleIndex, _sampleRateHz) {}
  destroy() {}
}

export class IframeVcdViewerAdapter extends WaveformViewerAdapter {
  constructor(iframe, { readyTimeoutMs = 3000 } = {}) {
    super();
    this.iframe = iframe;
    this.readyTimeoutMs = readyTimeoutMs;
    this.ready = false;
    this.messageHandler = (event) => {
      if (event.source === this.iframe.contentWindow && event.data?.type === "viewer-ready") {
        this.ready = true;
      }
    };
  }

  async initialize() {
    window.addEventListener("message", this.messageHandler);
    const deadline = performance.now() + this.readyTimeoutMs;
    while (!this.ready && performance.now() < deadline) {
      await delay(50);
    }
    if (!this.ready) {
      throw new Error("Waveform viewer could not be loaded.");
    }
  }

  async loadCapture(capture) {
    this.post({ type: "load-capture", capture: serializeCaptureForPostMessage(capture) }, [capture.packedSamples.buffer]);
  }

  async loadVcd(data) {
    this.post({ type: "load-vcd", data });
  }

  reset() {
    this.post({ type: "reset" });
  }

  setTheme(theme) {
    this.post({ type: "set-theme", theme });
  }

  setTriggerMarker(sampleIndex, sampleRateHz) {
    this.post({ type: "set-trigger-marker", sampleIndex, sampleRateHz });
  }

  destroy() {
    window.removeEventListener("message", this.messageHandler);
  }

  post(message, transfer = []) {
    this.iframe.contentWindow.postMessage(message, "*", transfer);
  }
}

function serializeCaptureForPostMessage(capture) {
  return {
    ...capture,
    packedSamples: capture.packedSamples,
  };
}

function delay(ms) {
  return new Promise((resolve) => window.setTimeout(resolve, ms));
}
