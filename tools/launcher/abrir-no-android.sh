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
generate() {
    python3 launcher.py --dump "$1" --output-dir "$downloads" "${replace[@]}"
}

if [[ $# -eq 1 ]]; then
    generate "$1"
elif command -v termux-storage-get >/dev/null; then
    selection=$(mktemp -d)
    trap 'rm -f -- "$selection/disc.bin"; rmdir -- "$selection"' EXIT
    echo "Selecione seu BIN original. Aguarde a leitura após fechar o seletor."
    termux-storage-get "$selection/disc.bin"
    if [[ ! -s "$selection/disc.bin" ]]; then
        echo "Nenhum arquivo foi selecionado."
        exit 1
    fi
    generate "$selection/disc.bin"
else
    shopt -s nullglob
    dumps=()
    for file in "$downloads"/*.[bB][iI][nN]; do
        if [[ -f "$file" ]]; then
            dumps+=("${file##*/}")
        fi
    done
    if [[ ${#dumps[@]} -eq 0 ]]; then
        echo "Coloque seu BIN original em Downloads ou passe o caminho como argumento."
        exit 1
    fi
    PS3="Selecione seu BIN pelo número: "
    select dump in "${dumps[@]}" "Cancelar"; do
        if [[ -z "$dump" ]]; then
            echo "Escolha um dos números da lista."
        elif [[ "$dump" == "Cancelar" ]]; then
            exit 0
        else
            generate "$downloads/$dump"
            break
        fi
    done
fi
