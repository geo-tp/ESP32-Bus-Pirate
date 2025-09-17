#pragma once

inline const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Bus Pirate</title>
  <link rel="stylesheet" href="/style.css">
  <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'%3E%3Ctext x='50%25' y='50%25' text-anchor='middle' dominant-baseline='central' font-family='sans-serif' font-size='56' font-weight='900' fill='%2300ff00'%3EB%3C/text%3E%3C/svg%3E">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover, user-scalable=no">
</head>
<body>

  <!-- Main Terminal Area -->
  <main>
    <!-- Output area -->
    <textarea id="output" readonly></textarea>
    <!-- Input area -->
    <div class="input-area">
      <div class="input-wrap">
        <input type="text" id="command" placeholder="Enter command"
              autocapitalize="off" autocomplete="off" autocorrect="off" spellcheck="false">
        <button id="files-btn" type="button" title="Manage files">📁 Files</button>
      </div>
      <div class="actions">
        <button onclick="sendCommand()">Send</button>
      </div>
    </div>
    <h3 id="history-title" style="display: none;">Command History</h3>
    <div id="history" class="history-area"></div>
  </main>

  <!-- WebSocket Lost -->
  <div id="ws-lost-popup" class="popup" style="display: none;">
    <span class="popup-text">Connection lost.</span>
    <a href="#" onclick="location.reload()">Refresh</a>
  </div>

  <!-- File Panel -->
  <div id="file-panel-overlay" class="fp-overlay" style="display:none;"></div>
  <div id="file-panel" class="fp" style="display:none;"
        ondragover="onDropOver(event)"
        ondragleave="onDropLeave(event)"
        ondrop="onDrop(event)">
    <!-- Header -->
    <div class="fp-header">
      <h2 id="fp-header-title">📁 LittleFS</h2>
      <button class="fp-close" type="button" aria-label="Close" onclick="closeFilePanel()">×</button>
    </div>
    <!-- Drag Drop -->
    <div id="dropzone" class="fp-drop"
         ondragover="onDropOver(event)"
         ondragleave="onDropLeave(event)"
         ondrop="onDrop(event)">
      <p>Drag & drop a file here, or</p>
      <label class="fp-upload-btn">
        <input id="file-input" type="file" onchange="onFileInput(event)" />
        Choose file
      </label>
    </div>

    <!-- File list -->
    <div class="fp-list-header">
      <span id="fp-space">Loading...</span>
    </div>
    <div id="file-list" class="fp-list"></div>
  </div>

  <script src="/scripts.js"></script>
</body>
</html>
)rawliteral";
