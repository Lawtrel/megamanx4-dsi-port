# Mega Man X4 nativo — beta 8.8

Saída local: `dist/native-n088/mmx4_native_n088.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N088.cmd`.
Log: `/mmx4/native_n088_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.005.376 bytes (aproximadamente 225 MiB), build
`ba7236c57a671aba086a`, SHA-256
`fededb1488be7bfe82a47469cc553b21770b2b2330e9d0075c2b7b58bbe7b554`.

## Montagem da lista gráfica

O [teste físico da 8.7](TESTE_N087_FASES_1_4.md) mostra cerca de 4–5 ms
por quadro na preparação de comandos gráficos. Esta revisão mantém o
desenho de cenários da 8.7 e otimiza a montagem da lista nativa da GPU:

- O cursor, grupo de opcodes e posição do próximo comando são locais
  durante a preparação, evitando atualizar índices globais repetidamente.
- Os quatro bytes de opcodes são montados em uma palavra temporária e
  gravados juntos, em vez de reler/modificar a mesma palavra em RAM para
  cada comando. Os parâmetros continuam na mesma ordem e posição.
- O último grupo é gravado mesmo quando contém menos de quatro opcodes.
  Uma lista vazia continua vazia e nunca referencia um grupo inexistente.

Essa montagem ocorre enquanto o buffer pertence à CPU, antes do VBlank.
Apresentação, DMA, proteção de texturas e lista recebida pela GPU ficam
preservados. Não há alteração de escala, efeitos, controles ou ritmo da lógica.

## Verificação

Comparação ARM946 contra a 8.7 (`5e6b288`), com 600 quadriláteros reais
do compositor nativo e leitura de todos os bytes da lista FIFO:

| Caso | Instruções antes | Agora | Redução |
| --- | ---: | ---: | ---: |
| Cenário | 151.532 | 137.847 | 9,03% |
| Cenário e HUD | 155.660 | 141.975 | 8,79% |
| Fade | 198.649 | 180.004 | 9,39% |
| Cores por vértice | 172.426 | 151.775 | 11,98% |

Os quatro casos produzem listas idênticas byte a byte, com o mesmo tamanho.
São contagens de instruções de preparação, sem simular cache, ciclos ou DMA;
não medem FPS nem ganho percentual do jogo inteiro.

Os testes com AddressSanitizer/UndefinedBehaviorSanitizer verificam a GPU
real compilada com captura de comandos: profundidade, prioridades assinadas,
empates, UV/flips, cores, listas vazias e até 1.500 comandos, além da
proteção de slots e dos limites de DMA.

Os 98 casos de motor (48 entradas de área e 50 sistemas) já validados
na 8.7 permanecem com os mesmos binários e fontes. Não foram reexecutados
por uma alteração exclusiva no backend da GPU e na identificação do frontend;
o pacote confere novamente suas fontes/hashes e recusa relatórios obsoletos.
Os testes específicos do backend modificado foram recompilados e executados.

A correção de Continue confirmada pelo usuário no DSi permanece. A 8.8
teve [teste físico recebido](TESTE_N088_FASES_1_4.md), com melhora de médias
e nova falha visual confirmada na fase 4. A beta 8.9 trata a saturação
reproduzida; a validação integral da campanha continua reservada para o final.

## Próximo teste

Repetir um trecho pesado da fase 4 com **X**, como no log da 8.7. Para a
fase 1, usar o mesmo personagem da comparação desejada. Manter arma,
equipamento, áudio e diagnóstico equivalentes. Comparar `prepare_us`,
FPS e tempos/atrasos de quadro e registrar qualquer mudança visual.

As pausas pontuais de diagnóstico/FAT continuam medidas e não foram
alteradas nesta revisão. A meta de 30 FPS sustentados segue aberta.
Cutscenes permanecem desativadas; os efeitos continuam pelo ARM7.
O `.nds` e os recursos originais permanecem locais, fora do GitHub público.
