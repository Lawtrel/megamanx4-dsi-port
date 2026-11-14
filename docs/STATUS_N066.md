# Mega Man X4 nativo — N06.6

Arquivo local: `dist/native-n066/mmx4_native_n066.nds`. Inicie em modo DSi.
As builds N06.4 e N06.5 foram preservadas. O alvo continua sendo 30 FPS.
Não há medição física da N06.5/N06.6 disponível nesta revisão; o último
registro analisado continua sendo o da N06.4. Esta build não garante 30 FPS.

## Mudanças

- **Transferências de texturas em blocos.** Texturas alteradas vizinhas são
  enviadas em uma operação, em vez de uma operação para cada tile. Os blocos
  terminam ao encontrar uma posição já limpa ou o limite de um banco de VRAM.
  A transferência mantém os mesmos bytes, a sincronização por VBlank e a
  proteção dos quadros anteriores.
- **Paletas inalteradas são reaproveitadas entre quadros.** O port compara
  as cores originais antes de reconstruir a paleta e suas tabelas. Alterações
  de cor e transparência continuam criando as versões corretas. As cópias
  das paletas usam cerca de 64 KiB adicionais de RAM.
- A transparência é consultada uma vez por paleta. O registro de um tile
  evita limpar os campos do comando que já serão todos preenchidos. No ARM9,
  os metadados de cada tile ficam alinhados a uma linha de cache de 32 bytes.
- As correções de texto, a conversão acelerada de pixels e a preparação
  dos comandos antes do VBlank da N06.5 foram mantidas. O ARM7 continua
  reproduzindo os efeitos; esta revisão não muda essa divisão de tarefas.

## Verificações

O teste de envio gráfico transferiu 300 tiles em 2 operações e 1.024 tiles
em 4 operações, mantendo todos os pixels. Também verifica que um bloco não
atravessa uma posição limpa e não cruza os 128 KiB de cada banco. Esses
números vêm de casos sintéticos e não são medições de FPS.

O teste de instruções ARM946 compara a N06.5 com esta revisão. Ele cobre
tiles novos, acertos de cache, paletas estáticas entre quadros e paletas
animadas, nos formatos 4bpp e 8bpp. Os hashes de imagem coincidem nos oito
casos. Instruções não medem a latência de memória/GPU nem substituem o DSi.
Os testes de cores também distinguem preto transparente, preto opaco e
mudanças no bit alto que não alteram a cor visível.

Passaram 48 entradas de área e 22 cenários de sistemas com o novo caminho
gráfico. As dez imagens de regressão de chefes, pausa/opções e salvar ficaram
idênticas às da N06.5. Os relatórios dos componentes de áudio/save inalterados
foram conferidos pelas suas impressões digitais ao validar o pacote.

## Teste físico necessário

1. Compare Magma Dragoon e gelo nos mesmos trechos usados na N06.5, com som
   ligado. Se só tiver resultado da N06.4, registre isso junto à comparação.
2. Confira animação de fogo/neve, cores, transparência e transições de tela.
3. Confira pausa/opções e a tela de salvar/continuar após uma vitória.
4. Recolha `/mmx4/native_n066_000.log`, ou o próximo número disponível.

`PROFILE`, `PIPELINE` e `FIFO` mantêm as métricas da revisão anterior. A nova
linha `UPLOAD tiles=... batches=...` mostra a quantidade de tiles e operações
do último quadro do intervalo. O tamanho transferido continua registrado
em `upload`. Some `prepare_us` aos demais tempos ao avaliar o trabalho total.

Controles: B pula/confirma, A dá dash, Y ataca, X usa especial, L/R trocam arma,
START pausa, L+R+SELECT volta à seleção de testes, SELECT+X alterna o som.
Compile com `COMPILAR_JOGO.cmd --no-pause` ou `COMPILAR_N06.cmd --no-pause`.

## Benchmark opcional

```sh
python3 native/tests/benchmark_n065_video.py --build-only --baseline 1505b218a943f173ed25bc02b487a1c215f99b4c --fixture native/tests/n066_video_benchmark.c --output build/native/n066-video-benchmark
python3 -m pip install --target build/native/test-python unicorn==2.1.4
python3 native/tests/benchmark_n065_video.py --run-only --extended --fixture native/tests/n066_video_benchmark.c --output build/native/n066-video-benchmark --python-path build/native/test-python
```

Somente dados sintéticos são usados. O teste não emula um console e não
entra no executável do DSi. Campanha completa, vídeos e comandos de áudio
pendentes continuam em qualificação.

Referências: [DMA no libnds](https://github.com/devkitPro/libnds/blob/master/include/nds/dma.h)
e [manual técnico ARM946E-S, linhas de cache](https://documentation-service.arm.com/static/5e8e3ee588295d1e18d3aa82).
