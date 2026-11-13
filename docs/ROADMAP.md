# Prioridades atuais

1. **Qualificar a N06.3 no DSi.** Confirmar a correção do reinício de sessões
   e a reprodução dos oito canais de efeitos pelo ARM7/hardware. A falha da
   N06.2 em `memcpy` foi reproduzida e corrigida no teste de memória.
2. **30 FPS sustentados.** Usar perfis do DSi, com trechos comparáveis.
   Desenho e áudio têm picos maiores que a lógica no log atual. Reduzir
   conversões e trabalho repetido, acesso a arquivos e custo da mistura de áudio.
3. **Regressões de qualidade.** Manter escala 1:1, cores após pausa,
   animações, transparência, HUD e recursos adicionais dos chefes.
4. **Campanha e áudio pendentes.** Completar qualificação das fases,
   comandos sonoros restantes, fidelidade de efeitos e vídeos depois da
   estabilidade e do desempenho mínimos.

## Critérios para chamar uma revisão de estável

- Repetir no DSi introdução de X, olho de gelo com Zero, pausa e troca de fase.
- Preservar logs anteriores ao encerramento ou à falha.
- Registrar versão, identificador do executável e trechos usados na medição.
- Tratar os resultados no computador e no console como evidências distintas.

A N06.3 ainda precisa de confirmação física desses critérios. Não há promessa de 30 ou 60 FPS em toda
a campanha na versão atual.
