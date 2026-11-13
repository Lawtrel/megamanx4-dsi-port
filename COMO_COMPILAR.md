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
python3 native/compile.py --n06
```

A compilação verifica os patches locais, prepara recursos e áudio, compila o
código ARM9, executa os testes e verifica os cabeçalhos, símbolos e dados
internos antes de criar o pacote local.

Saída: `dist/native-n063/mmx4_native_n063.nds`.
Logs de compilação: `build/native/logs/compile_*.log`.
Os objetos são reutilizados apenas quando as verificações de fontes,
cabeçalhos e opções permitem.

No Windows, depois do setup:

```powershell
.\COMPILAR_JOGO.cmd --no-pause
```

`COMPILAR_N06.cmd` é um atalho equivalente. A distribuição WSL padrão desses
atalhos é `Arch`, usada no ambiente de desenvolvimento. Para outra:

```powershell
$env:X4_WSL_DISTRO = 'Ubuntu'
.\COMPILAR_JOGO.cmd --no-pause
```

A janela permanece aberta sem `--no-pause`. Um `.nds` antigo em `dist/` não
prova que a nova compilação deu certo: confira a mensagem final e o log.
Nada é instalado no cartão e nenhum loader ou firmware é atualizado.

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

Inicie o resultado em modo DSi. Consulte [os controles e limites](docs/STATUS_N063.md).
A N06.3 tem correções verificadas no computador e ainda precisa de medição e
qualificação física no DSi.
Compilar com sucesso e passar testes no computador não confirmam estabilidade
ou 30 FPS no hardware.

O `.nds`, os arquivos de `build/` e todo conteúdo extraído do dump são para uso
local e ficam fora do Git. O pacote gerado inclui gráficos e áudio originais;
não o anexe ao repositório público ou a uma release.
