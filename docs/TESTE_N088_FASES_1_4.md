# Retorno físico — beta 8.8

Recebido em 16/11/2026: `native_n088_000.log`, 207.778 bytes,
SHA-256 `e38a16329147a32cb68d505df64c91280380aefbaabbf5eb2e79d3b11f99d8ec`.
256 amostras, build `ba7236c57a671aba086a`, DSi=1. Duas sessões de X com
os equipamentos do debug, fases 4 e 1, ambas as áreas.

Filtro: motor 6/0, gameplay desbloqueado, personagem vivo e fora do menu.
Áreas abaixo usam a numeração exibida no console, iniciando em 1.

| Fase/área | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| 4/1 | 48 | 23 / 32,35 / 46 | 15 |
| 4/2 | 42 | 27 / 43,48 / 54 | 2 |
| 1/1 | 59 | 28 / 37,05 / 59 | 8 |
| 1/2 | 64 | 29 / 45,06 / 59 | 1 |

As médias com X são maiores que as da 8.7, e a preparação média ficou
entre 3,42 e 4,11 ms nessas áreas. Percursos livres distintos não permitem
atribuir um ganho percentual à alteração. Ainda há quedas abaixo de 30;
não é confirmação de desempenho sustentado.

## Falha visual confirmada

O usuário confirmou sprites/efeitos sumindo ou piscando com X na fase 4.
O log acumulou três recusas, com último registro no tick 2.225: motivo 12,
objeto 3, pose 19, textura 0, CLUT 0x7802. Áudio e efeitos pendentes ficaram
em zero. O overflow amostrado também foi zero: a amostragem de um quadro
por intervalo pode perder a saturação que ocorreu entre as amostras.

Na reprodução automatizada do mesmo corredor, trocando as nove armas,
a 8.8 recusou seis objetos no tick 2.354 por falta de slot: 499 slots da
imagem atual e 525 da anterior estavam protegidos, totalizando 1.024.
É uma reprodução da mesma classe de recusa; as entradas exatas do teste
físico e a identidade de cada efeito percebido não foram reconstruídas.

A beta 8.9 compartilha somente blocos com pixels RGBA e índices idênticos,
antes de ocupar mais um slot. A proteção das duas imagens permanece.
A correção de Continue confirmada na 8.6 continua encerrada. A confirmação
visual da nova correção e a medição de FPS da 8.9 dependem do DSi.
