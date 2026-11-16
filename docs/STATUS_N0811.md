# Mega Man X4 nativo — beta 8.11

Saída local: `dist/native-n0811/mmx4_native_n0811.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N0811.cmd`.
Log: `/mmx4/native_n0811_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.011.520 bytes (aproximadamente 225 MiB),
build `a39bb7bf8145f382d2c6`, SHA-256
`73c5b1026af0c673096144d9ff9f1cd2e64950e582b8f5701dad2370abef6606`.

## Foco: desenho e transferência

O [teste físico da 8.10](TESTE_N0810_FASE4.md) confirmou que as piscadas
pararam, mas as médias ficaram em 20,31 e 27,91 FPS na fase 4 com X.
O desenho consumia cerca de 19–21 ms, além de esperas de transferência.

A 8.11 usa texturas nativas de 8 bits com paletas RGB555 separadas:

- Cada bloco 16×16 ocupa 256 bytes na VRAM, contra 512 do formato direto.
  Os mesmos 512 KiB dos bancos A–D comportam agora 2.048 blocos.
- A conversão gera índices, sem calcular/copiar RGBA nem procurar pixels
  duplicados pelo hash de conteúdo no caminho nativo.
- A versão das cores é independente da versão do remapeamento de índices.
  Uma animação que só troca cores mantém os pixels e envia outra paleta.
- Há 192 paletas físicas de 512 bytes nos bancos E/F/G. Paletas e texturas
  da imagem atual e da anterior ficam protegidas contra reutilização.
- Os limites da 8.10 permanecem: no máximo 8 KiB por transferência, início
  nas linhas 192–204, restauração imediata e espera com os bancos mapeados.
  Como os blocos são menores, um envio de 8 KiB leva até 32 blocos.

A representação é sem perda: não reduz resolução nem quantiza as cores
originais. Transparência e preto opaco continuam distintos. Escala,
animações, geometria, prioridades, controles e a lógica de 60 ticks/s
permanecem. Referência do formato e da paleta:
[GBATEK](https://mgba-emu.github.io/gbatek/#ds-3d-texture-formats).

## Verificação e custos

O teste escalar compara 512 combinações de tamanho, profundidade e paleta,
incluindo preto opaco. Outros casos verificam mudança de cor sem upload de
pixels, remapeamento de transparência, proteção de 2.048 slots e das paletas
anteriores. O recorder da GPU verifica os sete bancos A–G, preservação dos
192 CLUTs, até 1.500 comandos, FIFO, UV, cores, prioridades e a janela de DMA.

A comparação de 61 quadros do corredor da fase 4 contra a referência RGBA
preservou byte a byte 85.335.332 bytes de geometria, cores e índices. Os
testes de entrada em 48 áreas e 51 sistemas usam o novo coletor indexado.
Os previews são reconstruídos com a paleta física retida de cada comando.

Benchmark de instruções ARM946 contra o código da 8.10, com cores iguais:

| Caso sintético | Antes | Indexado |
| --- | ---: | ---: |
| 24 blocos 4bpp novos | 68.337 | 24.825 |
| 24 blocos 8bpp novos | 104.409 | 62.793 |
| 24 mudanças de paleta, 4bpp | 244.918 | 55.148 |
| 24 acertos diretos, 4bpp | 6.655 | 7.495 |

São reduções de cerca de 64% no primeiro caso e 77% no terceiro; acertos
diretos ficaram um pouco mais caros por validar a paleta física. Esses
números não medem ciclos, contenção de memória, DMA ou FPS do console.
A função de reconstrução das cores fica fora da região medida.

As listas FIFO mudam porque o formato e a base da paleta precisam ser
informados à GPU. A validação compara os atributos e os pixels resultantes;
não exige que comandos de formatos diferentes tenham os mesmos bytes.

O caminho RGBA permanece no código para comparação. Os buffers de referência
ainda reservam RAM nesta revisão; o frontend usa o indexado. Uma paleta nova
também custa uma transferência: a redução de 50% por bloco não significa
50% menos dados em qualquer quadro ou 50% mais FPS no jogo inteiro.

## Próximo teste físico

Repetir a fase 4 com X e os mesmos equipamentos/efeitos. Comparar FPS,
`draw_us`, `prepare_us`, `present_us`, upload e `VRAM_WINDOW`. Conferir cores,
transparências, HUD, pausa e animações de paleta. `total_late`, underflow e
overflow da GPU devem continuar em zero.

A meta continua em pelo menos 30 FPS sustentados e até 60 nos trechos que
couberem no orçamento. Esta versão ainda precisa de teste no DSi; os testes
de computador não confirmam o desempenho físico nem a campanha inteira.
Continue permanece corrigido; cutscenes ficam desativadas e efeitos no ARM7.
O NDS e os recursos originais permanecem locais, fora do GitHub público.
