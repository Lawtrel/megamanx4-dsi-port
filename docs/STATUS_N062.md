# Mega Man X4 nativo — N06.2

Arquivo: `mmx4_native_n062.nds`. Abra em modo DSi pelo carregador que já funcionava. O N06.1 continua preservado em `dist/native-n061/`.

**Atualização do teste físico:** o usuário encontrou uma falha ARM9 ao iniciar
outra sessão de fase. O gameplay desbloqueado registrou 7–30 FPS, com média
de 17,70 nas amostras. A versão não está estável e não sustenta 30 FPS.
Veja [a análise do hardware](HARDWARE_TESTS.md). Os resultados no computador
descritos abaixo não substituem essa evidência.

## Correções e desempenho

- A inicialização nativa agora usa a semente aleatória original, `0x0D37`. O port pulava a inicialização dos dispositivos do PS1, mas também deixava de executar essa configuração do jogo. Com semente zero, o gerador sempre devolvia zero. O olho de gelo podia entrar num laço infinito ao tentar selecionar um bloco já removido. Há também um limite de tentativas com escolha de um bloco existente para impedir esse travamento se a semente for danificada.
- A morte do dragão agora aguarda uma vaga no conjunto de efeitos antes de avançar. Antes, a falta de uma vaga podia deixar um ponteiro vazio ou antigo para a explosão. Essa é uma falha concreta de tratamento de recursos; ainda é necessário confirmar no DSi se explica o congelamento relatado.
- Os comandos de desenho agora são preparados na RAM e enviados juntos à GPU por DMA, em vez de milhares de escritas individuais do ARM9. Os últimos três vértices de cada bloco reutilizam a profundidade do primeiro. Cores, transparência, coordenadas e ordem de desenho são preservadas.
- A substituição de texturas usa uma busca circular por uma entrada liberada, preservando as texturas dos quadros atual e anterior. Evita procurar a entrada mais antiga entre todas as 1024 posições a cada textura nova. Paletas consecutivas também reutilizam a consulta.
- A aparência do N06.1 foi mantida: mundo em escala 1:1, recorte central de 256×192, indicadores e menus ajustados à tela. Não há nova redução dos gráficos originais.

O objetivo continua sendo 30 FPS ou mais de forma sustentada. A medição física
recebida mostra que isso ainda não foi alcançado em gameplay.

## Log e falhas

O log usa `/mmx4/native_n062_000.log`, com numeração crescente. Ele é fechado e reaberto a cada relatório, aproximadamente uma vez por segundo, para gravar também o tamanho do arquivo no cartão. Um congelamento ainda pode perder a atualização em andamento, mas não deve descartar toda a sessão já fechada. Não retire o cartão com o jogo ligado.

Uma exceção do ARM9 agora mostra uma tela de falha com PC, LR, SP, posição e último passo de execução. Fotografe essa tela se aparecer. O tratamento da exceção não tenta acessar o cartão. Uma espera infinita ou uma falha do ARM7 pode continuar produzindo apenas congelamento.

Passos de diagnóstico: 1 início/áudio, 2 lógica, 3 áudio após lógica, 4 desenho, 5 espera da tela, 6 envio à GPU.

## Teste físico prioritário

1. Introdução de X: derrotar o dragão e avançar os diálogos com B até sair da fase.
2. Zero na primeira área da fase de gelo: lutar contra o olho, causar dano, esperar os blocos caírem e derrotá-lo.
3. Entrar e sair da pausa e conferir gráficos/animações. Comparar FPS no mesmo trecho das versões anteriores, com som ligado.

Controles mantidos: B pula/confirma, A dá dash, Y ataca, X especial, L/R trocam arma, START pausa. L+R+SELECT volta à seleção de teste. SELECT+X alterna o som.

## Compilação e limites

`COMPILAR_N06.cmd --no-pause` compila, executa as verificações e gera `dist/native-n062/mmx4_native_n062.nds`.

Os testes incluem os 1023 conjuntos não vazios de blocos do olho com semente zero, o gerador após inicialização, falta de vagas para a explosão do dragão e decodificação dos comandos efetivamente produzidos pelo novo envio gráfico. Um teste de 6000 passos coloca Zero no checkpoint anterior ao olho e executa controles de movimento, pulo e ataque: houve dano, perda de blocos, derrota do miniboss e passagem para a segunda área. Nesse teste, a vida de Zero é protegida para manter a cobertura da luta. As verificações no computador continuam separadas da validação física. O teste de saída da introdução prepara o checkpoint e aciona a morte do chefe; não substitui uma luta completa no console.

O jogo permanece em desenvolvimento. Campanha completa, vídeos, alguns comandos de áudio e fidelidade total dos efeitos ainda precisam de trabalho. Não foi feito teste no melonDS nesta revisão.

Código de referência: [sozud/mmx4](https://github.com/sozud/mmx4), base `4e16dfc7c06597fd5719dd112654d8cf85d343c2`, AGPL-3.0. As alterações locais em `main_18_ice_core.c` e `main_08_eregion.c` são identificadas por comentários `Native`. O envio gráfico segue os comandos e a API `glCallList` do SDK libnds local.
