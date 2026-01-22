#pragma once

inline const char* scripts_js = R"rawliteral(

let socket = null;
let pendingEchoLines = 0;
let reconnectInterval = 1000; // ms
let responseTimeout = null;
let responseTimeoutDelay = 6000; // ms
let bridgeMode = false;
let fsTotalBytes = 0;
let fsUsedBytes  = 0;
let isUploading  = false;
const filePanel = document.getElementById("file-panel");
const filePanelOverlay = document.getElementById("file-panel-overlay");

/* =========================
   WebSocket / Terminal
   ========================= */

function connectSocket() {
  socket = new WebSocket("ws://" + window.location.host + "/ws");

  socket.onopen = function () {
    hideWsLostPopup();
    bridgeMode = false;
    pendingEchoLines = 0;
    console.log("[WebSocket] Connected");
  };

  socket.onmessage = function (event) {
    const output = document.getElementById("output");
    const lines = event.data.split("\n");

    if (event.data.includes("Bridge: Stopped by user.")) {
      bridgeMode = false;
      console.log("[WebSocket] Bridge mode exited.");
    }

    clearTimeout(responseTimeout);
    hideWsLostPopup(); 

    if (pendingEchoLines > 0) {
      pendingEchoLines -= 1;
      return;
    }

    output.value += lines.join("\n");
    output.scrollTop = output.scrollHeight;
    console.log("[WebSocket] Recv:", event.data);
  };

  socket.onerror = function (error) {
    console.error("[WebSocket] Error:", error);
  };

  socket.onclose = function () {
    console.warn("[WebSocket] Disconnected. Retrying in 1s...");
    showWsLostPopup();
    setTimeout(connectSocket, reconnectInterval);
  };
}

function showWsLostPopup() {
  if (bridgeMode) return;
  const popup = document.getElementById("ws-lost-popup");
  if (popup) popup.style.display = "block";
}
function hideWsLostPopup() {
  const popup = document.getElementById("ws-lost-popup");
  if (popup) popup.style.display = "none";
}

function sendCommand() {
  const input = document.getElementById("command");
  const output = document.getElementById("output");
  const cmd = input.value.trim();

  if (!socket || socket.readyState !== WebSocket.OPEN) return;

  if (cmd === "bridge" || cmd === "keyboard") {
    bridgeMode = true;
    clearTimeout(responseTimeout);
    hideWsLostPopup();
    console.log("[WebSocket] Bridge Mode");
    pendingEchoLines = cmd.length
    socket.send(cmd + "\n");
    input.value = "";
    addToHistory(cmd);
    output.value += cmd + "\n";
    return;
  }

  clearTimeout(responseTimeout);
  responseTimeout = setTimeout(() => {
    console.warn("[WebSocket] No response after command.");
    showWsLostPopup();
  }, responseTimeoutDelay);

  socket.send(cmd + "\n");
  input.value = "";

  if (!bridgeMode && !/^\d+$/.test(cmd)) {
    output.value += cmd;
    addToHistory(cmd);
    pendingEchoLines = cmd.length;
  } else {
    pendingEchoLines = 0;
  }
}

window.addEventListener("DOMContentLoaded", function () {
  const input = document.getElementById("command");
  const output = document.getElementById("output");

  output.value = 
`  ____                    _           _       
 | __ ) _   _ ___   _ __ (_)_ __ __ _| |_ ___ 
 |  _ \\| | | / __| | '_ \\| | '__/ _\` | __/ _ \\
 | |_) | |_| \\__ \\ | |_) | | | | (_| | ||  __/
 |____/ \\__,_|___/ | .__/|_|_|  \\__,_|\\__\\___|
                   |_|                        
     Version 1.3           Ready to board

 Type 'mode' to start or 'help' for commands

HIZ> `;

  input.addEventListener("keydown", function (event) {
    if (event.key === "Enter") { event.preventDefault(); sendCommand(); return; }
    if (event.key === "Escape") { if (bridgeMode) { event.preventDefault(); socket.send("\x1B"); } return; }
    if (event.ctrlKey && event.key.toLowerCase() === "c") { if (bridgeMode) { event.preventDefault(); socket.send("\x03"); } return; }
    if (event.key === "Tab") { if (bridgeMode) { event.preventDefault(); socket.send("\x09"); } return; }
    if (event.ctrlKey && event.key.toLowerCase() === "d") { if (bridgeMode) { event.preventDefault(); socket.send("\x04"); } return; }
    if (event.ctrlKey && event.key.toLowerCase() === "z") { if (bridgeMode) { event.preventDefault(); socket.send("\x1A"); } return; }
    if (event.ctrlKey && event.key.toLowerCase() === "x") { if (bridgeMode) { event.preventDefault(); socket.send("\x18"); } return; }
  });

  // Files button
  const filesBtn = document.getElementById("files-btn");
  if (filesBtn) filesBtn.addEventListener("click", openFilePanel);

  connectSocket();
});

