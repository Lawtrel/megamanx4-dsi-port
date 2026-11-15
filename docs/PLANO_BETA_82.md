# Beta 8.2 — proposta de escopo

## Base e decisão do usuário

Em 15/11/2026, o usuário decidiu não testar a N08.1 separadamente e indicou
a beta 8.2 como próxima etapa. A retirada das cutscenes será verificada
junto com as próximas alterações no DSi. Isso não confirma a N08.1 no console.

Base local: N08.1, commit `7b45222`, sem filmes, aproximadamente 225 MiB,
48 entradas de área e 36 cenários de sistemas aprovados no computador.
O teste físico completo da campanha permanece reservado para o final.

## Prioridade proposta: desempenho e estabilidade

1. **Localizar as quedas.** Usar os tempos já separados de lógica, desenho,
   preparação gráfica, envio e áudio. Acrescentar distribuição dos tempos
   por quadro e contagem de quadros que ultrapassam 33,3 ms. Separar gameplay
   desbloqueado de carregamentos, menus e diálogos na análise.
2. **Otimizar o trecho que os dados apontarem.** Medir conversão de tiles,
   reutilização de texturas/paletas e montagem/envio dos comandos. Comparar
   cada mudança com a base, preservando escala, sprites, animações e HUD.
   O objetivo é reduzir as quedas abaixo de 30 FPS; ainda não há resultado
   medido nem garantia de atingir essa taxa em todas as fases.
3. **Preservar sincronismo do jogo.** Conferir velocidade da simulação,
   resposta dos controles, áudio e descarte de ticks junto com os FPS de
   desenho. Manter o caminho de efeitos no ARM7 já implantado. Nova divisão
   de trabalho entre processadores depende de benefício demonstrado nas
   medições e de testes de sincronização.
4. **Regredir as funções existentes.** Título, novo jogo, salvar/carregar,
   pausa, armadura, vagões, saída da introdução e chamadas de filme puladas.

Os seis comandos de sequência de áudio, loops, panning/reverb e algumas
transparências continuam pendentes. São trabalho secundário ou de revisão
posterior; sua conclusão não é prometida para a beta 8.2.

## Entrega e próximo teste físico

- Gerar um `.nds` identificado como beta 8.2, sem arquivos de filmes.
- Repetir as 48 entradas de área e 36 cenários de sistemas; ampliar apenas
  onde as alterações precisarem de nova cobertura.
- Conferir imagens de regressão, memória e conteúdo do pacote.
- Entregar log que permita separar FPS de desenho, velocidade da simulação
  e custo de cada etapa. Testes no computador não serão apresentados como
  FPS medidos no DSi.
- Na próxima rodada no console, priorizar trechos curtos de Magma Dragoon,
  gelo/miniboss e trem, além de pausa e salvar/carregar. Usar percursos e
  condições repetíveis; logs antigos de trechos distintos não comprovam
  ganho percentual entre versões.

Este arquivo é um plano. A beta 8.2 ainda não foi implementada ou compilada.
