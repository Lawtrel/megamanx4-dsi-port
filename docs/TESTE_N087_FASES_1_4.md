# Beta 8.7 — fases 1 e 4 com X/Zero

Recebido em 16/11/2026: `native_n087_000.log`, 355.366 bytes,
build `8c80a93087b3ce08d6b0`, modo DSi. SHA-256:
`cb3f197b53eda6af5920839af77adc798214993ef45f292eeaf6d984fb0ebfde`.
Log bruto e análise detalhada permanecem privados.

Há 438 amostras em três sessões de debug equipado: X na fase 1, Zero
na fase 1 e X na fase 4. O usuário enviou o log sem relato visual adicional.
Os contadores amostrados de desenho, áudio, efeitos pendentes e overflow
ficam em zero; não há registro de `RENDER_FAULT`. Isso não substitui uma
confirmação visual do usuário nem qualifica a campanha inteira.

## Gameplay filtrado

Filtro: `engine=6/0`, `locked=0`, HP do jogador maior que zero e
`menu=0/0/0`. Áreas na tabela são índices internos + 1.

| Personagem/fase/área | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| X, 01/1 | 78 | 27 / 33,81 / 59 | 17 |
| X, 01/2 | 75 | 29 / 40,89 / 58 | 1 |
| Zero, 01/1 | 66 | 28 / 37,11 / 52 | 9 |
| Zero, 01/2 | 78 | 30 / 45,08 / 59 | 0 |
| X, 04/1 | 50 | 23 / 30,08 / 45 | 27 |
| X, 04/2 | 39 | 28 / 41,18 / 51 | 2 |

## Comparação e limites

Com Zero na fase 1, as médias da 8.6 foram 36,38/43,75 FPS por área;
na 8.7, 37,11/45,08. O desenho médio caiu de 10,50 para 10,22 ms na
primeira área e de 7,93 para 7,53 ms na segunda. São percursos livres,
com quantidades/custos de comandos diferentes: a tendência não prova
um ganho percentual controlado entre versões.

A fase 4 da 8.7 foi testada com X, enquanto a 8.6 usou Zero. O mínimo
23 FPS na primeira área com X não permite concluir regressão contra os
25 FPS de Zero na 8.6. A meta de 30 FPS sustentados segue aberta.

## Onde o tempo está sendo gasto

Na primeira área da fase 4 com X, médias filtradas:

- Lógica: 3,24 ms.
- Desenho: 13,57 ms.
- Preparação de comandos gráficos: 4,52 ms.
- Envio/apresentação: 2,14 ms.
- Áudio: 0,86 ms.
- Espera explícita: 9,17 ms.

Na janela de 23 FPS, o desenho médio foi 16,51 ms e a preparação 5,55 ms;
o maior trabalho de quadro registrado foi 68,46 ms. As fases são médias
por intervalo e o máximo é uma observação diferente; não devem ser
somados como se fossem o mesmo quadro. A amostra de cache no fim dessa
janela tinha 27 tiles novos; em outra janela de 25 FPS chegou a 405.
Esses contadores são do último desenho amostrado, não a média do intervalo.

A preparação permanece próxima de 4–5 ms por quadro em vários trechos.
O próximo alvo é reduzir as escritas e o controle da montagem da lista
nativa de comandos, preservando seu conteúdo exato. Não há proposta de
alterar a velocidade da lógica, a escala ou remover efeitos.

O Continue permanece confirmado como corrigido no teste físico da 8.6;
este log não reabre essa falha. A validação integral das rotas/finais
continua reservada para a etapa final.
