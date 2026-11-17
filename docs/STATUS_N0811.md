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

## Retorno físico e próximos alvos

No [teste recebido em 16/11/2026](TESTE_N0811_FASE4.md), o usuário confirmou
que a 8.11 está rodando muito bem. X equipado na fase 4 registrou médias
de 49,53/54,12 FPS por área. Nenhuma das 94 amostras de gameplay ficou
abaixo de 30; mínimo 30, máximo 59. Desenho caiu para 7.366/6.278 us e
apresentação para 1.252/1.134 us nos intervalos amostrados. Não é uma
comparação de percursos controlados contra a 8.10.

Sem erros registrados de desenho/áudio, atraso de restauração de VRAM,
underflow ou overflow da GPU. Ainda houve 26 intervalos acima de dois
campos entre 4.892 quadros medidos, com pico de 100.558 us. A aprovação
do desempenho percebido não equivale a cumprir 33,3 ms em todos os quadros.

A 8.11 fica como base com retorno físico positivo. Os próximos alvos são
esses picos e a medição das outras fases pesadas. A meta continua em pelo
menos 30 FPS sustentados e até 60 nos trechos que couberem no orçamento.
A campanha inteira permanece sem qualificação integral. Continue está
corrigido; cutscenes ficam desativadas e efeitos no ARM7. O binário testado
não muda com este registro; NDS e recursos ficam locais, fora do GitHub público.
