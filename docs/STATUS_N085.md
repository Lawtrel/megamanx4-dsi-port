# Mega Man X4 nativo — beta 8.5

Saída local: `dist/native-n085/mmx4_native_n085.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N085.cmd`.
Log: `/mmx4/native_n085_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.006.400 bytes (aproximadamente 225 MiB), build
`726ddbc98cf94e34ca11`, SHA-256
`dd93e04417444dc748323262d922b45003ca279959424cae92b972ee21dc4a06`.

## Foco em 30 FPS sustentados

A ordenação gráfica agora calcula os dois histogramas em uma leitura dos
comandos. A primeira passagem trabalha diretamente nos índices originais;
a segunda escreve a lista final, eliminando a lista de identidade e duas
cópias completas por quadro. A ordenação continua estável e decrescente,
inclusive nos extremos de 16 bits e em comandos com a mesma prioridade.

No benchmark ARM946 de 600 quadriláteros, contra a beta 8.4 (`b7b9623`):

| Cenário | Instruções antes | Agora | Redução |
| --- | ---: | ---: | ---: |
| Cenário do jogo | 170.573 | 151.532 | 11,16% |
| Cenário e HUD | 174.701 | 155.660 | 10,90% |
| Fade | 217.690 | 198.649 | 8,75% |
| Cores por vértice | 191.467 | 172.426 | 9,94% |

Os quatro casos produzem listas gráficas idênticas byte a byte. Essa é uma
contagem de instruções da preparação, sem modelar cache, ciclos ou DMA;
**não é ganho medido de FPS nem do tempo total do jogo**.

As linhas fixas de controles na tela inferior são escritas uma vez, e
somente os dados variáveis são atualizados durante o jogo. O log continua
fechando/reabrindo a cada intervalo para preservar os dados após um reset.

## Medição para encontrar as quedas

Além das médias de cada etapa, `FRAME_TIME` registra:

- `mean_us` e `max_us`: intervalo completo entre apresentações consecutivas,
  incluindo a atualização do diagnóstico e a gravação do log anterior.
- `p95_upper_us`: limite superior do percentil 95, em faixas de 1 ms;
  intervalos de 100 ms ou mais usam o máximo como limite superior.
- `over_two_fields`: quantos intervalos atravessaram mais de dois campos
  de vídeo. O relógio de referência é o VBlank real do console.
- `work_max_us`: maior duração do trabalho do quadro, retirando apenas a
  espera explícita pelo VBlank; não inclui o relatório posterior.
- `diagnostic_previous_us`: custo do relatório anterior, incluindo tela e FAT.

A primeira apresentação não entra no histograma. Cada relatório cobre
aproximadamente um segundo; o FPS exibido continua sendo o número real de
apresentações por intervalo. Não foi introduzido limite artificial de FPS,
descarte de camadas, redução da resolução nem alteração da simulação a 60 Hz.

## Verificação e próximo teste no DSi

Os testes do compositor verificam prioridades negativas/positivas, extremos,
empates, lista vazia, um comando e 1.500 comandos fora de ordem. Continuam
as verificações de profundidade, cores, UV/flips, proteção da VRAM e DMA
com AddressSanitizer e UndefinedBehaviorSanitizer.

Os 96 cenários de motor da beta 8.4 permanecem vinculados às mesmas fontes
e binários verificados. A lógica do motor, seus recursos e os controles
não mudaram nesta revisão. O empacotamento recusa relatórios desatualizados.

A confirmação de 30 FPS sustentados depende do próximo teste físico.
Priorizar introdução, fase 11 e os checkpoints 5/7/9 da fase 12, mantendo
diagnóstico/log ligados para capturar os piores intervalos. As correções
gráficas da 8.3 já foram confirmadas pelo usuário; os contadores históricos
não são tratados como defeito visual aberto.

As betas anteriores ficam preservadas. O `.nds` e os recursos originais
permanecem locais, fora do repositório público. As demais prioridades
estão no [roadmap](ROADMAP.md).
