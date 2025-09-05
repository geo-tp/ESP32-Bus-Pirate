const flashEl   = document.getElementById("flashEl");   // <esp-web-install-button>
const flashBtn  = document.getElementById("flashBtn");  // slotted button
const flashHint = document.getElementById("flashHint");
const grid      = document.getElementById("boards");

let selected = null;

function setActiveCard(key) {
  const cards = grid.querySelectorAll(".card");
  for (const el of cards) {
    const on = !!key && el.dataset.key === key;
    el.classList.toggle("active", on);
    el.setAttribute("aria-selected", on ? "true" : "false");
  }
}

function setDisabled(state) {
  if (state) {
    flashBtn.disabled = true;
    flashBtn.setAttribute("disabled", "");
  } else {
    flashBtn.disabled = false;
    flashBtn.removeAttribute("disabled");
  }
}

function updateUI() {
  if (!selected) {
    flashEl.removeAttribute("manifest");
    setDisabled(true);
    flashHint.textContent = "Select a board above.";
    setActiveCard(null);
    return;
  }
  const manifestUrl = `./manifests/${selected}.json`;
  flashEl.setAttribute("manifest", manifestUrl);
  setDisabled(false);
  flashHint.textContent = `Ready: Click Flash.`;
  setActiveCard(selected);
}

// Clic board
grid.addEventListener("click", (e) => {
  const el = e.target.closest(".card");
  if (!el) return;
  selected = el.dataset.key || null;
  updateUI();
});

// Init
updateUI();