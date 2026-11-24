# Megaman X4 Ds · Launcher beta 9

Use seu próprio dump USA **SLUS-00561** em formato `.bin`. O launcher verifica
o SHA-256 e monta `mmx4_ds_beta.nds` localmente. A pasta temporária usada durante
a montagem é removida ao terminar.

## Windows

Extraia o ZIP inteiro em uma pasta do Windows. Abra **ABRIR_NO_WINDOWS.cmd**,
selecione o BIN e escolha onde salvar o jogo. Acompanhe as etapas na janela de
terminal.

É necessário ter WSL x64 com uma distribuição Linux padrão e Python 3.11 ou
superior nessa distribuição. Se quiser selecionar outra distribuição, defina
`X4_WSL_DISTRO` com o nome exibido por `wsl --list --quiet`.

## Linux

Extraia o ZIP inteiro. Abra o terminal na pasta extraída e execute
`python3 launcher.py` para abrir o seletor de arquivos. Use uma distribuição
x86-64 com glibc 2.35 ou superior, Python 3.11 ou superior e Tkinter.

Em uma máquina sem ambiente gráfico, use
`python3 launcher.py --dump "/caminho/Mega Man X4.bin" --output-dir "/caminho/saida"`.
Para escolher um ícone próprio pelo terminal, acrescente `--icon-bmp` e o caminho
de um BMP de 32×32 pixels, indexado em 4 bits.

## Espaço e saída

Deixe pelo menos 3 GB livres para os arquivos temporários e a saída.
O launcher valida o cabeçalho DS/DSi e cria `mmx4_ds_beta.nds` na pasta
escolhida ao terminar. O ZIP contém o launcher, o motor e as ferramentas
necessárias para processar o dump do usuário.
