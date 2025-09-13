#pragma once

inline const char* style_css = R"rawliteral(

/* Global styles */
* { margin: 0; padding: 0; box-sizing: border-box; overflow-wrap: break-word; }
html, body {
  height: 100%; width: 100%; max-width: 100vw; max-height: 100dvh;
  font-family: Menlo, 'Courier New', Courier, 'Liberation Mono', monospace;
  background-color: #121212; color: #e0e0e0; overflow: hidden; touch-action: manipulation;
}
main {
  display: flex; flex-direction: column; flex-grow: 1; height: 100dvh;
  padding: max(10px, env(safe-area-inset-top)) max(10px, env(safe-area-inset-right)) max(10px, env(safe-area-inset-bottom)) max(10px, env(safe-area-inset-left));
  gap: 10px; overflow: hidden; min-width: 0; max-width: 100%;
}

/* Terminal output */
#output {
  font-family: Menlo, 'Courier New', Courier, 'Liberation Mono', monospace;
  font-size: 0.9rem; letter-spacing: 0; line-height: 1.2;
  flex-grow: 1; min-height: 0; width: 100%;
  background: linear-gradient(145deg, #1a1a1a, #0f0f0f); color: #00ff00;
  border: 1px solid #333; border-radius: 8px; padding: 12px; resize: none;
  box-shadow: inset 0 0 5px #000; overflow-y: auto; overflow-x: hidden; white-space: pre-wrap; word-break: break-word; caret-color: transparent;
}
#output:focus { outline: none; border-color: #00ffcc; box-shadow: 0 0 5px #00ffcc44; }
#output::-webkit-scrollbar { display: none; }

/* Input area */
.input-area {
  display: flex;
  gap: 8px;
  width: 100%;
  flex-shrink: 0;
}
.input-wrap {
  position: relative;
  flex: 1;
}
.input-wrap input[type="text"] {
  width: 100%;
  padding: 10px;
  padding-right: 80px; /* bouton Files */
  background-color: #1c1c1c;
  color: #00ff00;
  border: 1px solid #333;
  border-radius: 8px;
  font-size: 1rem;
  outline: none;
  min-width: 0;
}
#files-btn {
  position: absolute;
  top: 50%;
  right: 10px;
  transform: translateY(-50%);
  height: 28px;
  padding: 0 10px;
  font-size: 0.9rem;
  line-height: 28px;

  display: flex;
  align-items: center;
  gap: 4px;

  border: 1px solid #444;
  border-radius: 6px;
  background: linear-gradient(145deg, #2a2a2a, #1a1a1a);
  color: #ffdc7d;
  cursor: pointer;
}
#files-btn:hover {
  background: #333;
  box-shadow: 0 0 6px #00ff0044;
}
input[type="text"]:focus {
  border-color: #00ffcc;
  box-shadow: 0 0 5px #00ffcc44;
}
button {
  padding: 10px 16px;
  background: linear-gradient(145deg, #2a2a2a, #1a1a1a);
  color: #00ff00;
  border: 1px solid #444;
  border-radius: 8px;
  cursor: pointer;
  font-size: 1rem;
  white-space: nowrap;
  transition: background 0.2s ease, box-shadow 0.2s;
}
button#files-btn {
  padding: 0 10px;     /* override */
  font-size: 0.9rem;   /* override */
}
button:hover {
  background: #333;
  box-shadow: 0 0 6px #00ff0044;
}

