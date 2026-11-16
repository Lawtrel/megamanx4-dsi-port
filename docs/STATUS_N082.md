# Mega Man X4 nativo — beta 8.2

Saída local: `dist/native-n082/mmx4_native_n082.nds`. Inicie em modo DSi.
Compilação: `COMPILAR_JOGO.cmd` ou `COMPILAR_N082.cmd`.

Pacote gerado em 15/11/2026: 236.003.328 bytes (aproximadamente 225 MiB),
identificador `f876e6285bcd22521168`.

## O que esta compilação entrega

- Cutscenes desativadas e arquivos de filmes excluídos do `.nds`.
- Seleção de fase/debug com oito armas de X ou todas as técnicas de Zero.
- 48 HP, oito Heart Tanks, dois tanques de vida cheios, tanque de armas e
  EX Tank. X inicia com as quatro peças de armadura e o braço Plasma.
- O equipamento não preenche os indicadores de chefes derrotados; os
  encontros continuam disponíveis. Dano e energia seguem as regras do jogo.
- A campanha aberta pelo título mantém sua progressão normal.
- Preservados áudio pelo ARM7, quatro correções de efeitos da N08, menus,
  salvamento e as otimizações gráficas anteriores.

No seletor inicial, pressione SELECT para alternar da campanha à seleção
de teste. Escolha personagem/fase/área e pressione START para iniciar com
equipamento completo. L+R+SELECT retorna ao seletor durante o jogo.

Controles: B pula, A dash, Y ataque, X especial, L/R troca de arma e START
abre o menu. Para Zero, as técnicas usam os comandos originais do personagem.

## Validação e limites

Passaram no computador 48 entradas de área e 38 cenários de sistemas,
incluindo equipamento debug de X/Zero, seleção de armas, menu, recursos
finitos, reaparecimento e retorno à entrada normal. Os componentes foram
compilados para ARM9. O empacotamento conferiu cabeçalhos, símbolos, fontes
e hashes dos 317 arquivos internos. Dez imagens de regressão anteriores
ficaram idênticas. O pacote anterior da N08.1 foi preservado.

Esta compilação ainda precisa de teste no DSi. A N08.1 não foi testada
separadamente, conforme decisão do usuário; a retirada das cutscenes será
conferida nesta rodada. A campanha completa fica para a revisão final.

As otimizações adicionais de desempenho da proposta da beta 8.2 continuam
pendentes. Esta entrega não comprova melhoria de FPS nem 30/60 FPS sustentados.
Também permanecem seis comandos de sequência de áudio, fidelidade de
loops/panning/reverb, algumas transparências e diagnóstico do melonDS.

Logs: `/mmx4/native_n082_000.log`, ou o próximo número livre. O início da
sessão registra `debug_loadout=1` no modo de teste; FPS e tempos de lógica,
desenho, preparação, envio e áudio continuam separados no log.

O `.nds` contém recursos do dump e permanece local, fora do GitHub público.
