#!/usr/bin/env python3
"""Prepare a local .nds using the launcher kit and a user-selected dump."""
import argparse
import json
from contextlib import suppress
import os
from pathlib import Path
import platform
from queue import Empty, Queue
import shutil
import struct
import subprocess
import sys
import tempfile
import threading

KIT = Path(__file__).resolve().parent / "runtime"
NAME = "mmx4_ds_beta.nds"
STAGES = (
    ("Verificando o BIN original (SHA-256)…", "tools/prepare_disc.py"),
    ("Extraindo os arquivos do disco…", "native/prepare_full_content.py"),
    ("Montando o índice de recursos…", "native/prepare_resource_index.py"),
    ("Convertendo música e vozes…", "native/prepare_music.py"),
    ("Preparando os efeitos sonoros…", "native/prepare_sfx.py"),
    ("Preparando as sequências de áudio…", "native/prepare_sequences.py"),
    ("Preparando a tela inferior…", "native/prepare_lower_assets.py"),
    ("Montando o arquivo .nds…", "native/build_n05_flow.py"),
)


def build(disc, output_dir, icon=None, replace=False, progress=print):
    if not disc.is_file():
        raise ValueError("Selecione o arquivo BIN original USA SLUS-00561.")
    destination = output_dir / NAME
    if not output_dir.is_dir():
        raise ValueError("A pasta escolhida não existe.")
    if destination.exists() and not replace:
        raise FileExistsError(f"Já existe um {NAME} na pasta escolhida.")

    with tempfile.TemporaryDirectory(prefix="mmx4-launcher-") as temporary:
        workspace = Path(temporary) / "build"
        shutil.copytree(KIT, workspace)
        suffix = ".exe" if os.name == "nt" else ""
        for executable in ("host/xa_to_pcm", "host/atlas", "ambiente/sdk/tools/bin/ndstool"):
            (workspace / (executable + suffix)).chmod(0o700)
        sys.path.insert(0, str(workspace / "tools"))
        try:
            from prepare_disc import EXPECTED_SIZE
            from nds_header import validate_native_package
        finally:
            sys.path.pop(0)
        if disc.stat().st_size != EXPECTED_SIZE:
            raise ValueError("BIN incompatível. Selecione o dump USA SLUS-00561 de 592.217.136 bytes.")

        # Use a fixed name for the CUE to support arbitrary names in the file picker.
        local_disc = workspace / "disc.bin"
        if os.name == "nt":
            # MinGW tools accept relative UTF-8-independent paths in this workspace.
            try:
                os.link(disc, local_disc)
            except OSError:
                progress("Preparando uma cópia temporária do BIN…")
                shutil.copyfile(disc, local_disc)
        else:
            local_disc.symlink_to(disc)
        (workspace / "disc.cue").write_text(
            'FILE "disc.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n',
            encoding="ascii",
        )
        env = dict(os.environ, X4_DISC=str(local_disc), X4_HOST_TOOLS=str(workspace / "host"),
                   PYTHONUNBUFFERED="1")
        arguments = {
            STAGES[0][1]: [str(local_disc), "--cue", "disc.cue"],
            STAGES[-1][1]: ["--package-engine", "engine.elf"],
        }
        if icon:
            arguments[STAGES[-1][1]].extend(("--icon-bmp", str(icon)))
        with (Path(temporary) / "compile.log").open("w+", encoding="utf-8") as log:
            for message, script in STAGES:
                progress(message)
                completed = subprocess.run(
                    [sys.executable, script, *arguments.get(script, ())], cwd=workspace,
                    env=env, stdout=log, stderr=subprocess.STDOUT,
                )
                if completed.returncode:
                    log.seek(0)
                    error = log.read().strip().splitlines()
                    if script == STAGES[0][1] and any("Dump differs" in line for line in error):
                        raise ValueError("O SHA-256 não corresponde ao dump USA SLUS-00561 suportado.")
                    raise RuntimeError(f"{message}\n" + "\n".join(error[-5:]))

        prepared = workspace / NAME
        validate_native_package(prepared, workspace / "nitrofiles")
        progress("Salvando o arquivo verificado…")
        with tempfile.NamedTemporaryFile(dir=output_dir, prefix=f".{NAME}-", delete=False) as stream:
            pending = Path(stream.name)
            try:
                with prepared.open("rb") as source:
                    shutil.copyfileobj(source, stream, 1024 * 1024)
                stream.close()
                if destination.exists() and not replace:
                    raise FileExistsError(f"Já existe um {NAME} na pasta escolhida.")
                pending.replace(destination)
            finally:
                with suppress(FileNotFoundError):
                    pending.unlink()
    return destination


