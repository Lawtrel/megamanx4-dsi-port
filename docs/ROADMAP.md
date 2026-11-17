# Roadmap do port nativo de Mega Man X4

Prioridades registradas e aprovadas pelo usuário em 16/11/2026.

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
2. **Completar o áudio — foco atual.** Resolver seis comandos de sequência e refinar
   loops, estéreo/panning e reverberação. A reprodução nativa pelo ARM7
   permanece como base.
   A [8.12 em teste](STATUS_N0812.md) acrescenta posição e ganho dos efeitos
   diretos, respeitando mono/estéreo. Os seis descritores, loops precisos,
   estéreo dos recursos e reverberação permanecem abertos. Ainda requer DSi.
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
   Há relato de tela preta depois dos créditos; seu fluxo permanece aberto.
   Não há novo log para encerrar a medição de FPS/regularidade.
5. **Conteúdo extra depois da base estável.** Investigar uma arma de X5 por
   vez no banco experimental do debug, com normal/carregado, colisão,
   energia e recursos preparados do dump do usuário. Não integra a meta
   atual de desempenho. O usuário também propôs integrar a campanha de X5
   como continuação após os créditos de X4. É uma direção futura de pesquisa,
   sem implementação ou prazo assumidos. Ver [referências e limites](X5_ARMAS_DEBUG.md).

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

## Preparação da beta 9 — pedido de 17/11/2026

- Polir gráficos; a 8.15 começa pelo painel inferior com nomes e energia
  legíveis, contraste e cabeçalho do radar. Ajustes do renderer superior
  precisam de casos visuais concretos; não são declarados concluídos.
- Debug com Sigma 1 golpe e acesso direto aos créditos para testar o final.
- Iniciar X5 jogável depois dos créditos passa a integrar o objetivo pedido
  para a beta 9. Não é só uma imagem/tela de transição. O dump USA foi
  recebido e verificado; boot, overlays e arquivo de dados estão extraídos
  em `private/x5-disc/`. Ainda falta indexar/adaptar os recursos e integrar
  o motor X5 ao ARM9. A 8.15 é preparação, sem declarar a beta 9 pronta.
- O usuário pediu polimento nas duas telas. A 8.15 ajustou apenas a inferior;
  não há alteração no renderer superior nem alvo visual concreto registrado
  ainda. Tratar o polimento de ambas como pendência da beta 9.
- Manter título original direto e a validação de X/Zero já fornecida.
