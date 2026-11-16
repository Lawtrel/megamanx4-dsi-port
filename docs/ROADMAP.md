# Roadmap do port nativo de Mega Man X4

Prioridades registradas e aprovadas pelo usuário em 16/11/2026.

1. **30 FPS sustentados — foco atual.** Reduzir o custo e a variação do tempo
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
2. **Completar o áudio.** Resolver seis comandos de sequência e refinar
   loops, estéreo/panning e reverberação. A reprodução nativa pelo ARM7
   permanece como base.
3. **Polir a experiência de jogo — implementação futura.** Iniciar direto
   no título original, tornar o debug opcional, permitir ocultar os
   diagnósticos da tela inferior e oferecer configurações persistentes.
4. **Validar a campanha completa — ao final.** Conferir as rotas de X/Zero,
   oito Mavericks, upgrades, salvar/carregar, Sigma e os encerramentos sem
   cutscenes. Menus, saves e progressão já têm implementação; esta etapa
   é a qualificação integral, não uma declaração de que toda a campanha
   ainda precisa ser reimplementada.
5. **Conteúdo extra depois da base estável.** Investigar uma arma de X5 por
   vez no banco experimental do debug, com normal/carregado, colisão,
   energia e recursos preparados do dump do usuário. Não integra a meta
   atual de desempenho. Ver [referências e limites](X5_ARMAS_DEBUG.md).

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
  A meta é sustentar pelo menos 30 FPS e alcançar 60 nos trechos leves;
  a confirmação de desempenho da 8.11 depende do próximo teste no DSi.
- Bases de medição: [fase 12 na 8.2](TESTE_N082_FASE12.md) e
  [teste aleatório da 8.3](TESTE_N083_ALEATORIO.md). Percursos diferentes
  não comprovam ganho percentual entre versões.
- Testes automatizados e benchmarks de instruções não medem FPS no console
  nem qualificam a campanha inteira. Não há confirmação de 30/60 FPS
  sustentados em todas as fases na versão atual.
