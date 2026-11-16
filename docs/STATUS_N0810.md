# Mega Man X4 nativo — beta 8.10

Saída local: `dist/native-n0810/mmx4_native_n0810.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N0810.cmd`.
Log: `/mmx4/native_n0810_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.008.448 bytes (aproximadamente 225 MiB),
build `039d8cb98b65797ea31a`, SHA-256
`77312e92244154c8a1bdae798859972d8cbca2457c2a7f9d3da7305896655c49`.

## Regressão de desempenho da 8.9

O [teste físico](TESTE_N089_FASE4.md) registrou médias de 19,25 e 11,23 FPS
nas duas áreas da fase 4 com X. O usuário relatou forte queda e mostrou uma
faixa preta intermitente na parte superior da imagem, em todas as fases.

A 8.9 conservava apenas um alias por posição de uma tabela de 512 entradas.
Atlas alinhados causavam colisões frequentes: uma referência útil expulsava
a anterior, e o jogo voltava a converter e comparar pixels. A 8.10 usa
4.096 aliases em cadeias de hash, com validação da sequência de reutilização
do slot e proteção das referências recentes. Acertos nessas cadeias não
precisam converter a textura novamente.

O índice de conteúdo agora considera todos os pixels RGBA, evitando cadeias
longas de blocos com bordas parecidas. A igualdade completa de RGBA e dos
índices continua obrigatória. Isso tem custo em blocos novos; não é uma
promessa de que todo tipo de consulta ficará mais rápido.

No benchmark ARM946 contra a 8.9, com saída de pixels preservada:

| Caso sintético | Instruções 8.9 | Instruções 8.10 |
| --- | ---: | ---: |
| 1.024 consultas de atlas repetido, 4bpp | 3.632.236 | 312.104 |
| 1.024 consultas de atlas repetido, 8bpp | 5.163.436 | 317.384 |
| 24 acertos diretos, 4bpp | 6.847 | 6.655 |
| 24 blocos novos distintos, 4bpp | 48.873 | 68.337 |

A redução do primeiro caso é de 91,4% das instruções daquele caminho, não
91,4% do tempo do jogo. Blocos novos custam mais por usar uma chave completa;
esta desvantagem também fica registrada. Cache, ciclos, barramento, DMA e
FPS físicos não são medidos por esse benchmark.

## Transferências e piscadas pretas

A GPU do DS volta a renderizar na linha 214, dentro do período de VBlank.
O código anterior retirava os quatro bancos do renderizador para limpar
cache da CPU e copiar dados, sem conferir se a restauração acontecia a
tempo. É um risco real de apresentação e uma hipótese compatível com a
faixa do vídeo; a causa física exata ainda depende do novo diagnóstico.
Referência: [GBATEK — estado da GPU 3D](https://mgba-emu.github.io/gbatek/#ds-3d-status).

Na 8.10:

- A preparação/limpeza da cache da CPU acontece antes de esperar o VBlank,
  enquanto os bancos continuam disponíveis para a GPU.
- Cada transferência cobre no máximo 8 KiB e mexe somente no banco necessário.
- O início é aceito nas linhas 192 a 204. O trecho curto de mapeamento/cópia/
  restauração impede interrupções, e o banco volta a ser textura imediatamente.
- Se a janela acabou, todos os bancos ficam mapeados como texturas enquanto
  o programa espera o próximo VBlank. A imagem anterior é mantida até terminar
  o envio completo; não se descarta metade do desenho.
- O log registra as linhas de início/restauração, atrasos, esperas e observações
  acumuladas de underflow e overflow da GPU, em `VRAM_WINDOW`.

O limite de 8 KiB é conservador, mas a duração real depende do hardware.
`total_late` deve permanecer zero; qualquer ocorrência exige rever o orçamento.
Uma carga muito grande pode ocupar outra janela e reduzir FPS. Esse custo
fica exposto no log e não é apresentado como desempenho sustentado.

## Verificação e próximo teste

Os testes verificam aliases alinhados que colidem, igualdade de pixels,
referências antigas, limites de slots, UV, cores, ordem de desenho e proteção
das duas imagens. O recorder da GPU verifica entradas dentro/fora da janela,
transferências em partes, espera com os bancos mapeados, preparação repetida
e reconhecimento dos sinais de erro da GPU. É um modelo de chamadas e tempo;
não reproduz ciclos, contenção de barramento ou a rasterização física do DS.

Os testes do motor continuam cobrindo 48 entradas de áreas e 51 sistemas,
incluindo a carga da fase 4, Continue, menus, upgrades e chefes. Comparações
de instruções ARM946 e hashes de saída não medem FPS no DSi.

Repetir a fase 4 com X, com movimentos e ataques. Conferir a faixa preta no
alto da imagem, FPS e os registros `VRAM_WINDOW`, `TEXTURE_SHARING` e
`VIDEO_FAULT`. A meta de 30 FPS sustentados e a ausência de piscadas ainda
precisam ser confirmadas no console.

Resolução, sprites, animações, controles e lógica original a 60 ticks/s são
preservados. Continue continua corrigido; os efeitos seguem pelo ARM7;
cutscenes permanecem desativadas. O NDS e os recursos originais ficam locais.
