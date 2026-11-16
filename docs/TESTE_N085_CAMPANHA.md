# Beta 8.5 — campanha com X e retorno após Continue

Recebido em 16/11/2026: `native_n085_000.log`, 2.642.896 bytes,
build `726ddbc98cf94e34ca11`, modo DSi. SHA-256:
`a7662dcd59b02972baf5e462becc2d8d79fafb87dc15238b62de1ad6791d726a`.
O arquivo bruto e a análise detalhada ficam na pasta privada.

O usuário relata ter jogado do início até a luta com Double, com 60 FPS
nos trechos leves e aproximadamente 30 com efeitos. Relata também que,
após perder todas as vidas e escolher Continue, as cores da sombra e dos
tiros de X se inverteram e a barra de vida ganhou uma textura amarela.
Isso é uma falha visual nova confirmada pelo relato, apesar dos contadores
`gfxerr`, `audioerr`, `missingfx` e overflow amostrado ficarem em zero.
Não reabre as ocorrências antigas que o usuário confirmou corrigidas na 8.3.

## Desempenho observado

Há 3.245 amostras em uma sessão iniciada pelo título original, com X.
Filtro de gameplay: `engine=6/0`, `locked=0`, HP do jogador maior que zero,
`menu=0/0/0`. Restam 2.142 amostras, das quais 131 ficaram abaixo de 30 FPS;
a média aritmética é 44,40 FPS. Áreas abaixo são índices internos + 1.

| Fase/área | Amostras | FPS mín./média/máx. | Amostras abaixo de 30 |
| --- | ---: | --- | ---: |
| 08/1 | 60 | 30 / 46,75 / 59 | 0 |
| 08/2 | 116 | 28 / 37,63 / 60 | 14 |
| 01/1 | 103 | 25 / 33,38 / 55 | 23 |
| 01/2 | 119 | 27 / 37,97 / 58 | 10 |
| 03/1 | 152 | 29 / 50,12 / 60 | 1 |
| 03/2 | 216 | 28 / 48,35 / 60 | 17 |
| 09/1 | 84 | 52 / 59,07 / 60 | 0 |
| 06/1 | 77 | 29 / 50,04 / 60 | 1 |
| 06/2 | 75 | 30 / 50,39 / 59 | 0 |
| 07/1 | 61 | 37 / 56,75 / 59 | 0 |
| 07/2 | 261 | 29 / 42,69 / 60 | 3 |
| 04/1 | 291 | 25 / 43,81 / 48 | 21 |
| 04/2 | 97 | 25 / 36,87 / 52 | 20 |
| 02/1 | 94 | 29 / 34,79 / 59 | 12 |
| 02/2 | 179 | 29 / 43,94 / 59 | 7 |
| 10/1 | 97 | 29 / 51,94 / 60 | 1 |
| 11/1 | 60 | 29 / 37,15 / 58 | 1 |

As amostras de FPS são intervalos de aproximadamente um segundo, não
mínimos instantâneos de cada quadro. `FRAME_TIME` inclui o diagnóstico e a
gravação anterior: mesmo intervalos com FPS médio acima de 30 podem conter
um quadro atrasado. O estado filtrado é o do fim do intervalo, que ainda
pode conter uma transição ou pausa anterior. Isso não é uma captura
controlada contra a 8.4 nem prova 30 FPS sustentados em toda a campanha.

As fases 1 e 4 concentram oportunidades para a próxima otimização. Nas
amostras filtradas, a fase 1 consumiu em média 10,55 ms no desenho e
4,48 ms na preparação; a fase 4, 9,77 e 3,99 ms. A lógica ficou em
2,22 e 1,52 ms, respectivamente. O diagnóstico anterior chegou a 19,53 ms
no conjunto filtrado. Esses custos não são todos do mesmo pior quadro e
não devem ser somados como se fossem uma medição única.

## Continue: reprodução e correção da beta 8.6

O log mostra Game Over na fase 7/área 2, em torno do tick 108.532, e
retorno ao gameplay no intervalo seguinte. Não há erro de desenho nesse
ponto: as cores erradas são dados válidos para o desenhador.

O teste no computador reproduz a perda da última vida, a janela original
e a confirmação de Continue. A adaptação da janela guarda 512 cores e
substitui a paleta em RAM; o fluxo original de Continue volta ao respawn
sem chamar a restauração que a adaptação precisava. No PlayStation, esse
caminho apenas solicitava uma atualização posterior da paleta na VRAM.

A beta 8.6 restaura a paleta emprestada ao sair de Game Over, antes do
próximo estado carregar dados ou criar o personagem. A entrada no menu
de salvamento (estado 12) continua com a paleta da janela; não é tratada
como retorno à fase. O teste compara todas as 512 cores na saída e verifica
novamente jogador/HUD e ataques após o retorno, com X e Zero. As paletas
animadas de READY são verificadas no momento correto, pois mudam
legitimamente durante o respawn.

O mesmo teste, vinculado ao adaptador da beta 8.5 (`f7a4cb0`), encontra
478 cores diferentes na saída da janela e falha como esperado. Com a
correção, os casos de X e Zero encontram zero diferenças nesse ponto.
Esse controle compara o mesmo momento da transição, sem confundir as
animações posteriores de READY com corrupção.

A correção no computador ainda precisa de confirmação no DSi. A campanha
completa e todas as rotas/finais continuam reservadas para a etapa final.
