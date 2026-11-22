#!/usr/bin/env python3
"""Serve the existing native build pipeline over IPv4 loopback."""
import hashlib
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import platform
import shutil
import signal
import subprocess
import sys
import tempfile
import threading

from prepare_disc import EXPECTED_SHA256, EXPECTED_SIZE

ROOT = Path(__file__).resolve().parents[1]
HOST = "127.0.0.1"
PORT = 8765
AUTHORITY = f"{HOST}:{PORT}"
ORIGIN = f"http://{AUTHORITY}"
TARGET = Path("dist/native-n09/mmx4_native_n09.nds")
BUSY = {"uploading", "validating", "setup", "building"}


class InvalidDump(Exception):
    pass


class Builder:
    def __init__(self, workspace):
        self.workspace = workspace
        self.disc = workspace / "disc.bin"
        self.target = workspace / TARGET
        self.lock = threading.Lock()
        self.stop = threading.Event()
        self.worker = None
        self.update("idle", "Aguardando arquivo.")

    def update(self, state, message):
        with self.lock:
            self.job = {"state": state, "message": message}

    def snapshot(self):
        with self.lock:
            return self.job.copy()

    def reserve(self):
        with self.lock:
            if self.job["state"] in BUSY:
                return False
            self.job = {"state": "uploading", "message": "Lendo o dump nesta máquina…"}
            return True

    def receive(self, stream):
        self.target.unlink(missing_ok=True)
        remaining = EXPECTED_SIZE
        digest = hashlib.sha256()
        with self.disc.open("wb") as destination:
            while remaining:
                chunk = stream.read(min(remaining, 1024 * 1024))
                if not chunk:
                    raise InvalidDump("A leitura foi interrompida. Selecione o arquivo novamente.")
                destination.write(chunk)
                digest.update(chunk)
                remaining -= len(chunk)
        if digest.hexdigest() != EXPECTED_SHA256:
            raise InvalidDump("O SHA-256 não corresponde ao dump USA SLUS-00561 suportado.")
        self.update("validating", "Dump conferido. Preparando os arquivos de compilação…")
        self.worker = threading.Thread(target=self.build)
        self.worker.start()

    def run(self, script, *args):
        env = os.environ.copy()
        env.update(X4_DISC=str(self.disc), ASAN_OPTIONS="detect_leaks=0", PYTHONUNBUFFERED="1")
        with subprocess.Popen([sys.executable, script, *args], cwd=self.workspace,
                              env=env, start_new_session=True) as process:
            while not self.stop.is_set():
                try:
                    if process.wait(timeout=0.5):
                        raise RuntimeError(f"{script} terminou com código {process.returncode}")
                    return
                except subprocess.TimeoutExpired:
                    continue
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            process.wait()
            raise RuntimeError("Compilação interrompida")

    def build(self):
        try:
            # The supported BIN hash fixes the layout of its single data track.
            cue = self.disc.with_suffix(".cue")
            cue.write_text('FILE "disc.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n',
                           encoding="ascii")
            self.update("setup", "Verificando o dump e preparando o SDK e as dependências…")
            self.run("tools/setup_native.py", "--disc", str(self.disc), "--cue", str(cue))
            self.update("building", "Compilando e executando os testes. O terminal mostra o progresso.")
            self.run("native/compile.py")
            if not self.target.is_file() or self.target.stat().st_size == 0:
                raise RuntimeError("O compilador não produziu o arquivo .nds")
            state = "done"
            message = "Compilação concluída. Seu arquivo .nds está pronto."
        except (OSError, RuntimeError) as error:
            print(f"Compilação local: {error}", file=sys.stderr, flush=True)
            state = "error"
            message = "A compilação falhou. Confira o erro no terminal e tente novamente."
        finally:
            self.disc.unlink(missing_ok=True)
        self.update(state, message)


class BuilderServer(ThreadingHTTPServer):
    daemon_threads = False

    def __init__(self, address, builder):
        self.builder = builder
        super().__init__(address, Handler)