/* =========================
   Command History
   ========================= */
function addToHistory(cmd) {
  if (!isValidCommand(cmd)) return;
  const history = document.getElementById("history");
  const last = history.firstChild;
  if (last && last.textContent === cmd) return;

  const btn = document.createElement("button");
  const maxLength = 15;
  const displayText = cmd.length > maxLength ? cmd.slice(0, maxLength - 3) + "..." : cmd;
  btn.textContent = displayText;
  btn.title = cmd;
  btn.onclick = () => {
    document.getElementById("command").value = cmd;
    document.getElementById("command").focus();
  };
  history.insertBefore(btn, history.firstChild);
}
function isValidCommand(cmd) {
  if (!cmd) return false;
  if (cmd.length < 2) return false;
  if (/^\d+$/.test(cmd)) return false;
  return true;
}

/* =========================
   File Panel
   ========================= */
  
function onOverlayClick(e) {
  if (e.target === filePanelOverlay && !isUploading) closeFilePanel();
}

function openFilePanel() {
  document.getElementById("file-panel-overlay").style.display = "block";
  document.getElementById("file-panel").style.display = "flex";
  filePanelOverlay.addEventListener("click", onOverlayClick);
  refreshFileList();
}

function closeFilePanel() {
  document.getElementById("file-panel-overlay").style.display = "none";
  document.getElementById("file-panel").style.display = "none";
}

async function refreshFileList() {
  const listEl = document.getElementById("file-list");
  listEl.innerHTML = "<div class='fp-empty'>Loading...</div>";
  try {
    const r = await fetch("/littlefs/list?dir=/", { cache: "no-store" });
    const j = await r.json();
    fsTotalBytes = Number(j.total) || 0;
    fsUsedBytes  = Number(j.used)  || 0;

    // update space info
    const totalKB = Math.floor(j.total / 1024);
    const usedKB  = Math.floor(j.used / 1024);
    const percent = totalKB > 0 ? Math.round((usedKB / totalKB) * 100) : 0;
    document.getElementById("fp-space").textContent =
      `Total: ${totalKB} KB — Used: ${usedKB} KB (${percent}%)`;
    
    // Get files
    const entries = (j && Array.isArray(j.entries)) ? j.entries : [];
    const files = entries.filter(e => !e.isDir);
    if (files.length === 0) {
      listEl.innerHTML = "<div class='fp-empty'>No files found.</div>";
    } else {
      renderFileList(files);
    }
  } catch (e) {
    listEl.innerHTML = "<div class='fp-empty'>Failed to load.</div>";
    console.error(e);
  }
}

