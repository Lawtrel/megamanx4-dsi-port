# Megaman X4 Ds · Launcher beta 9

Use seu próprio dump USA **SLUS-00561** em formato `.bin`. O launcher verifica
o SHA-256 e monta `mmx4_ds_beta.nds` localmente. A pasta temporária usada durante
a montagem é removida ao terminar.

## Windows x64

Extraia o ZIP inteiro em uma pasta do Windows. Abra **ABRIR_NO_WINDOWS.cmd**,
selecione o BIN e escolha onde salvar o jogo. Acompanhe as etapas na janela de
terminal.

O pacote Windows x64 inclui Python e os conversores nativos. Se você baixou
o pacote Linux/WSL para usar no Windows, ele requer Python 3.11 ou superior
na distribuição WSL x64. Use `X4_WSL_DISTRO` para selecionar uma distribuição
com o nome exibido por `wsl --list --quiet`.

## Linux

Extraia o ZIP inteiro. Abra o terminal na pasta extraída e execute
`python3 launcher.py` para abrir o seletor de arquivos. Use uma distribuição
x86-64 com glibc 2.35 ou superior, Python 3.11 ou superior e Tkinter.

Em uma máquina sem ambiente gráfico, use
`python3 launcher.py --dump "/caminho/Mega Man X4.bin" --output-dir "/caminho/saida"`.
Para escolher um ícone próprio pelo terminal, acrescente `--icon-bmp` e o caminho
de um BMP de 32×32 pixels, indexado em 4 bits.

## Android ARM64 com Termux

1. Instale [Termux](https://github.com/termux/termux-app). Rode `pkg install
   python` e depois `termux-setup-storage`. Autorize o acesso a Downloads.
2. Coloque o ZIP e o seu BIN original em Downloads. Cole este comando no Termux.

   ```sh
   python3 -m zipfile -e "$HOME/storage/downloads/megaman-x4-ds-launcher-android-arm64.zip" "$HOME" && \
     bash "$HOME/megaman-x4-ds-launcher-android-arm64/abrir-no-android.sh"
   ```

3. Escolha o BIN pelo número na lista. O `mmx4_ds_beta.nds` fica em Downloads.

Para montar outra vez, execute apenas o comando `bash` acima. Você pode passar
o caminho do BIN como argumento. A opção **Escolher em outra pasta** fica
disponível com o [Termux:API](https://github.com/termux/termux-api) e o pacote
`termux-api` instalados.

## Espaço e saída

Deixe pelo menos 3 GB livres para os arquivos temporários e a saída.
O launcher valida o cabeçalho DS/DSi e cria `mmx4_ds_beta.nds` na pasta
escolhida ao terminar. O ZIP contém o launcher, o motor e as ferramentas
necessárias para processar o dump do usuário.
