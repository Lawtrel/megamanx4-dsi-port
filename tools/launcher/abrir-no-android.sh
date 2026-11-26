#!/usr/bin/env bash
set -euo pipefail

if [[ $# -gt 1 ]]; then
    echo "Uso: bash abrir-no-android.sh [caminho do BIN]"
    exit 1
fi
if [[ $# -eq 1 ]]; then
    requested=$1
    if [[ "$requested" != /* ]]; then
        requested="$PWD/$requested"
    fi
fi
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

generate() {
    local answer
    local replace=()
    if [[ -f "$downloads/mmx4_ds_beta.nds" ]]; then
        read -r -p "Substituir mmx4_ds_beta.nds em Downloads? [s/N] " answer || return 0
        [[ "$answer" == [sS] ]] || return 0
        replace=(--replace)
    fi
    python3 launcher.py --dump "$1" --output-dir "$downloads" "${replace[@]}"
}

select_from_android() (
    selection=$(mktemp -d)
    trap 'rm -f -- "$selection/disc.bin"; rmdir -- "$selection"' EXIT
    echo "Selecione seu BIN original. Aguarde a leitura após fechar o seletor."
    termux-storage-get "$selection/disc.bin"
    if [[ ! -s "$selection/disc.bin" ]]; then
        echo "Nenhum arquivo foi selecionado."
        exit 1
    fi
    generate "$selection/disc.bin"
)

if [[ $# -eq 1 ]]; then
    generate "$requested"
    exit
fi

shopt -s nullglob
dumps=()
for file in "$downloads"/*.[bB][iI][nN]; do
    if [[ -f "$file" ]]; then
        dumps+=("${file##*/}")
    fi
done

other_location='Escolher em outra pasta'
if [[ ${#dumps[@]} -eq 0 ]]; then
    if command -v termux-storage-get >/dev/null; then
        select_from_android
        exit
    fi
    echo "Coloque seu BIN original em Downloads ou passe o caminho como argumento."
    exit 1
fi
if command -v termux-storage-get >/dev/null; then
    dumps+=("$other_location")
fi

PS3="Selecione seu BIN pelo número: "
select dump in "${dumps[@]}" "Cancelar"; do
    case "$dump" in
        '') echo "Escolha um dos números da lista." ;;
        Cancelar) exit 0 ;;
        "$other_location") select_from_android; break ;;
        *) generate "$downloads/$dump"; break ;;
    esac
done