function renderFileList(entries) {
  const listEl = document.getElementById("file-list");
  listEl.innerHTML = "";
  if (!Array.isArray(entries) || entries.length === 0) {
    listEl.innerHTML = "<div class='fp-empty'>No files yet.</div>";
    return;
  }
  entries
    .filter(e => !e.isDir)
    .sort((a,b)=> a.name.localeCompare(b.name))
    .forEach(e => {
      const card = document.createElement("div");
      card.className = "fp-item";

      const icon = document.createElement("div");
      icon.className = "fp-icon";
      icon.textContent = "📄";
      icon.title = e.name;

      const name = document.createElement("div");
      name.className = "fp-name";
      name.textContent = e.name;

      const meta = document.createElement("div");
      meta.className = "fp-meta";
      meta.textContent = humanSize(e.size);

      const actions = document.createElement("div");
      actions.className = "fp-actions";

      const dl = document.createElement("button");
      dl.className = "fp-dl";
      dl.textContent = "💾";
      dl.title = "Download file";
      dl.onclick = (ev) => {
        ev.stopPropagation();
        downloadFile(e.name);
      };

      const del = document.createElement("button");
      del.className = "fp-del";
      del.textContent = "🗑️";
      del.title = "Delete file";
      del.onclick = async (ev) => {
        ev.stopPropagation();
        await deleteFile(e.name);
      };

      actions.appendChild(dl);
      actions.appendChild(del);

      const topWrap = document.createElement("div");
      topWrap.style.display = "flex";
      topWrap.style.flexDirection = "column";
      topWrap.style.gap = "4px";
      topWrap.appendChild(icon);
      topWrap.appendChild(name);
      topWrap.appendChild(meta);

      card.appendChild(topWrap);
      card.appendChild(actions);
      listEl.appendChild(card);
    });
}

function humanSize(n) {
  const units = ["B","KB","MB","GB"];
  let i = 0, s = Number(n);
  while (s >= 1024 && i < units.length - 1) { s /= 1024; i++; }
  return (i===0 ? s.toString() : s.toFixed(1)) + " " + units[i];
}

/* Upload */

function onDropOver(e) {
  if (isUploading) return; // ignore
  e.preventDefault();
  e.dataTransfer.dropEffect = "copy";
  document.getElementById("dropzone").classList.add("drag");
}
function onDropLeave(e) {
  e.preventDefault();
  document.getElementById("dropzone").classList.remove("drag");
}
async function onDrop(e) {
  e.preventDefault();
  if (isUploading) return;
  document.getElementById("dropzone").classList.remove("drag");
  const files = e.dataTransfer.files;
  if (!files || !files.length) return;
  await uploadFile(files[0]);
}

async function onFileInput(e) {
  const f = e.target.files && e.target.files[0];
  if (!f) return;
  await uploadFile(f);
  e.target.value = "";
}

function disableDropArea(disabled) {
  const dz  = document.getElementById("dropzone");
  const inp = document.getElementById("file-input");
  if (!dz || !inp) return;
  if (disabled) {
    dz.classList.add("disabled");
    inp.disabled = true;
  } else {
    dz.classList.remove("disabled");
    inp.disabled = false;
  }
}

/* Upload a single file */

async function uploadFile(file) {
  const titleEl = document.getElementById("fp-header-title");
  const originalTitle = titleEl.textContent;

  // Free space check
  const FREE_MARGIN = 1024; // 1 KB de marge
  const freeBytes = Math.max(0, fsTotalBytes - fsUsedBytes - FREE_MARGIN);

  // Size
  if (file.size > freeBytes) {
    const needKB = Math.ceil(file.size / 1024);
    const freeKB = Math.floor(freeBytes / 1024);
    alert(`Not enough space.\nNeeded: ${needKB} KB\nFree: ${freeKB} KB`);
    return;
  }

  try {
    isUploading = true;
    disableDropArea(true);
    titleEl.textContent = "⏳ Upload... Please wait";

    const r = await fetch("/littlefs/upload?file=" + encodeURIComponent(file.name), {
      method: "POST",
      headers: { "Content-Type": "application/octet-stream" },
      body: file
    });

    if (!r.ok) throw new Error("Upload failed");

    await refreshFileList();
  } catch (e) {
    alert("Upload error: " + e.message);
    console.error(e);
  } finally {
    titleEl.textContent = originalTitle;
    disableDropArea(false);
    isUploading = false;
  }
}

/* Download */

function downloadFile(name) {
  // Content-Type binaire Content-Disposition: attachment
  const url = "/littlefs/download?file=" + encodeURIComponent(name);
  window.open(url, "_blank");
}

/* Delete */

async function deleteFile(name) {
  if (!confirm(`Delete '${name}' ?`)) return;
  try {
    const url = "/littlefs/delete?file=" + encodeURIComponent(name);
    const r = await fetch(url, { method: "DELETE" });
    if (!r.ok) throw new Error("Delete failed");
    await refreshFileList();
  } catch (e) {
    alert("Delete error: " + e.message);
    console.error(e);
  }
}
)rawliteral";