def gui():
    try:
        import tkinter as tk
        from tkinter import filedialog, messagebox, ttk
        window = tk.Tk()
    except (ImportError, RuntimeError) as error:
        raise RuntimeError("Instale Tkinter ou execute com --dump e --output-dir.") from error
    except tk.TclError as error:
        raise RuntimeError("Abra numa sessão gráfica ou execute com --dump e --output-dir.") from error

    window.title("Megaman X4 Ds")
    window.geometry("460x170")
    frame = ttk.Frame(window, padding=20)
    frame.pack(fill="both", expand=True)
    ttk.Label(frame, text="Selecione seu BIN original USA (SLUS-00561).").pack(anchor="w")
    status = tk.StringVar(value="O arquivo será processado neste aparelho.")
    ttk.Label(frame, textvariable=status, wraplength=415).pack(anchor="w", pady=12)
    bar = ttk.Progressbar(frame, mode="indeterminate")
    events = Queue()

    def poll():
        try:
            while True:
                state, value = events.get_nowait()
                if state == "progress":
                    status.set(value)
                    continue
                bar.stop()
                bar.pack_forget()
                button.pack(anchor="w")
                window.protocol("WM_DELETE_WINDOW", window.destroy)
                status.set("Compilação concluída." if state == "done" else "Falha na compilação.")
                if state == "done":
                    messagebox.showinfo(window.title(), f"Arquivo salvo em:\n{value}", parent=window)
                else:
                    messagebox.showerror(window.title(), value, parent=window)
                return
        except Empty:
            window.after(100, poll)

    def choose():
        disc = filedialog.askopenfilename(parent=window, title="Selecione o dump original",
                                          filetypes=[("Dump do jogo", "*.bin"), ("Todos", "*")])
        if not disc:
            return
        folder = filedialog.askdirectory(parent=window, title="Escolha onde salvar o .nds")
        if not folder:
            return
        target = Path(folder) / NAME
        replace = target.exists()
        if replace and not messagebox.askyesno(window.title(), f"Substituir {target}?", parent=window):
            return

        def run():
            try:
                result = build(Path(disc).resolve(), Path(folder).resolve(), replace=replace,
                               progress=lambda message: events.put(("progress", message)))
            except (OSError, ValueError, RuntimeError) as error:
                events.put(("error", str(error)))
            else:
                events.put(("done", result))

        button.pack_forget()
        bar.pack(fill="x")
        bar.start()
        window.protocol("WM_DELETE_WINDOW", lambda: status.set("Aguarde o término da compilação."))
        threading.Thread(target=run).start()
        window.after(100, poll)

    button = ttk.Button(frame, text="Selecionar BIN e gerar .nds", command=choose)
    button.pack(anchor="w")
    window.mainloop()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dump")
    parser.add_argument("--output-dir")
    parser.add_argument("--icon-bmp")
    parser.add_argument("--replace", action="store_true")
    parser.add_argument("--windows-paths", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if sys.version_info < (3, 11):
        parser.exit(1, "Use Python 3.11 ou superior.\n")
    if not KIT.is_dir():
        parser.exit(1, "Extraia o pacote inteiro antes de abrir o launcher.\n")
    machine = platform.machine().lower()
    target = json.loads((KIT / "platform.json").read_text(encoding="utf-8"))
    supported = (
        (target == "linux-wsl-x64" and sys.platform == "linux" and machine == "x86_64")
        or (target == "windows-x64" and os.name == "nt" and machine in {"amd64", "x86_64"})
        or (target == "android-arm64" and machine == "aarch64"
            and (sys.platform == "android" or "com.termux" in os.environ.get("PREFIX", "")))
    )
    if not supported:
        parser.exit(1, f"Use o pacote para seu sistema. Este pacote é {target}.\n")

    def path(value):
        if args.windows_paths:
            value = subprocess.check_output(["wslpath", "-a", "-u", value], text=True).rstrip("\r\n")
        return Path(value).expanduser().resolve()

    try:
        if args.dump is None and args.output_dir is None and args.icon_bmp is None:
            gui()
        elif args.dump is None or args.output_dir is None:
            parser.error("Use --dump e --output-dir juntos.")
        else:
            icon = path(args.icon_bmp) if args.icon_bmp else None
            result = build(path(args.dump), path(args.output_dir), icon, args.replace,
                           progress=lambda message: print(message, flush=True))
            print(f"Arquivo salvo em {result}.")
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
