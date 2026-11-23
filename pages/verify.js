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
let buildWorker;
let objectUrl;
let verifiedFile;
const buildOptions = document.querySelector("#build-options");
const buildButton = document.querySelector("#build");
const buildStatus = document.querySelector("#build-status");
const download = document.querySelector("#nds-download");
const icon = document.querySelector("#icon");
if (browserBuildAvailable) document.querySelector("#local-only-text").hidden = true;

function clearBuild() {
  buildWorker?.terminate();
  buildWorker = null;
  buildButton.disabled = input.disabled = icon.disabled = false;
  button.disabled = !input.files.length;
  buildStatus.textContent = "";
  if (objectUrl) URL.revokeObjectURL(objectUrl);
  objectUrl = null;
  download.removeAttribute("href");
  download.hidden = true;
}

function stop() {
  worker?.terminate();
  worker = null;
  progress.hidden = true;
  button.disabled = !input.files.length;
}

input.addEventListener("change", () => {
  clearBuild();
  verifiedFile = null;
  buildOptions.hidden = true;
  stop();
  status.textContent = "";
  fingerprint.hidden = true;
});

form.addEventListener("submit", (event) => {
  event.preventDefault();
  const file = input.files[0];
  if (!file || worker) return;

  clearBuild();
  verifiedFile = null;
  buildOptions.hidden = true;
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
          if (data.state === "valid") {
            verifiedFile = file;
            buildOptions.hidden = !browserBuildAvailable;
          }
          status.textContent = data.state === "valid"
            ? (browserBuildAvailable ? "Dump compatível com a beta 9."
              : "Dump compatível com a beta 9. Veja a opção de compilação para computador abaixo.")
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

buildButton.addEventListener("click", () => {
  if (!verifiedFile || buildWorker) return;
  clearBuild();
  buildButton.disabled = input.disabled = icon.disabled = button.disabled = true;
  buildStatus.textContent = "Iniciando a compilação neste aparelho…";

  function failure(message) {
    clearBuild();
    buildStatus.textContent = message;
  }

  try {
    buildWorker = new Worker("./build-worker.mjs", {type: "module"});
    buildWorker.onerror = () => failure("Não foi possível carregar o compilador. Tente novamente.");
    buildWorker.onmessage = ({data}) => {
      if (data.state === "working") {
        buildStatus.textContent = data.message;
      } else if (data.state === "done") {
        clearBuild();
        objectUrl = URL.createObjectURL(data.file);
        download.href = objectUrl;
        download.hidden = false;
        buildStatus.textContent = "Arquivo .nds pronto para baixar.";
      } else {
        failure(data.message?.startsWith("Selecione um BMP") || data.message === "O BMP está incompleto."
          ? data.message : "A compilação foi interrompida. Verifique o arquivo e tente novamente.");
      }
    };
    buildWorker.postMessage({disc: verifiedFile, icon: icon.files[0]});
  } catch {
    failure("Não foi possível iniciar a compilação neste navegador.");
  }
});

stop();
panel.hidden = false;
