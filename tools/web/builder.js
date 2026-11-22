"use strict";

const form = document.querySelector("#builder");
const discInput = document.querySelector("#disc");
const button = form.querySelector("button");
const status = document.querySelector("#status");
const download = document.querySelector("#download");
const busyStates = new Set(["uploading", "validating", "setup", "building"]);
let downloadOnSuccess = false;
let pollTimer;

function show(job) {
  status.textContent = job.message;
  download.hidden = job.state !== "done";
  discInput.disabled = button.disabled = busyStates.has(job.state);
  if (!download.hidden && downloadOnSuccess) {
    downloadOnSuccess = false;
    download.click();
  }
}

async function poll() {
  try {
    const response = await fetch("/status", {cache: "no-store"});
    if (!response.ok) throw new Error();
    const job = await response.json();
    show(job);
    if (busyStates.has(job.state)) pollTimer = setTimeout(poll, 1200);
  } catch {
    status.textContent = "O compilador local está indisponível. Confira o terminal e recarregue a página.";
    discInput.disabled = button.disabled = true;
    download.hidden = true;
  }
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const disc = discInput.files[0];
  if (!disc) return;
  if (disc.size !== 592217136) {
    show({state: "error", message: "Selecione o BIN USA SLUS-00561 de 592.217.136 bytes."});
    return;
  }

  clearTimeout(pollTimer);
  show({state: "uploading", message: "Lendo o dump nesta máquina…"});
  try {
    const response = await fetch("/build", {
      method: "POST",
      headers: {"Content-Type": "application/octet-stream"},
      body: disc,
    });
    const job = await response.json();
    downloadOnSuccess = response.status === 202;
    show(job);
    if (busyStates.has(job.state)) pollTimer = setTimeout(poll, 1200);
  } catch {
    show({state: "error", message: "A leitura foi interrompida. Confira o terminal e tente novamente."});
  }
});

void poll();
