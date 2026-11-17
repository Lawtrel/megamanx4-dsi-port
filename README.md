# Mega Man X4 — port nativo para Nintendo DSi

Port experimental do motor de Mega Man X4 para ARM9, baseado no código C de
[sozud/mmx4](https://github.com/sozud/mmx4). O jogo usa desenho e áudio adaptados
ao DSi. O executável nativo não inclui interpretador de CPU, BIOS ou emulador
completo de PlayStation.

**Beta 8.11: texturas indexadas nativas e paletas protegidas na GPU.**
Mantém o desenho de cenários otimizado, o Continue corrigido e a medição
de quadros completos, com foco em 30 FPS sustentados, SELECT para Giga/R2,
debug e cutscenes desativadas.
No [teste físico da 8.11](docs/TESTE_N0811_FASE4.md), o usuário confirmou
boa jogabilidade. X na fase 4 teve médias de 49,53/54,12 FPS por área;
nenhuma das 94 amostras de gameplay ficou abaixo de 30 (mínimo 30,
máximo 59). Sem erros registrados de desenho/áudio ou atraso de VRAM.
Ainda há picos individuais entre quadros, e as outras fases pesadas
precisam de medição. As piscadas foram encerradas na 8.10.
Os efeitos continuam pelo ARM7/hardware. O log físico da N06.7 registrou
médias de 47,63 FPS com X na primeira área do trem e 39,19 com Zero na segunda,
em trechos distintos. Há quedas abaixo de 30 FPS. Após relato de vídeos lentos
e áudio fora de sincronia na N08, as cenas foram retiradas a pedido do usuário.
O teste físico da 8.5 registra trechos a 60 FPS e quedas até 25 FPS.
A correção das cores após Continue na 8.6 foi confirmada pelo usuário no
DSi. O teste das fases 1 e 4 ainda registra quedas abaixo de 30 FPS. Veja
[os testes físicos](docs/HARDWARE_TESTS.md).

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
python3 native/compile.py --n0811
```

O setup obtém o SDK e as fontes fixadas. O dump suportado é a edição USA
SLUS-00561; o script verifica seu tamanho e SHA-256 antes de extrair dados.
A saída é `dist/native-n0811/mmx4_native_n0811.nds`, para uso local em modo DSi.
Não publique esse arquivo: ele inclui os recursos extraídos do jogo.

No Windows, depois do setup, execute `COMPILAR_JOGO.cmd` ou `COMPILAR_N0811.cmd`.
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

O teste automatizado cobre 48 entradas de área e 51 cenários de sistemas,
incluindo a luta do olho com Zero, pausa, transições, saída da introdução e
seis sessões consecutivas no mesmo processo, sete tipos de vagão e
salvar/carregar pelos menus originais com X e Zero.
A validação completa da campanha fica para o final. Alguns comandos de áudio,
fidelidade de efeitos, qualificação integral da campanha e otimização continuam pendentes.

- [Texturas indexadas na beta 8.11](docs/STATUS_N0811.md)
- [Cache e janela de VRAM na beta 8.10](docs/STATUS_N0810.md)
- [Correção do cache de texturas na beta 8.9](docs/STATUS_N089.md)
- [Otimização da lista gráfica na beta 8.8](docs/STATUS_N088.md)
- [Resultado do último teste físico](docs/HARDWARE_TESTS.md)
- [Prioridades](docs/ROADMAP.md)
- [Arquitetura](native/README.md)

## Licença e créditos

Código sob [AGPL-3.0](LICENSE). A base upstream é fixada em
`4e16dfc7c06597fd5719dd112654d8cf85d343c2`; os patches do port são aplicados
pelo setup. Veja [NOTICE.md](NOTICE.md) para dependências e atribuições.
Mega Man e os recursos originais pertencem aos seus respectivos titulares.
Este projeto não é oficial nem afiliado à Capcom.
