# Prioridades atuais

1. **Qualificar a N06.5 no DSi.** Comparar Magma Dragoon e gelo com a N06.4,
   conferir efeitos e paletas e validar os textos de salvar/continuar.
2. **30 FPS sustentados.** Usar perfis do DSi, com trechos comparáveis.
   Na N06.4 o desenho de Magma Dragoon foi o maior custo. A N06.5 acelera
   a conversão dos tiles e prepara comandos antes da espera de vídeo. Somar
   `prepare_us` aos demais tempos e medir os piores intervalos antes de
   transferir novas tarefas entre CPUs.
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

A N06.5 ainda precisa de confirmação física desses critérios. Não há promessa de 30 ou 60 FPS em toda
a campanha na versão atual.
