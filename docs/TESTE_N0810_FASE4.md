# Retorno físico — beta 8.10

Recebido em 16/11/2026: `native_n0810_000.log`, 197.910 bytes,
SHA-256 `d1ed06e4c21f97cf225834a1752ac1932f4e848b73dda76ab2fd6a0320e07702`.
202 amostras, build `039d8cb98b65797ea31a`, DSi=1. X com equipamentos
do debug na fase 4, ambas as áreas.

**O usuário confirmou que as piscadas pretas pararam no DSi.** Essa correção
fica encerrada. A meta de desempenho permanece entre 30 FPS sustentados e
60 FPS nos trechos que couberem no orçamento, sem mudar a lógica de 60 ticks/s.

Filtro: motor 6/0, gameplay desbloqueado, personagem vivo, fora do menu.
Áreas abaixo usam a numeração exibida, começando em 1.

| Área | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| 1 | 52 | 14 / 20,31 / 37 | 50 |
| 2 | 135 | 14 / 27,91 / 35 | 87 |

| Custo médio informado por intervalo | Área 1 | Área 2 |
| --- | ---: | ---: |
| Lógica | 3.714 us | 1.757 us |
| Desenho | 21.299 us | 18.653 us |
| Preparação | 4.167 us | 3.421 us |
| Apresentação, incluindo espera por nova janela | 11.425 us | 4.890 us |

O contador acumulou 1.570 esperas por novas janelas de transferência.
`total_late`, underflow e overflow da GPU ficaram em zero. A maior linha
de início observada foi 204 e a maior linha de restauração foi 207.
Desenho, áudio e efeitos pendentes ficaram em zero; sem `RENDER_FAULT`
ou `VIDEO_FAULT`.

Nos quadros amostrados, foram enviados em média 72.852 e 55.558 bytes por
área. Esses tamanhos e misses descrevem o quadro observado no final do
intervalo, enquanto os tempos de fases são médias do intervalo; não se
deve tratá-los como uma medição sincronizada de todos os mesmos quadros.

O desenho e as esperas de transferência são alvos da 8.11. O formato
indexado reduz cada bloco de 512 para 256 bytes e permite mudar cores pela
paleta sem converter/republicar todos os pixels. Os limites de tempo que
eliminaram as piscadas ficam preservados. Não há confirmação de 30 FPS
sustentados na 8.10 nem medição física da 8.11 nesta etapa.
