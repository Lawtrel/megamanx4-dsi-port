# Roadmap do port nativo de Mega Man X4

Em 18/11/2026, o usuário definiu o fechamento de Mega Man X4 como o foco
do projeto. A beta 9 é a referência para avaliar as mudanças no DSi.

1. **30 FPS sustentados — base 8.11 preservada.** Reduzir o custo e a variação do tempo
   de desenho, preparação e envio, preservando escala, animações, controles
   e a simulação original a 60 ticks/s. Medir tempo total por quadro e perdas
   de prazo, incluindo o custo dos diagnósticos. Priorizar a introdução,
   fase 11 e checkpoints 5/7/9 da fase 12 nos percursos já identificados.
   Um orçamento de 33,3 ms não é uma garantia de que todos os quadros o
   cumpram: a confirmação exige medição no DSi, incluindo os piores trechos.
   O [teste da 8.5](TESTE_N085_CAMPANHA.md) acrescenta as fases 1 e 4 como
   alvos de desenho/preparação e mostra o custo pontual dos diagnósticos.
   O usuário confirmou o Continue corrigido no [teste da 8.6](TESTE_N086_FASES_1_4.md);
   a meta de desempenho permanece aberta nas fases 1 e 4.
   Na [8.11](TESTE_N0811_FASE4.md), a fase 4 com X ficou em 30–59 FPS nas
   94 amostras de gameplay, médias de 49,53/54,12 por área, com retorno
   positivo do usuário. Preservar essa base e investigar os picos pontuais
   entre quadros; repetir a medição nas outras fases pesadas. As amostras
   de FPS não encerram a meta de regularidade para todos os quadros.
2. **Completar o áudio.** Refinar os pontos de loop, o estéreo da
   preparação e a reverberação. A reprodução nativa pelo ARM7 permanece como base.
   Dois comandos passam a tocar a sequência zero já preparada do dump,
   seguindo o comportamento de `SsSeqPlay` do jogo original.
   A [8.12 em teste](STATUS_N0812.md) acrescenta posição e ganho dos efeitos
   diretos, respeitando mono/estéreo. Os descritores restantes apontam para
   as sequências 2 e 3 do acesso 1. Os arquivos `PL00SEP.ARC` e
   `PL01SEP.ARC` contêm oito registros no acesso 0 e dois no acesso 1,
   que usa os índices 0 e 1. A leitura dos cabeçalhos dos 138 recursos
   USA não encontrou entradas de substituição SEP de fase. Os quatro
   descritores foram identificados em `PL00_U.ARC` e `PL02_U.ARC`,
   no banco 1, índices 24 e 25. O port os registra como `SEP_INVALID`
   quando acionados, sem interferir em efeitos ativos. A validação do
   comportamento no DSi permanece pendente.
   Em paralelo, um painel de vida, armas, energia e upgrades passa a ocupar
   a tela inferior, com diagnóstico acessível por toque no topo.
3. **Polir a experiência de jogo.** A [beta 8.13 em teste](STATUS_N0813.md)
   acrescenta painel gráfico com poderes/upgrades e troca de armas por toque,
   radar local sem revelar segredos e botão de diagnósticos. Mantém os dados
   consultivos separados das solicitações explícitas de troca de arma.
   O usuário aprovou a tela inferior e não percebeu quedas com o radar ativo
   no DSi. O custo em tempos por quadro ainda não recebeu novo log.
   A [beta 8.14](STATUS_N0814.md) inicia diretamente no título original,
   mantendo o seletor de testes acessível pela aba Debug/atalho. Os
   diagnósticos já ficam numa aba separada. Configurações persistentes
   permanecem pendentes; o início direto ainda precisa de teste físico.
4. **Campanhas de X e Zero validadas pelo usuário no DSi.** Em 17/11/2026,
   confirmou conclusão com ambos os personagens, Sigma derrotado e créditos,
   sem problemas percebidos durante a campanha. Isso substitui a pendência
   genérica de exigir novo playthrough integral. Ver [o registro](TESTE_N0813_CAMPANHAS.md).
   A beta 9 corrigiu a tela preta após os créditos. O usuário confirmou o
   retorno ao título com X no DSi. A meta de 30 FPS sustentados segue aberta.

