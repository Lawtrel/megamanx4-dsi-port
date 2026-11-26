# Compilação local

## Launcher para jogadores

Baixe o pacote para seu sistema na [página do projeto](https://lawtrel.github.io/megamanx4-dsi-port/)
e siga o `LEIA-ME.md` do ZIP.

- Windows x64 inclui Python. Abra `ABRIR_NO_WINDOWS.cmd` após extrair o ZIP.
- Linux x64 ou WSL usa Python 3.11 ou superior. O seletor gráfico no Linux usa Tkinter.
- Android ARM64 usa Termux com Python e lista os BINs de Downloads. Termux:API é opcional.

Selecione seu BIN original USA SLUS-00561 e uma pasta de destino.
O launcher verifica o SHA-256, converte os dados no aparelho e monta
`mmx4_ds_beta.nds` com o motor pré-compilado.

## Requisitos

- Linux x86-64 ou WSL com uma distribuição Linux x86-64.
- Python 3.11 ou superior, GCC, Git, Make e Tar.
- Acesso à internet no primeiro setup para obter as dependências fixadas.
- Seu próprio BIN/CUE USA de Mega Man X4, SLUS-00561.

O SDK é extraído de uma camada oficial do devkitPro com SHA-256 fixado;
não é necessário instalar Docker. As fontes upstream e o submódulo `psyz`
necessário à preparação de áudio são obtidos nas revisões correspondentes.

## Primeiro uso

Na raiz do projeto, em Linux/WSL:

```sh
python3 tools/setup_native.py --disc '/caminho/Mega Man X4.bin' --cue '/caminho/Mega Man X4.cue'
```

O script verifica o dump antes de gravar `private/SLUS_005.61` e o manifesto
local. O caminho particular fica apenas nesse manifesto, ignorado pelo Git.
Também é possível apontar `X4_DISC` para o BIN e registrar o CUE com
`tools/prepare_disc.py`.

Dump suportado:

- Tamanho: `592217136` bytes.
- SHA-256: `3ceab06ac99add4035f912188bcbf5b16c02056f46a33d38ff0c8dbce6cb613b`.

Não são incluídas nem procuradas cópias do jogo na internet. Outro dump ou
outra edição exige adaptação e verificação; renomear o arquivo não basta.

## Compilar

```sh
python3 native/compile.py
```

A compilação verifica os patches locais, prepara recursos e áudio, compila o
código ARM9, executa os testes e verifica os cabeçalhos, símbolos e dados
internos antes de criar o pacote local.

O `.nds` da beta 9 fica em `dist/native-n09/mmx4_ds_beta.nds`.
Para compilar a beta 9, use `python3 native/compile.py --n09` no Linux
ou WSL. A saída fica em `dist/native-n09/mmx4_ds_beta.nds`.
Para usar uma imagem própria no menu do console, forneça um BMP indexado de 32×32
e 4 bits, com até 15 cores além da transparência:

```sh
python3 native/compile.py --icon-bmp '/caminho/icone.bmp'
```

Para desenvolver o empacotador sem carregar o dump, execute
`python3 native/compile.py --engine-only` após o setup do SDK. O resultado
é o ELF ARM em `build/native/engine/mmx4_ds_beta.elf`. O índice e o
atlas da tela inferior ficam no NitroFS do pacote final.

A montagem no navegador usa um kit de desenvolvimento. Depois do setup
e da compilação com `--engine-only`, execute os comandos abaixo com
Emscripten e Node.js disponíveis:

```sh
python3 native/tests/check_n05_flow.py --sources-only
bash tools/build_ndstool_wasm.sh
python3 tools/build_browser_preparer.py
npm ci --prefix pages --ignore-scripts
node tools/test_browser_preparer.mjs
python3 tools/build_pages.py --with-browser build/browser-preview
python3 -m http.server 8766 --bind 127.0.0.1 --directory build/browser-preview
```

O botão na [prévia local](http://127.0.0.1:8766/) só aparece após validar
o SHA-256. O kit traz as ferramentas, e o navegador lê e processa o BIN
no próprio aparelho. O workflow **Validar compilador WebAssembly**
reproduz a compilação do kit e o teste sintético. A página pública segue
com o verificador enquanto o fluxo completo aguarda validação.

Logs de compilação: `build/native/logs/compile_*.log`.
Os objetos são reutilizados apenas quando as verificações de fontes,
cabeçalhos e opções permitem.

No Windows, depois do setup:

```powershell
.\COMPILAR_JOGO.cmd --no-pause
```

`COMPILAR_JOGO.cmd` é o único atalho e sempre compila a versão atual, sem
fixar um número de versão. Os atalhos `COMPILAR_N*.cmd` foram removidos;
as referências a eles em documentos antigos são históricas. A distribuição
WSL padrão é `Arch`. Para outra:

```powershell
$env:X4_WSL_DISTRO = 'Ubuntu'
.\COMPILAR_JOGO.cmd --no-pause
```

A janela permanece aberta sem `--no-pause`. Um `.nds` antigo em `dist/` não
prova que a nova compilação deu certo: confira a mensagem final e o log.
Nada é instalado no cartão e nenhum loader ou firmware é atualizado.

## Compilar pelo navegador

Em Linux x86-64 ou WSL, na raiz do clone Git:

```sh
python3 tools/serve_builder.py
```

Abra [o compilador local](http://127.0.0.1:8765/), selecione seu BIN
USA SLUS-00561 e clique em **Compilar e baixar**. O servidor usa uma cópia
temporária do commit atual, verifica a SHA-256 do BIN e registra o CUE
correspondente. O dump e o `.nds` são processados na própria máquina.
Mantenha o terminal aberto até o download terminar. Ctrl+C encerra o
servidor e descarta os arquivos temporários.

O workflow **Compilador para Nintendo DS** publica a página de acesso.
Em **Settings → Pages**, selecione **GitHub Actions** como fonte da página.
O workflow publica a interface, o verificador SHA-256 e o ZIP do launcher.
A página verifica o BIN em blocos no navegador, inclusive no celular.

## Verificar dependências sem rede

```sh
python3 tools/setup_native.py --offline
```

## Testes sem dump ou SDK

Estes testes usam apenas dados sintéticos escritos no próprio teste:

```sh
python3 tools/check_source_tree.py
python3 native/tests/check_n06_video.py
python3 native/tests/check_n061_gpu.py
```

Os demais testes e o `.nds` precisam das dependências e dos dados privados.
O template ARM9 está em `native/Makefile.nds`; uma pasta `build/` anterior
não é requisito para iniciar uma compilação.

## Uso e limites

Abra o mesmo arquivo no DS, DS Lite ou DSi. No DSi, selecione o modo DSi/TWL no carregador para habilitar o ARM9 a 134 MHz e o cache ampliado. Consulte [os controles e o status da beta 9](docs/STATUS_N09.md).
Compilar com sucesso e passar testes no computador não confirmam estabilidade
ou 30 FPS no hardware.

O `.nds`, os arquivos de `build/` e todo conteúdo extraído do dump são para uso
local e ficam fora do Git. O pacote gerado inclui gráficos e áudio originais;
não o anexe ao repositório público ou a uma release.

## Cutscenes desativadas

A beta 9 mantém os vídeos desativados e dispensa o FFmpeg. As chamadas
originais seguem imediatamente para o próximo evento. Os arquivos e
ferramentas históricos da N08 não entram na compilação atual.
