# Megaman X4 Ds

Este projeto adapta o motor de **Mega Man X4** para rodar de forma nativa no
Nintendo DS, DS Lite e DSi. Ele usa o ARM9 e a GPU do console para executar o jogo. Não é um
emulador de PlayStation.

O código parte da descompilação de [sozud/mmx4](https://github.com/sozud/mmx4).
Para compilar, cada pessoa precisa fornecer seus próprios arquivos do jogo.
Eles ficam no computador e não são enviados ao GitHub.

## Estado atual

A compilação atual é a **beta 9**. Ela inicia no título original e inclui:

- campanhas de X e Zero, validadas pelo usuário no DSi;
- painel inferior com armas, energia, upgrades e radar;
- troca de armas por toque para X;
- aba de diagnóstico e ferramentas de debug;
- opção para deixar Sigma com 1 HP e testar o encerramento direto.

O retorno ao título depois dos créditos foi confirmado no DSi com X.
O foco atual é fechar a qualidade de áudio, o desempenho e os detalhes
da campanha de X4.

O desempenho e alguns detalhes de áudio ainda precisam de mais medições no
console. Os resultados conhecidos estão em [testes no DSi](docs/HARDWARE_TESTS.md).
Veja os controles e o conteúdo no [status da beta 9](docs/STATUS_N09.md).

## Preparar e compilar

O [launcher](COMO_COMPILAR.md#launcher-para-jogadores) aceita o BIN original
e salva o `.nds` no aparelho.

### Requisitos

Use Linux ou WSL com Python 3.11 ou superior, GCC, Git, Make e Tar. O setup
baixa as dependências necessárias e prepara o código local.

### 1. Forneça seu dump de X4

Na raiz do projeto, execute:

```sh
python3 tools/setup_native.py \
  --disc '/caminho/Mega Man X4.bin' \
  --cue '/caminho/Mega Man X4.cue'
```

O port aceita a edição USA SLUS-00561. O setup confere o arquivo antes de
preparar os recursos. Os arquivos extraídos ficam em pastas locais ignoradas
pelo Git.

### 2. Compile o jogo

```sh
python3 native/compile.py
```

O pacote aparece em `dist/native-n09/mmx4_ds_beta.nds`. Use-o localmente,
pois contém recursos do seu dump. Para compilar a beta 9, use
`python3 native/compile.py --n09`.

No Windows, depois de preparar o ambiente pelo Linux ou WSL, execute
`COMPILAR_JOGO.cmd`. Esse é o único atalho `.cmd` da raiz e sempre compila a
versão atual.

## Onde encontrar as informações

- [Roteiro de desenvolvimento](docs/ROADMAP.md)
- [Testes físicos e resultados no DSi](docs/HARDWARE_TESTS.md)
- [Retorno no DSi com a beta 9](docs/STATUS_N09.md)
- [Arquitetura do port](native/README.md)
- [Como compilar](COMO_COMPILAR.md)

## Pastas do projeto

| Pasta | O que contém |
| --- | --- |
| `native/dsi/` | Código específico do Nintendo DSi |
| `native/src/` | Componentes compartilhados do motor e da interface |
| `native/tests/` | Verificações automatizadas executadas no computador |
| `tools/` | Preparação dos dumps, recursos e dependências |
| `patches/` | Ajustes feitos sobre o código de origem |
| `docs/` | Guias, resultados de testes e estado do projeto |

As pastas `ambiente/`, `referencias/`, `private/`, `build/` e `dist/` guardam
dependências, arquivos locais e resultados gerados. Elas são ignoradas pelo
Git.

## Créditos e licença

O código do projeto está sob a licença [AGPL-3.0](LICENSE). A base de X4 é
fixada na revisão `4e16dfc7c06597fd5719dd112654d8cf85d343c2`; veja
[NOTICE.md](NOTICE.md) para créditos e dependências. Mega Man e os recursos
originais pertencem aos seus respectivos titulares. Este projeto não é
oficial nem afiliado à Capcom.