## Estado confirmado e limites

- O usuário confirmou as correções gráficas na beta 8.3. Os contadores
  históricos do log não mantêm, sozinhos, uma pendência visual aberta.
- SELECT/Giga está implementado na [beta 8.4](STATUS_N084.md); seus novos
  controles ainda aguardam teste no DSi.
- A [beta 8.5](STATUS_N085.md) inicia o foco em desempenho com ordenação
  gráfica mais leve e medição de intervalos completos, p95 e atrasos.
  O ganho de instruções da preparação não confirma FPS sustentado no DSi.
- A N08.1 não terá teste físico separado por decisão do usuário.
- A [beta 8.6](STATUS_N086.md) tem a correção de Continue confirmada
  pelo usuário no DSi. O objetivo de 30 FPS sustentados permanece aberto.
- A [beta 8.7](STATUS_N087.md) reduz consultas e cálculos repetidos no
  desenho de cenários, com equivalência de comandos/pixels em seis casos.
  O [teste físico](TESTE_N087_FASES_1_4.md) ainda mostra quedas abaixo de 30.
- A [beta 8.8](STATUS_N088.md) reduz escritas e controle da montagem da
  lista gráfica. O [teste físico](TESTE_N088_FASES_1_4.md) mostra quedas
  abaixo de 30 FPS e uma nova falha visual confirmada na fase 4.
- A [beta 8.9](STATUS_N089.md) compartilha pixels idênticos entre recursos,
  mantendo a proteção de duas imagens. O novo [teste físico](TESTE_N089_FASE4.md)
  não registrou recusas de desenho, mas
  houve regressão severa de FPS e piscadas pretas no alto da tela.
- A [beta 8.10](STATUS_N0810.md) trata a regressão da cache e limita as
  transferências a janelas controladas. No teste físico, o usuário confirmou o fim das piscadas, mas as médias
  ficaram em 20,31/27,91 FPS na fase 4. Veja [o retorno](TESTE_N0810_FASE4.md).
- A [beta 8.11](STATUS_N0811.md) usa texturas indexadas e paletas protegidas
  para reduzir conversão e envio. A janela de VRAM da 8.10 permanece.
  O [teste físico](TESTE_N0811_FASE4.md) confirmou boa jogabilidade percebida
  e nenhuma amostra de gameplay abaixo de 30 FPS na fase 4 com X. Houve
  picos acima de dois campos de vídeo; outras fases e a regularidade de
  todos os quadros continuam pendentes. A meta é pelo menos 30 FPS
  sustentados e até 60 nos trechos leves.
- Bases de medição: [fase 12 na 8.2](TESTE_N082_FASE12.md) e
  [teste aleatório da 8.3](TESTE_N083_ALEATORIO.md). Percursos diferentes
  não comprovam ganho percentual entre versões.
- Testes automatizados e benchmarks de instruções não medem FPS no console
  nem substituem a validação física das campanhas relatada pelo usuário. Não há confirmação de 30/60 FPS
  sustentados em todas as fases na versão atual.

## Beta 9 em desenvolvimento

