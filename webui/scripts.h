#pragma once

inline const char* scripts_js = R"rawliteral(

let socket = null;
let pendingEchoLines = 0;
let reconnectInterval = 1000; // ms
let responseTimeout = null;
let responseTimeoutDelay = 6000; // ms
let bridgeMode = false;

function connectSocket() {
  socket = new WebSocket("ws://" + window.location.host + "/ws");

  socket.onopen = function () {
    hideWsLostPopup(); // Supprime le popup
    bridgeMode = false;
    pendingEchoLines = 0;
    console.log("[WebSocket] Connected");
  };

  socket.onmessage = function (event) {

    const output = document.getElementById("output");
    const lines = event.data.split("\n");

    // Bridge mode specific
    if (event.data.includes("Bridge: Stopped by user.")) {
      bridgeMode = false;
      console.log("[WebSocket] Bridge mode exited.");
    }

    // Clear response timeout
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
    showWsLostPopup(); // Affiche le popup
    setTimeout(connectSocket, reconnectInterval);
  };
}

function showWsLostPopup() {
  if (bridgeMode) return;
  const popup = document.getElementById("ws-lost-popup");
  if (popup) {
    popup.style.display = "block";
  }
}

function hideWsLostPopup() {
  const popup = document.getElementById("ws-lost-popup");
  if (popup) {
    popup.style.display = "none";
  }
}

function sendCommand() {
  const input = document.getElementById("command");
  const output = document.getElementById("output");
  const cmd = input.value.trim();

  // No socket
  if (socket.readyState !== WebSocket.OPEN) return;

  // Bridge mode start
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
    return; // enter bridge mode
  }

  // Set timeout in non-bridge mode
  clearTimeout(responseTimeout);
  responseTimeout = setTimeout(() => {
    console.warn("[WebSocket] No response after command.");
    showWsLostPopup();
  }, responseTimeoutDelay);

  // Normal Send
  socket.send(cmd + "\n");
  input.value = "";

  // Don't save the cmd if it's just a number, unless we're in bridge mode
  if (!bridgeMode && !/^\d+$/.test(cmd)) {
    output.value += cmd;
    addToHistory(cmd);
    pendingEchoLines = cmd.length;
  } else {
      pendingEchoLines = 0;
  }
}

// Bind
window.addEventListener("DOMContentLoaded", function () {
  const input = document.getElementById("command");
  const output = document.getElementById("output");

  // Initialize terminal
  output.value = 
`  ____                    _           _       
 | __ ) _   _ ___   _ __ (_)_ __ __ _| |_ ___ 
 |  _ \\| | | / __| | '_ \\| | '__/ _\` | __/ _ \\
 | |_) | |_| \\__ \\ | |_) | | | | (_| | ||  __/
 |____/ \\__,_|___/ | .__/|_|_|  \\__,_|\\__\\___|
                   |_|                        
     Version 0.8           Ready to board

 Type 'mode' to start or 'help' for commands

HIZ> `;

  // Key events
  input.addEventListener("keydown", function (event) {
    // --- ENTER ---
    if (event.key === "Enter") {
      event.preventDefault();
      sendCommand();
      return;
    }

    // --- ESCAPE (0x1B) ---
    if (event.key === "Escape") {
      if (bridgeMode) {
        event.preventDefault();
        socket.send("\x1B");
      }
      return;
    }

    // --- CTRL+C (0x03) ---
    if (event.ctrlKey && event.key.toLowerCase() === "c") {
      if (bridgeMode) {
        event.preventDefault();
        socket.send("\x03");
      }
      return;
    }
    
    // --- TAB (0x09) ---
    if (event.key === "Tab") {
      if (bridgeMode) {
        event.preventDefault();
        socket.send("\x09");
      }
      return;
    }

    // --- CTRL+D -----
    if (event.ctrlKey && event.key.toLowerCase() === "d") {
      if (bridgeMode) {
        event.preventDefault();
        socket.send("\x04");
      }
      return;
    }
    
    // --- CTRL+Z -----
    if (event.ctrlKey && event.key.toLowerCase() === "z") {
      if (bridgeMode) {
        event.preventDefault();
        socket.send("\x1A");
      }
      return;
    }

    // --- CTRL+X -----
    if (event.ctrlKey && event.key.toLowerCase() === "x") {
      if (bridgeMode) {
        event.preventDefault();
        socket.send("\x18");
      }
      return;
    }

  });

  connectSocket(); // Initial connection
});

function addToHistory(cmd) {
  if (!isValidCommand(cmd)) return;
  const history = document.getElementById("history");

  const last = history.firstChild;
  if (last && last.textContent === cmd) return;

  const btn = document.createElement("button");
  
  // Limiter l'affichage du texte à 15 caractères
  const maxLength = 15;
  const displayText = cmd.length > maxLength ? cmd.slice(0, maxLength - 3) + "..." : cmd;

  btn.textContent = displayText;
  btn.title = cmd; // Tooltip complet au survol
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

)rawliteral";
