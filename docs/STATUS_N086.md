# Mega Man X4 nativo — beta 8.6

Saída local: `dist/native-n086/mmx4_native_n086.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N086.cmd`.
Log: `/mmx4/native_n086_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.006.400 bytes (aproximadamente 225 MiB), build
`c62e6a2364a0c83f2b08`, SHA-256
`bc43c8ab4827b0025a9c4b6b328bd70cc0aa831774c2c0e4d506a66cd590482b`.

## Cores após Continue

Corrige a paleta da janela de Game Over que permanecia aplicada ao jogo
depois de Continue. Isso alterava cores de sprites, efeitos e HUD sem
necessariamente produzir um erro no contador gráfico.

Ao sair do estado de Game Over, a adaptação restaura as 512 cores
emprestadas antes do próximo carregamento/respawn. Uma entrada no menu
de salvamento continua com a janela ativa. Não há mudança no número de
vidas, progresso, equipamento, dano ou consumo de energia.

O teste reproduz a perda da última vida na fase 7/área 2, confirma Continue
pelo menu original e compara a paleta completa na saída. Depois verifica
cores do jogador/HUD e execução de ataques, com X e Zero. As mudanças
legítimas da paleta de READY são separadas dessa verificação.
O mesmo caso de X falha na 8.5 com 478 cores diferentes na saída; os
casos corrigidos de X e Zero apresentam zero diferenças.

## Desempenho e testes

Mantém a ordenação gráfica e as medições da [beta 8.5](STATUS_N085.md).
Esta revisão corrige o retorno do menu; não anuncia um ganho novo de FPS.
Os testes abrangem 48 entradas de área e 50 cenários de sistemas,
incluindo dois novos casos de Continue. A correção também foi confirmada
pelo usuário no DSi, conforme o retorno físico abaixo.

O [log físico da 8.5](TESTE_N085_CAMPANHA.md) tem 2.142 amostras filtradas
de gameplay, 131 abaixo de 30 FPS e mínimo 25. A meta continua sendo
30 FPS sustentados; fases 1 e 4 e o custo dos diagnósticos são os próximos
alvos de medição. O relato do usuário de 60 FPS nos trechos leves e cerca
de 30 com efeitos é preservado separadamente dos mínimos do log.

Controles: B pula, A dash, Y ataque, X especial, L/R troca de arma,
START abre o menu e SELECT aciona Giga/R2. L+R+SELECT volta ao seletor.
O debug inicia equipado; a campanha normal mantém a progressão original.
Cutscenes permanecem desativadas. As betas anteriores ficam preservadas;
o `.nds` e os recursos originais permanecem locais, fora do GitHub público.

## Retorno físico recebido em 16/11/2026

O usuário confirmou a correção de Continue no DSi após testar as fases
1 e 4. Essa falha fica encerrada. O log registra Zero com equipamento
debug completo, contadores de desenho/áudio/overflow em zero e médias de
32,53–43,75 FPS por área, com quedas até 25. Ainda não confirma 30 FPS
sustentados. Veja [dados e limites](TESTE_N086_FASES_1_4.md).