- O retorno ao título após os créditos foi confirmado no DSi com X e no
  computador com X e Zero. O usuário também aprovou o visual da beta 9.
  Veja [o registro físico](STATUS_N09.md#teste-no-dsi).
- A tela inferior destaca os poderes disponíveis e separa os detalhes dos
  cartões. O Debug tem Sigma em um golpe e acesso direto aos créditos.
- Veja a saída local e os controles no [status da beta 9](STATUS_N09.md).

## Trabalho após a beta 9

- A preparação da GPU reaproveita a última cor calculada durante fades e
  carrega o dígito alto da ordem no buffer de ordenação. No teste ARM946
  com 600 quads, o fade uniforme caiu de 185.747 para 147.436 instruções
  (20,6%). O caso de camadas com HUD caiu de 143.814 para 142.597.
  Os seis cenários produziram os mesmos bytes de FIFO. O teste mede
  instruções da preparação. A medição de quadros no DSi permanece aberta.
- A preparação de retângulos com textura reaproveita a cor e as
  coordenadas das bordas. No teste ARM946 com Clang, os 14 casos com
  600 ou 1.500 comandos produziram os mesmos bytes de FIFO. Com
  1.500 comandos, o cenário usou 22,32% menos instruções, o fade
  25,26% menos e a mistura de tiles e gradientes 20,07% menos.
  O tempo total dos quadros segue pendente no console.
- O desenho de cenários reaproveita as colunas visíveis em todas as
  linhas de cada camada. No teste ARM946 com texturas indexadas, o caso
  de três camadas caiu de 894.636 para 882.036 instruções (1,4%) e o
  caso da emenda do trem caiu de 851.328 para 832.527 (2,2%). Os nove
  cenários produziram as mesmas saídas byte a byte. O teste no DSi
  ainda precisa medir o tempo total por quadro.
- O preparo local aplica a reverberação Studio A aos instrumentos
  marcados para esse envio. A cauda fica no PCM e o DS usa o mixer
  existente. A biblioteca de preparo verifica a versão da interface
  antes de usar um cache. A prova C no Windows verificou o envio por
  voz, a cauda, o reset e a recuperação de um cache antigo com dados
  sintéticos. A medição da duração dos efeitos e da pressão no cache
  em jogo permanece pendente, especialmente no perfil NTR.
- Se o PCM seco tem período exato, o preparo também pode compactar
  a cauda reverberada que estabilizou. O trecho observado precisa
  conter três ciclos com erro máximo de uma unidade PCM16 em relação
  ao ciclo armazenado. Dois casos sintéticos caíram de 113.400 bytes
  para 46.362 e 54.104 bytes, abaixo do cache NTR de 96 KiB. No melonDS
  em modo NTR, o primeiro passou da mistura por software para o
  cache PCM do ARM7, usando 46.368 bytes após alinhamento. O impacto
  em jogo ainda precisa de medição no console.
- No streaming de efeitos, o mixer reaproveita o período curto que já
  leu no bloco atual. Em um segundo de PCM estéreo sintético com
  período de sete amostras e cache NTR cheio, `fread` caiu de 2.876
  para 381 chamadas e `fseek` de 2.700 para 365. O teste preservou as
  amostras, a fase dos loops e o tratamento de falhas de leitura.
  O custo da leitura no cartão físico segue pendente.
- O cache de áudio repete períodos inteiros para alinhar o PCM às palavras
  do ARM7. Quando a cauda do PCM repete três ciclos idênticos, o banco
  conserva a introdução e grava o ponto de repetição. O mixer em streaming
  usa o mesmo ponto. A análise de 115 descritores de loop já preparados
  encontrou cinco caudas exatas, com redução potencial de 518.970 bytes.
  Os demais preservam a aproximação de três segundos. No melonDS DS em
  modo NTR, uma amostra sintética com ponto e período ímpares manteve 186
  ciclos consecutivos de 445–446 amostras na captura.
  Efeitos em streaming conservam posição e ganho na saída estéreo.
  A música do pacote é centralizada. O teste do mixer cobre cache,
  streaming e PCM curto. Uma prova no melonDS DS em modo NTR, com onda
  sintética posicionada à esquerda, capturou 197.487 quadros estéreo.
  A soma da amplitude absoluta à esquerda foi 3,27 vezes a da direita.
  A conferência no console continua pendente.

- O painel e o radar reaproveitam a imagem quando os dados exibidos
  continuam iguais. A conferência segue a cada seis VBlanks, e o log
  `LOWER_SCREEN` conta as atualizações dispensadas em `skips`. Medir o
  efeito na regularidade dos quadros na introdução, na fase 11 e em Sigma.
- Os comandos single-SEQ de `ST0B_0X.ARC` e `ST0B_0Z.ARC` passam a selecionar
  o PCM da sequência zero. O diagnóstico diferencia referências fora
  da faixa do banco e PCM pendente.
- A [pesquisa de X5](X5_POS_CREDITOS.md) fica arquivada fora do roteiro de X4.
