# Mega Man X4 nativo — beta 8.7

Saída local: `dist/native-n087/mmx4_native_n087.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N087.cmd`.
Log: `/mmx4/native_n087_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.006.400 bytes (aproximadamente 225 MiB), build
`8c80a93087b3ce08d6b0`, SHA-256
`db09d12ff71ff6acf6cb135de93703477f860f795c1a379bfe5a35ac51d951ef`.

## Desenho dos cenários

O teste físico da 8.6 indica custo médio de 11,65 ms no desenho e 2,33 ms
na lógica nas amostras filtradas da fase 4. Esta revisão reduz trabalho
repetido no laço que percorre os tiles dos cenários:

- Os endereços dos bancos, tamanhos e configuração são obtidos uma vez
  por desenho; não são relidos de globais a cada tile.
- Limites visíveis, prioridade da camada e endereço da linha são
  calculados fora do laço interno. Linhas fora do mapa são descartadas antes.
- A marcação de coordenadas de mundo já é feita pelo início do quadro;
  não é repetida em cada tile do cenário.
- As verificações de limites permanecem antes da leitura de dados.

São guardados somente endereços/configuração durante a chamada. Os valores
do mapa, tiles, atributos e paletas são lidos novamente a cada desenho.
Animações, modificações de colisão, gelo, vagões e a repetição do trem
continuam compartilhando os mesmos dados do motor original.

Não há redução de resolução, remoção de efeitos, limite artificial de FPS
ou mudança da simulação de 60 Hz. A reprodução dos efeitos pelo ARM7 e
a correção de Continue confirmada no DSi permanecem.

## Evidência de comparação

`native/tests/benchmark_n087_background.py` compila o desenhador real da
8.6 (`5557bce`) e o atual, junto do mesmo cache de tiles e fixture sintética.
Não usa recursos do jogo. Captura todos os comandos e os pixels RGBA
referenciados em três desenhos consecutivos. A comparação completa é
feita no computador com AddressSanitizer/UndefinedBehaviorSanitizer e
também ao executar o código ARM946 instrumentado.

| Caso | Instruções antes | Agora | Redução |
| --- | ---: | ---: | ---: |
| Três camadas | 920.022 | 820.866 | 10,78% |
| Limites/câmera negativa | 698.487 | 621.039 | 11,09% |
| Emenda repetida do trem | 875.715 | 782.496 | 10,64% |
| Mutações de mapa/tiles/atributos/paleta | 976.653 | 877.497 | 10,15% |

Os seis casos produzem saída idêntica byte a byte. Camadas desativadas
continuam em 261 instruções. O caso de rejeição por dados truncados passa
de 1.278 para 1.380 instruções, preservando o mesmo erro e saída; esse é
o caminho de diagnóstico de dados inválidos, não um cenário válido.

Essas contagens cobrem o trecho medido de cenário/cache, sem tempos de
memória/cache, ciclos, DMA, lógica, sprites ou gravação de log. Não são
medição de FPS nem percentual de ganho do jogo inteiro.

Os 98 cenários de regressão do motor continuam abrangendo 48 entradas de
área e 50 sistemas, incluindo Continue com X e Zero. A medição no DSi
ainda é necessária para esta versão; a validação integral da campanha
permanece reservada para a etapa final.

## Próximo teste físico

Repetir os trechos das fases 1 e 4, com Zero e o mesmo equipamento debug
da 8.6. Manter áudio e diagnóstico ligados para comparação. O log inclui
FPS real, tempos das etapas, máximo/p95 do quadro e atrasos; registrar
também se houve diferença visual. As pausas pontuais de diagnóstico/FAT
continuam medidas e ainda são uma oportunidade de otimização separada.

Controles: B pula, A dash, Y ataque, X especial, L/R arma, START menu,
SELECT Giga/R2 e L+R+SELECT seletor. Cutscenes continuam desativadas.
O `.nds` e os recursos originais permanecem locais, fora do GitHub público.

## Retorno físico de 16/11/2026

O log da 8.7 registra X/Zero na fase 1 e X na fase 4. Os contadores de
desenho/áudio/overflow ficaram em zero; o usuário não anexou relato visual.
Zero na fase 1 teve médias de 37,11/45,08 FPS; X na primeira área da fase
4 teve média 30,08 e mínimo 23. Ainda não há 30 FPS sustentados.
Veja [comparação e limites](TESTE_N087_FASES_1_4.md).
