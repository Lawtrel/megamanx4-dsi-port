# Beta 8.6 — fases 1/4 e Continue confirmado no DSi

Recebido em 16/11/2026: `native_n086_000.log`, 323.125 bytes,
build `c62e6a2364a0c83f2b08`, modo DSi. SHA-256:
`2fb54af16a1fb2b44bc41f25fe6e07c309423e06a912402be6b314bfbe21cb02`.
Log bruto e análise detalhada permanecem privados.

## Confirmação do usuário

O usuário testou as fases 1 e 4 e confirmou: “a correção do Continue está
finalizada”. A falha relatada na 8.5 fica encerrada por validação física.
Isso não equivale a qualificar todas as rotas/finais da campanha.

O log contém 399 amostras em duas entradas pelo debug, ambas com Zero
e equipamento completo, nas fases 1 e 4. A segunda sessão também retorna
à seleção e entra na introdução, mas não há amostra de gameplay da intro
que satisfaça o filtro abaixo. O Game Over da fase 4/área 2 aparece em
`tick=5426`, seguido do retorno ao gameplay em `tick=5486`.

## Desempenho observado

Filtro: `engine=6/0`, `locked=0`, HP do jogador maior que zero e
`menu=0/0/0`. Áreas na tabela = índice interno + 1.

| Personagem/fase/área | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| Zero, 01/1 | 84 | 28 / 36,38 / 56 | 10 |
| Zero, 01/2 | 81 | 30 / 43,75 / 59 | 0 |
| Zero, 04/1 | 43 | 25 / 32,53 / 49 | 13 |
| Zero, 04/2 | 80 | 26 / 35,96 / 52 | 16 |

Os contadores amostrados `gfxerr`, `audioerr`, `missingfx` e overflow
ficam em zero; não há `RENDER_FAULT`. A confirmação visual vem do relato
do usuário, não somente desses contadores.

As médias variam de 32,53 a 43,75 FPS, com mínimos de 25–30 por área.
Ainda não há 30 FPS sustentados nos quatro trechos. Mesmo a área com
mínimo amostrado de 30 tem intervalos de quadro acima de dois campos.
`FRAME_TIME` inclui atualização do diagnóstico e gravação anterior; o
maior custo anterior de diagnóstico foi 54.001 us na fase 4/área 2.
Os campos medem janelas que podem incluir transições anteriores ao estado
final filtrado; não isolam uma causa única de cada queda.

Este percurso usa Zero/debug e difere da campanha com X na 8.5. As médias
não comprovam melhora/piora percentual entre versões. A 8.6 corrigiu
Continue e manteve as otimizações de preparação da 8.5. O objetivo de
30 FPS sustentados segue como foco, especialmente no desenho das fases
1 e 4 e na redução das pausas de diagnóstico/gravação.