class Handler(BaseHTTPRequestHandler):
    def setup(self):
        self.request.settimeout(30)
        super().setup()

    def allowed(self):
        return self.headers.get("Host") == AUTHORITY and (
            self.path == "/" or self.headers.get("Sec-Fetch-Site") != "cross-site"
        )

    def respond(self, status, content_type, size, attachment=None):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(size))
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Security-Policy",
                         "default-src 'none'; script-src 'self'; style-src 'self'; "
                         "connect-src 'self'; form-action 'none'; frame-ancestors 'none'")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header("X-Content-Type-Options", "nosniff")
        if attachment:
            self.send_header("Content-Disposition", f'attachment; filename="{attachment}"')
        self.end_headers()

    def reply(self, status, value):
        body = json.dumps(value, ensure_ascii=False).encode("utf-8")
        self.respond(status, "application/json; charset=utf-8", len(body))
        self.wfile.write(body)

    def do_GET(self):
        if not self.allowed():
            self.send_error(HTTPStatus.FORBIDDEN)
            return
        builder = self.server.builder
        if self.path == "/status":
            self.reply(HTTPStatus.OK, builder.snapshot())
        elif self.path == "/download":
            with builder.lock:
                if builder.job["state"] != "done":
                    self.send_error(HTTPStatus.NOT_FOUND)
                    return
                artifact = builder.target.open("rb")
            with artifact:
                size = os.fstat(artifact.fileno()).st_size
                self.respond(HTTPStatus.OK, "application/octet-stream", size, TARGET.name)
                shutil.copyfileobj(artifact, self.wfile)
        else:
            static = {
                "/": ("tools/web/index.html", "text/html"),
                "/builder.js": ("tools/web/builder.js", "text/javascript"),
                "/style.css": ("pages/style.css", "text/css"),
            }
            if self.path not in static:
                self.send_error(HTTPStatus.NOT_FOUND)
                return
            path, media_type = static[self.path]
            body = (ROOT / path).read_bytes()
            self.respond(HTTPStatus.OK, f"{media_type}; charset=utf-8", len(body))
            self.wfile.write(body)

    def do_POST(self):
        if not self.allowed() or self.headers.get("Origin") != ORIGIN:
            self.send_error(HTTPStatus.FORBIDDEN)
            return
        if self.path != "/build":
            self.send_error(HTTPStatus.NOT_FOUND)
            return
        if (self.headers.get_content_type() != "application/octet-stream"
                or self.headers.get("Content-Length") != str(EXPECTED_SIZE)
                or "Transfer-Encoding" in self.headers):
            self.reply(HTTPStatus.BAD_REQUEST, {
                "state": "error",
                "message": "Envie o BIN USA SLUS-00561 de 592.217.136 bytes.",
            })
            return

        builder = self.server.builder
        if not builder.reserve():
            self.reply(HTTPStatus.CONFLICT, builder.snapshot())
            return
        try:
            builder.receive(self.rfile)
        except (InvalidDump, OSError) as error:
            builder.disc.unlink(missing_ok=True)
            if isinstance(error, InvalidDump):
                status, message = HTTPStatus.UNPROCESSABLE_ENTITY, str(error)
            else:
                status = HTTPStatus.INTERNAL_SERVER_ERROR
                message = "Falha na leitura local. Confira o espaço em disco e tente novamente."
                print(error, file=sys.stderr, flush=True)
            builder.update("error", message)
            self.reply(status, builder.snapshot())
            return
        self.reply(HTTPStatus.ACCEPTED, builder.snapshot())


def main():
    if (sys.platform != "linux" or platform.machine() != "x86_64"
            or sys.version_info < (3, 11)):
        raise SystemExit("Use Linux x86-64 ou WSL com Python 3.11 ou mais recente.")
    missing = [name for name in ("git", "gcc", "make", "tar") if not shutil.which(name)]
    if missing:
        raise SystemExit("Ferramentas ausentes na máquina: " + ", ".join(missing))
    with tempfile.TemporaryDirectory(prefix="mmx4-builder-") as directory:
        # Use committed sources in a private workspace, isolated from local builds.
        sources = subprocess.check_output(["git", "archive", "HEAD"], cwd=ROOT)
        subprocess.run(["tar", "-xf", "-", "-C", directory], input=sources, check=True)
        builder = Builder(Path(directory))
        with BuilderServer((HOST, PORT), builder) as server:
            print(f"Abra {ORIGIN}/ no navegador. Encerre com Ctrl+C.", flush=True)
            try:
                server.serve_forever()
            except KeyboardInterrupt:
                pass
            finally:
                builder.stop.set()
        if builder.worker:
            builder.worker.join()


if __name__ == "__main__":
    main()
