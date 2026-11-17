# X5 jogável após os créditos — objetivo da beta 9

Pedido do usuário em 17/11/2026: polimento gráfico, atalho de debug para
Sigma/créditos e início de Mega Man X5 jogável após o encerramento de X4.
Não é uma tela de título simulada nem a execução de um emulador.

## Estado real

- Dump USA de X5 recebido em `Recursos/MMX5` e verificado em 17/11/2026.
  O BIN CUE tem 582,954,960 bytes, faixa única MODE2/2352 e SHA-1
  `10709231f857636b5ccd3cd9acebc91458dcb5fd`, correspondente ao disco USA.
  `SLUS_013.34` confere com SHA-1 `476e55ff4ac698a1ee0654c0f08804a5837bd2f3`.
- `tools/prepare_x5_disc.py` valida o disco e extrai `SYSTEM.CNF`,
  `SLUS_013.34`, `ROCK_X5.BIN` e `ROCK_X5.DAT` para `private/x5-disc/`,
  ignorada pelo Git. O dump original não é modificado.
- `tools/index_x5_archives.py` valida e cria índice local dos 172 membros de
  `ROCK_X5.DAT` e das 59 entradas de `ROCK_X5.BIN` (55 overlays de código e
  quatro overlays de dados), incluindo tamanhos e SHA-1 individuais. Os
  payloads continuam privados e não foram convertidos para o renderer ARM9.
- `private/mmx5-audit` é uma amostra de pesquisa: quatro unidades C,
  dois headers e documentos, revisão `120cb521f8b08b3e056d9738945cf950d004d07a`.
- Não há motor X5 ligado ao ARM9 nem recursos X5 no pacote.
- A referência reconstrói o executável de PS1 e 55 overlays. Código C que
  ainda acessa scratchpad `0x1F800xxx` e RAM `0x801Fxxxx` não funciona no
  DSi apenas por ser compilado para ARM.

Fonte primária: [anzaldoivan/mmx5](https://github.com/anzaldoivan/mmx5).
O README informa a edição USA SLUS-01334, SHA-1 da faixa de dados
`10709231f857636b5ccd3cd9acebc91458dcb5fd` e do executável
`476e55ff4ac698a1ee0654c0f08804a5837bd2f3`. O hash da faixa não deve ser
confundido com o hash do BIN inteiro sem analisar a estrutura do CUE.

## Caminho de implementação

1. **Concluído:** verificar o dump, extrair os arquivos de boot e dados e
   indexar os 172 membros e 59 entradas de código/dados em pastas locais
   ignoradas pelo Git.
   Próximo: identificar as tabelas, imagens e áudio entre esses membros e
   preparar conversores para o renderer ARM9.
2. Fixar a árvore completa da referência e mapear funções C e assembly
   necessárias ao título, seleção de personagem e primeira fase.
3. Adaptar memória global/scratchpad, tarefas e overlays, controles,
   colisão, desenho e áudio aos serviços nativos do DSi. Não adicionar
   interpretador MIPS como substituição de funções ausentes.
4. Primeiro marco verificável: título e uma área jogável de X5 compilados
   para ARM9, com movimento, tiro, colisão e recursos originais corretos.
   Isso ainda não equivale à campanha X5 completa.
5. Conectar o término real de X4 ao backend X5 funcional, liberando sessões,
   objetos, caches e áudio. Manter saves/progressão de X4 e X5 separados.

O ponto original de saída é `func_800239E0`, após o fade dos créditos,
antes de agendar novamente o boot. Os testes da 8.15 exercitam o encerramento
por atalho de debug e registram a permanência no estado final preto; o
problema não é tratado como X5 já disponível.

A beta 8.15 é preparação. A beta 9 solicitada não está concluída enquanto
a integração jogável de X5 e o polimento gráfico pedido não forem entregues.
