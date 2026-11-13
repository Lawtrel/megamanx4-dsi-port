# Mega Man X4 — port nativo para Nintendo DSi

Port experimental do motor de Mega Man X4 para ARM9, baseado no código C de
[sozud/mmx4](https://github.com/sozud/mmx4). O jogo usa desenho e áudio adaptados
ao DSi. O executável nativo não inclui interpretador de CPU, BIOS ou emulador
completo de PlayStation.

**Em desenvolvimento: N06.2. Existe uma falha ARM9 conhecida ao iniciar uma
área após uma sessão anterior. O desempenho ainda não sustenta 30 FPS.**
O último teste físico registra gameplay entre 7 e 30 FPS, com média de 17,70
nas 209 amostras desbloqueadas. Veja [testes no hardware](docs/HARDWARE_TESTS.md).
Os testes no computador não substituem essa validação.

## O que está aqui

- Código do port, carregamento de recursos e controles de X/Zero.
- Desenho pela GPU do DS, cache de texturas, HUD e menu de pausa.
- Música, vozes e efeitos PCM preparados localmente a partir do dump.
- Testes de recursos, áudio, tarefas, fases, sistemas e comandos gráficos.
- Scripts para obter dependências em revisões fixas e gerar o `.nds` localmente.

**Este repositório não contém o jogo, dump, BIOS, texturas, músicas ou `.nds`
com os recursos originais.** É necessário fornecer seu próprio dump compatível.
Os dados extraídos e os binários gerados permanecem nas pastas ignoradas pelo Git.

## Preparar e compilar

Use Linux ou WSL com Python 3.11+, GCC, Git, Make e Tar.
No Linux/WSL, a partir da raiz:

```sh
python3 tools/setup_native.py --disc '/caminho/Mega Man X4.bin' --cue '/caminho/Mega Man X4.cue'
python3 native/compile.py --n06
```

O setup obtém o SDK e as fontes fixadas. O dump suportado é a edição USA
SLUS-00561; o script verifica seu tamanho e SHA-256 antes de extrair dados.
A saída é `dist/native-n062/mmx4_native_n062.nds`, para uso local em modo DSi.
Não publique esse arquivo: ele inclui os recursos extraídos do jogo.

No Windows, depois do setup, execute `COMPILAR_JOGO.cmd` ou `COMPILAR_N06.cmd`.
As duas opções compilam o mesmo alvo atual. Detalhes em
[COMO_COMPILAR.md](COMO_COMPILAR.md).

## Organização

| Pasta | Conteúdo |
| --- | --- |
| `native/dsi/` | Adaptação ao console, tarefas, carregamento e desenho |
| `native/src/` | Componentes portáveis do motor nativo |
| `native/tests/` | Testes e verificações no computador |
| `native/Makefile.nds` | Modelo versionado da compilação ARM9 |
| `tools/` | Setup, verificação do dump e auditoria do código publicável |
| `patches/` | Alterações locais sobre a revisão fixa do upstream |
| `docs/` | Estado atual, testes físicos e prioridades |

`ambiente/`, `referencias/`, `private/`, `build/` e `dist/` são locais e ignoradas.
Os nomes internos `n05_*` identificam os adaptadores sobre os quais a N06 evoluiu;
não representam um segundo produto ou um emulador.

## Estado e próximos passos

O teste automatizado cobre 48 entradas de área e 16 cenários de sistemas,
incluindo a luta do olho com Zero, pausa, transições e saída da introdução.
A campanha completa permanece em validação. Vídeos, alguns comandos de áudio,
fidelidade de efeitos, crash ARM9 e otimização continuam pendentes.

- [Estado e controles da N06.2](docs/STATUS_N062.md)
- [Resultado do último teste físico](docs/HARDWARE_TESTS.md)
- [Prioridades](docs/ROADMAP.md)
- [Arquitetura](native/README.md)

## Licença e créditos

Código sob [AGPL-3.0](LICENSE). A base upstream é fixada em
`4e16dfc7c06597fd5719dd112654d8cf85d343c2`; os patches do port são aplicados
pelo setup. Veja [NOTICE.md](NOTICE.md) para dependências e atribuições.
Mega Man e os recursos originais pertencem aos seus respectivos titulares.
Este projeto não é oficial nem afiliado à Capcom.
