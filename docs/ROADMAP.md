# Prioridades atuais

1. **Testar SELECT/Giga da beta 8.4 no DSi.** Conferir X e Zero, energia e
   atalhos de som/retorno. O usuário confirmou as correções gráficas na 8.3;
   [o registro do teste](TESTE_N083_ALEATORIO.md) mantém os contadores
   históricos sem abrir uma pendência visual baseada apenas neles. A N08.1
   não terá teste físico separado por decisão do usuário em 15/11/2026.
   Já há [teste parcial da fase 12](TESTE_N082_FASE12.md). Priorizar os
   checkpoints 5/7/9 com X, onde houve quedas abaixo de 30 FPS. O maior custo medido está na
   composição e preparação gráfica.
   Ver [controles e limites da beta 8.4](STATUS_N084.md). A integração de
   [armas de X5 no debug](X5_ARMAS_DEBUG.md) permanece como extensão futura.
2. **30 FPS sustentados.** Medir as quedas e otimizar os trechos responsáveis,
   preservando escala, animações e velocidade da simulação. Confirmar título,
   diálogos, menus e salvar/carregar na próxima rodada no DSi. Remover filmes
   reduz o pacote, sem comprovar aumento dos FPS de gameplay.
3. **Áudio e fidelidade gráfica restantes.** Investigar seis comandos de
   sequência, panning/reverb, loops e transparências do compositor.
4. **Validação completa da campanha — ao final.** Por escolha explícita do
   usuário, deixar os oito Mavericks, rotas de X/Zero, salvamento ao longo
   da campanha e todas as formas de Sigma para a revisão geral final.

Testes automatizados de regressão continuam durante o trabalho. Eles não
equivalem à qualificação física de uma campanha inteira. Não há promessa de
30 ou 60 FPS em todas as fases na versão atual.
