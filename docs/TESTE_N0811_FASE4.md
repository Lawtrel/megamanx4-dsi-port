# Retorno físico — beta 8.11

Recebido em 16/11/2026: `native_n0811_000.log`, 122.506 bytes,
SHA-256 `0c64a8aafc60dd68261ff576f9675353ba5daadeed72c78a4c160e7bb674ad1b`.
127 amostras, build `a39bb7bf8145f382d2c6`, DSi=1. Uma sessão de X na
fase 4 (Magma Dragoon/Volcano), ambas as áreas, com equipamentos do debug.
O usuário informou: **“agora sim, está rodando muito bem”**.

## Desempenho observado

Filtro: motor 6/0, gameplay desbloqueado, personagem vivo, fora do menu.
Áreas abaixo usam a numeração exibida, começando em 1.

| Área | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| 1 | 45 | 30 / 49,53 / 59 | 0 |
| 2 | 49 | 30 / 54,12 / 59 | 0 |

São médias aritméticas dos FPS informados por intervalo. As 94 amostras de
gameplay ficaram na faixa pretendida de 30–60 FPS. A sessão inclui a arena
do chefe; termina com Magma Dragoon ainda vivo, sem validar sua derrota
ou a transição seguinte nesta versão.

| Custo médio informado por intervalo | Área 1 | Área 2 |
| --- | ---: | ---: |
| Lógica | 1.910 us | 1.527 us |
| Desenho | 7.366 us | 6.278 us |
| Preparação | 4.031 us | 3.857 us |
| Apresentação, incluindo espera por nova janela | 1.252 us | 1.134 us |
| Áudio no ARM9 | 582 us | 521 us |
| Espera pela apresentação | 5.573 us | 5.392 us |

A lógica apresentou média de 60 ticks/s nos intervalos filtrados. O
contador acumulado de ticks descartados não ficou em zero: variou de 9 a
11 na primeira área e de 24 a 32 na segunda. Não se deve declarar que
todos os prazos da simulação foram cumpridos.

## Regularidade e transferências

| Intervalos entre apresentações nos períodos filtrados | Área 1 | Área 2 |
| --- | ---: | ---: |
| Quadros medidos | 2.235 | 2.657 |
| Acima de dois campos de vídeo | 12 | 14 |
| Maior intervalo | 100.558 us | 67.109 us |
| Maior custo de trabalho do quadro | 93.097 us | 51.369 us |
| Maior diagnóstico anterior informado | 13.002 us | 12.916 us |

O FPS por intervalo não mostra todos os picos individuais: houve 26
intervalos acima de dois campos entre 4.892 quadros medidos, cerca de
0,53%. Os blocos de tempos abrangem o intervalo anterior ao estado
amostrado; perto de transições, esse estado não descreve necessariamente
todos os quadros do bloco. O log não isola a causa de cada pico. Ainda
não há garantia de 33,3 ms em todos os quadros ou na campanha inteira.

O contador acumulou sete esperas por novas janelas de transferência.
`total_late`, underflow e overflow da GPU ficaram em zero. As maiores
linhas de início e restauração observadas foram ambas 197. Não houve
`RENDER_FAULT`, `VIDEO_FAULT`, erro de desenho, erro de áudio, efeito
pendente ou overflow de comandos registrado.

Nos quadros amostrados, os envios tiveram médias de 2.930 e 1.813 bytes
por área, já incluindo as paletas físicas. Upload, misses e comandos
descrevem o quadro observado no final do intervalo; os tempos de fases
são médias do intervalo. Não são medições sincronizadas de todos os
mesmos quadros. `shared=0` e `alias_hits=0` são esperados no novo caminho
indexado, que não usa a deduplicação de pixels RGBA.

## Comparação e próximos passos

Na [8.10](TESTE_N0810_FASE4.md), X equipado na mesma fase teve médias de
20,31 e 27,91 FPS por área; desenho em 21.299/18.653 us e apresentação em
11.425/4.890 us. O novo teste tem médias de 49,53/54,12 FPS e menor custo
nessas etapas. Os percursos, ações e quantidades de amostras diferem:
esses valores não constituem um benchmark controlado ou ganho percentual
garantido para o jogo inteiro.

A 8.11 passa a ser a base com retorno físico positivo. Preservar formato
indexado, paletas retidas e a janela segura de transferência. Os próximos
alvos são os picos pontuais e os trechos pesados já identificados nas
outras fases, sem reabrir Continue ou as piscadas encerradas na 8.10.
Completar a qualificação da campanha continua reservado para o final.

Este retorno não exige outro binário: o `.nds` testado permanece o mesmo.
O manifesto de compilação preserva a informação disponível na geração;
este documento registra a validação física posterior. Log bruto, dump,
recursos e NDS ficam locais, fora do GitHub público.
