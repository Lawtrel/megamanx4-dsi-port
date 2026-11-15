# Mega Man X4 nativo — N08.1, sem cutscenes

Saída local: `dist/native-n081/mmx4_native_n081.nds`. Requer modo DSi.

O usuário decidiu não testar esta revisão separadamente em 15/11/2026.
Sua verificação no console fica junto com a futura
[beta 8.2](PLANO_BETA_82.md); ela não foi confirmada no hardware.

Em 15/11/2026, o usuário relatou vídeos lentos e áudio fora de sincronia
na N08 e pediu para retirar as cutscenes. Esta revisão pula imediatamente
as onze chamadas originais de vídeo, incluindo logo, história e finais.
Diálogos desenhados pelo jogo, menus e progressão continuam no motor original.

Os arquivos de vídeo e seu PCM não entram no NitroFS. O código de reprodução
e a troca temporária para tela bitmap também saem do executável. O build
normal não converte filmes nem requer FFmpeg. As ferramentas históricas
da N08 e os pacotes anteriores ficam preservados separadamente.

O `.nds` tem aproximadamente **225 MiB**, contra 785 MiB da N08: redução
de cerca de 71%. Retirar filmes não comprova melhora de FPS nas fases.

Permanecem as quatro correções de efeitos da N08, áudio pelo ARM7/hardware,
menus, salvamento, peças de armadura, vagões e otimizações gráficas da N07.
A escala e o recorte do gameplay permanecem iguais aos da N07.

## Verificação e limites

Os testes cobrem 48 entradas de área e 36 cenários de sistemas, incluindo
onze chamadas de filme sem alterar controles, música ou tarefas, para X e
Zero. As regressões incluem título, novo jogo, salvar/carregar, saída da
introdução, pausa, armadura e vagões. O empacotamento rejeita arquivos de
filme e símbolos do reprodutor no executável.

Essas verificações no computador não comprovam FPS ou estabilidade no DSi.
A validação física desta revisão ainda é necessária; a campanha completa
continua reservada para o final. Permanecem seis comandos de sequência de
áudio, fidelidade de loops/panning/reverb e algumas transparências pendentes.
Não há promessa de 30/60 FPS sustentados em todas as fases.

## Uso

Compile com `COMPILAR_JOGO.cmd` ou `COMPILAR_N081.cmd`.
Use a nova saída em modo DSi, mantendo seus saves em `/mmx4/`.
O log usa `native_n081_000.log`, ou o próximo número livre.

Controles: B pula, A dash, Y ataque, X especial, L/R troca de arma,
START abre o menu e L+R+SELECT abre a seleção de teste.

Confira título → novo jogo, retorno de uma cena de história ao gameplay e
salvar/carregar. O `.nds` contém recursos do seu dump e permanece local;
não deve ser publicado no repositório.
