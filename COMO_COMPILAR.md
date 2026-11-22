# Compilação local

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

O `.nds` da beta 9 fica em `dist/native-n09/mmx4_native_n09.nds`.
Para compilar a beta 9, use `python3 native/compile.py --n09` no Linux
ou WSL. A saída fica em `dist/native-n09/mmx4_native_n09.nds`.
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
O workflow inclui apenas o HTML e o CSS públicos.

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