/* History */
#history-title { margin: 4px 0 0 0; font-size: 1rem; display: none; flex-shrink: 0; }
.history-area {
  display: flex; flex-wrap: nowrap; gap: 6px; white-space: nowrap; padding-bottom: 4px;
  flex-shrink: 0; width: 100%; max-width: 100%; overflow-x: auto; overflow-y: hidden; min-width: 0;
  -webkit-overflow-scrolling: touch;
}
.history-area::-webkit-scrollbar { width: 0; height: 0; }
.history-area button:hover { background: #2a2a2a; }
.history-area::-webkit-scrollbar { height: 6px; }
.history-area::-webkit-scrollbar-thumb { background-color: #444; border-radius: 4px; }
.history-area::-webkit-scrollbar-track { background-color: #222; }
.history-area button {
  flex: 0 0 auto; background: #1a1a1a; color: #00ff00; border: 1px solid #333; border-radius: 6px;
  padding: 6px 10px; font-size: 0.85em; transition: background 0.2s ease;
}

/* Popup (WS Lost) */
.popup {
  position: fixed; top: 50%; left: 50%; transform: translate(-50%, -50%);
  background: #222; color: #ff5555; padding: 12px 18px; border: 1px solid #444; border-radius: 8px;
  box-shadow: 0 0 8px #000; z-index: 9999; font-size: 0.95rem; max-width: 90vw;
  display: flex; align-items: center; justify-content: center; gap: 10px; white-space: nowrap; overflow: hidden;
}
.popup a { color: #00ffcc; text-decoration: underline; }
.popup a:hover { color: #00ffff; }

/* File Panel */
.fp-overlay {
  position: fixed; inset: 0; background: rgba(0,0,0,0.4); z-index: 9000;
  backdrop-filter: blur(2px);
}
.fp {
  position: fixed; right: 12px; bottom: 12px; width: min(560px, 92vw); height: min(70vh, 560px);
  background: #1a1a1a; border: 1px solid #333; border-radius: 10px; z-index: 9010;
  box-shadow: 0 8px 24px rgba(0,0,0,0.6);
  display: flex; flex-direction: column; gap: 8px; padding: 10px;
  /* for Firefox */
  scrollbar-width: thin;
  scrollbar-color: #666 #1a1a1a;
 }
.fp::-webkit-scrollbar {
  width: 8px;
}
.fp::-webkit-scrollbar-track {
  background: #1a1a1a;
  border-radius: 10px;
}
.fp::-webkit-scrollbar-thumb {
  background-color: #666;
  border-radius: 10px;
  border: 2px solid #1a1a1a;
}
.fp::-webkit-scrollbar-thumb:hover {
  background-color: #888;
}
.fp-header { display:flex; align-items:center; justify-content: space-between; }
.fp-header h2 { font-size: 1.05rem; }
.fp-close {
  background:#2a2a2a;
  color:#ccc;
  border:1px solid #444;
  border-radius:50%;
  width:28px;
  height:28px;
  font-size:1rem;
  display:flex; 
  align-items:center;
  justify-content:center;
  padding:0;
}
.fp-close:hover { background:#333; color:#fff; }
.fp-drop {
  display:flex; flex-direction: column; align-items:center; justify-content:center; gap:6px;
  border: 1px dashed #444; border-radius: 8px; padding: 14px; text-align:center; color:#bbb;
  background: #161616;
}
.fp-drop.drag { border-color: #00ffcc; box-shadow: 0 0 6px #00ffcc44; }
.fp-upload-btn {
  display:inline-block; padding: 8px 10px; background:#2a2a2a; border:1px solid #444; border-radius:8px; cursor:pointer; color:#00ff00;
}
.fp-upload-btn input { display:none; }
.fp-refresh { padding:6px 10px; }
.fp-list {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
  gap: 10px;
  overflow: auto;
  flex: 1;
  border: 1px solid #222;
  border-radius: 8px;
  padding: 10px;
  background:#121212;
  grid-auto-rows: 150px;
}

/* File item */
.fp-item {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  gap: 6px;
  background:#181818;
  border:1px solid #2a2a2a;
  border-radius:8px;
  padding:8px;
  height: 100%;
  min-height: 0;
}
.fp-icon {
  font-size: 28px;
  text-align:center;
}
.fp-name {
  font-size: 0.9rem;
  text-align:center;
  color:#e0e0e0;
  overflow:hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  padding: 5px 0;
}
.fp-meta {
  font-size: 0.8rem;
  text-align:center;
  color:#aaa;
}

/* File actions (Download, Delete) */
.fp-actions {
  display: flex;
  gap: 6px;
  justify-content: center;
  margin-top: 4px;
}
.fp-dl {
  padding: 6px 8px;
  font-size: 0.85rem;
  color:#7dd3fc;                 /* bleu clair */
  border:1px solid #264653;      /* cadre bleu */
  background:#10222b;            /* fond sombre bleu */
  border-radius:6px;
}
.fp-dl:hover { filter: brightness(1.1); }
.fp-del {
  align-self: center;
  padding: 6px 8px;
  font-size: 0.85rem;
  color:#ff6666;
  border:1px solid #553;
  background:#2a1515;
  border-radius: 6px;
}
.fp-del:hover { filter: brightness(1.1); }


/* Responsive */
@media (max-width: 600px), (max-height: 600px) {
  main { padding: 8px; gap: 6px; }
  #output { font-size: 0.8rem; padding: 8px; }
  .input-area { flex-direction: column; gap: 6px; }
  input[type="text"] { font-size: 0.95rem; width: 100%; }
  .input-area button:not(#files-btn) { width: 100%; font-size: 0.95rem; }
  .history-area { gap: 4px; }
  .history-area button { font-size: 0.75em; padding: 5px 8px; }
  .fp { right: 8px; left: 8px; width: auto; height: min(70vh, 520px); }
  .fp-list { grid-auto-rows: 150px; }
}
)rawliteral";
