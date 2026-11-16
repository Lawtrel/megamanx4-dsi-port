# Beta 8.3 — teste físico aleatório

Recebido em 16/11/2026: `native_n083_000.log`, 339.914 bytes, build
`c0da08be8754468257a2`, modo DSi. SHA-256:
`94542fc7ddcbb536c7e9c6f0ea258cd793507e53d1ef881599ac05820af96697`.
O usuário informou que fez um teste aleatório e que o jogo rodou bem.

O arquivo contém 501 amostras e duas entradas explícitas pelo debug: Zero
na fase 11/área interna 1 e X na fase 5/área interna 1. A sessão de X também
registra a introdução e a fase 8. O log bruto e a análise permanecem privados.

## Amostras de gameplay

Filtro: `engine=6/0`, `locked=0`, HP do jogador maior que zero e
`menu=0/0/0`. As áreas na tabela são as do seletor (índice interno + 1).

| Trecho | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| Zero, fase 11/área 2 | 81 | 25 / 34,54 / 55 | 25 |
| X, fase 5/área 2 | 107 | 28 / 51,64 / 60 | 1 |
| X, introdução/área 1 | 55 | 26 / 33,73 / 59 | 17 |
| X, introdução/área 2 | 44 | 26 / 31,41 / 55 | 17 |
| X, fase 8/área 1 | 12 | 33 / 46,33 / 59 | 0 |

São médias aritméticas de amostras aproximadamente de um segundo, não
tempos de cada quadro. Os trechos diferem dos do log da 8.2; não demonstram
ganho ou perda percentual entre versões. Há quedas abaixo de 30 FPS.

## Contadores

`audioerr=0` e `missingfx=0` em todas as amostras. Não há registro de falha
fatal. Isso não substitui qualificação da campanha completa ou de áudio.

O contador de desenho chega a 9 na sessão de X. Há uma linha detalhada
`RENDER_FAULT count=8 tick=11125 reason=12 id=0 pose=12 texture=1d60
clut=7908 bg=-1`, seguida de uma amostra na pausa (`engine=6/2`,
`menu=0/2/0`), na introdução/área 1, checkpoint 1. O motivo 12 indica que
o envio de um tile retornou erro; ainda não identifica a causa específica.
A nona ocorrência não tem detalhe de objeto registrado nessa linha.

`overflow=0` nas amostras gravadas não prova que não houve overflow num
quadro anterior ao instante amostrado. Esses contadores são por quadro,
enquanto o contador de desenho acumula ocorrências durante a sessão.

O usuário não relatou falha visual nesta mensagem. A ocorrência na pausa
permanece pendente de reprodução e diagnóstico. O log não percorre Sigma,
portanto não confirma fisicamente a correção do diálogo feita na 8.3.

## Solicitação de controle

O usuário pediu Giga/R2 no SELECT. A beta 8.4 implementa essa ação com os
requisitos e a energia originais, mantendo o debug e os atalhos do frontend.
