"use strict";

const panel = document.querySelector("#verify-panel");
const form = document.querySelector("#verify");
const input = document.querySelector("#dump");
const button = form.querySelector("button");
const progress = document.querySelector("#hash-progress");
const status = document.querySelector("#hash-status");
const fingerprint = document.querySelector("#fingerprint");
const hashValue = fingerprint.querySelector("code");
let worker;

function stop() {
  worker?.terminate();
  worker = null;
  progress.hidden = true;
  button.disabled = !input.files.length;
}

input.addEventListener("change", () => {
  stop();
  status.textContent = "";
  fingerprint.hidden = true;
});

form.addEventListener("submit", (event) => {
  event.preventDefault();
  const file = input.files[0];
  if (!file || worker) return;

  status.textContent = "Verificando o arquivo na sua máquina…";
  fingerprint.hidden = true;
  button.disabled = true;
  progress.value = 0;
  progress.hidden = false;

  const failed = () => {
    stop();
    status.textContent = "A leitura foi interrompida. Selecione o arquivo e tente novamente.";
  };

  try {
    worker = new Worker("./verify-worker.js");
    worker.onerror = failed;
    worker.onmessage = ({data}) => {
      switch (data.state) {
        case "progress":
          progress.value = data.progress;
          break;
        case "valid":
        case "hashMismatch":
          stop();
          status.textContent = data.state === "valid"
            ? "Dump compatível com a beta 9. Veja a opção de compilação para computador abaixo."
            : "O SHA-256 difere da edição USA SLUS-00561 suportada.";
          hashValue.textContent = data.sha256;
          fingerprint.hidden = false;
          break;
        case "sizeMismatch":
          stop();
          status.textContent = `Selecione um BIN de ${data.bytes.toLocaleString("pt-BR")} bytes.`;
          break;
        default:
          failed();
      }
    };
    worker.postMessage(file);
  } catch {
    failed();
  }
});

stop();
panel.hidden = false;
