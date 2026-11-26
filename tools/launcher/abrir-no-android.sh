#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"

if ! command -v python3 >/dev/null; then
    echo "Instale Python no Termux com: pkg install python"
    exit 1
fi
downloads="$HOME/storage/downloads"
if [[ ! -d "$downloads" || ! -w "$downloads" ]]; then
    echo "Autorize o acesso à pasta Downloads com: termux-setup-storage"
    exit 1
fi
if [[ $# -gt 1 ]]; then
    echo "Uso: bash abrir-no-android.sh [caminho do BIN]"
    exit 1
fi
if [[ -f "$downloads/mmx4_ds_beta.nds" ]]; then
    read -r -p "Substituir mmx4_ds_beta.nds em Downloads? [s/N] " answer
    [[ "$answer" == [sS] ]] || exit 0
    replace=(--replace)
else
    replace=()
fi
if [[ $# -eq 1 ]]; then
    python3 launcher.py --dump "$1" --output-dir "$downloads" "${replace[@]}"
else
    if ! command -v termux-storage-get >/dev/null; then
        echo "Para abrir o seletor, instale o app Termux:API e rode: pkg install termux-api"
        exit 1
    fi
    selection=$(mktemp -d)
    trap 'rm -f -- "$selection/disc.bin"; rmdir -- "$selection"' EXIT
    echo "Selecione seu BIN original. Aguarde a leitura após fechar o seletor."
    termux-storage-get "$selection/disc.bin"
    if [[ ! -s "$selection/disc.bin" ]]; then
        echo "Nenhum arquivo foi selecionado."
        exit 1
    fi
    python3 launcher.py --dump "$selection/disc.bin" --output-dir "$downloads" "${replace[@]}"
fi
