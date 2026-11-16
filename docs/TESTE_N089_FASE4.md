# Retorno físico — beta 8.9

Recebido em 16/11/2026: `native_n089_000.log`, 116.683 bytes,
SHA-256 `b7f7c4f2ae96fb75de9d60f14b05f972f0889665839fa16aea4790f274c023d9`.
136 amostras, build `ebeca042224a8e5a3d86`, DSi=1, X equipado no debug,
fase 4 (vulcão). A trilha 0x11 corresponde a `MUSIC_STAGE_VOLCANO` no
upstream. O antigo nome interno do teste `frostrender` não identifica a fase
corretamente; ele executa a fase 4, não a fase de gelo.

Filtro: motor 6/0, gameplay desbloqueado, personagem vivo, fora do menu.

| Área exibida | Amostras | FPS mín./média/máx. | Abaixo de 30 |
| --- | ---: | --- | ---: |
| 1 | 68 | 12 / 19,25 / 28 | 68 |
| 2 | 48 | 6 / 11,23 / 19 | 48 |

O desenho médio foi 33.846 e 72.416 us por área. Desenho, áudio, efeitos
pendentes e overflow amostrado ficaram em zero; não houve `RENDER_FAULT`.
O usuário confirmou uma queda forte de FPS e explicou que o defeito visual
é uma piscada/faixa preta na parte superior da tela, em todas as fases.

O vídeo recebido, de aproximadamente 2,66 s, mostra essa faixa horizontal
intermitente no alto da imagem do jogo. A ausência de recusas no log não
significa ausência de defeito de apresentação no hardware.

## Diagnóstico e limites

A 8.9 comparava e convertia blocos repetidamente porque cada uma das 512
posições de alias guardava apenas uma referência. Aliases de atlas alinhados
se deslocavam entre si. Os contadores `hits` incluem compartilhamentos
que ainda precisaram converter pixels, portanto não significam sempre um
acerto barato. A 8.10 retém as referências em cadeias de hash com 4.096
entradas e usa uma chave calculada sobre todos os pixels.

Outro risco independente estava na transferência: os quatro bancos de
texturas eram retirados do renderizador ao mesmo tempo, sem conferir
quando seriam restaurados. A [documentação GBATEK](https://mgba-emu.github.io/gbatek/#ds-3d-status)
explica que a renderização 3D recomeça na linha 214, ainda dentro do VBlank.
Assim, apenas esperar o início do VBlank não garante uma janela segura para
uma transferência longa. Esse risco é compatível com a faixa preta do
vídeo, mas a causa física exata ainda exige o novo diagnóstico no DSi.

Os percursos da 8.8 e 8.9 não são idênticos; suas médias não constituem um
benchmark controlado. A regressão relatada é aceita, e a meta de 30 FPS
sustentados permanece aberta. A ausência da faixa preta e os FPS da 8.10
ainda precisam ser confirmados no console.
