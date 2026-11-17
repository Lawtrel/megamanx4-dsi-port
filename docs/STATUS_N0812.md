# Mega Man X4 nativo — beta 8.12 em teste

Saída local: `dist/native-n0812/mmx4_native_n0812.nds`, em modo DSi.
Compilar: `python3 native/compile.py --n0812` ou `COMPILAR_N0812.cmd`.
Log: `/mmx4/native_n0812_000.log`, ou próximo número livre.
A beta 8.11 e seu pacote permanecem disponíveis como base testada.

## Áudio: primeira etapa

Os efeitos diretos passam a usar a posição do objeto relativa à câmera,
seguindo o cálculo do adaptador original. O ARM7 recebe panning e ganho
por reprodução; a cache continua compartilhando o mesmo PCM entre posições.
O ganho compensa a distribuição entre os canais. Sons com posição fixa,
sequências e chamadas sem objeto permanecem centralizados. A opção original
mono/estéreo é respeitada nas novas reproduções, e silenciar/reativar conserva
o ganho de cada voz. Não há síntese de PS1 durante o jogo.

Isso ainda não completa o passo 2 do roadmap. A música e os instrumentos
preparados continuam mono; a separação desta versão é espacial nos efeitos
diretos. Loops aproximados de três segundos e reverberação permanecem
pendentes, assim como seis descritores sem conversão:

- Recursos `4b` e `4e`, banco 1, índices 24/25: pedem sequências 2/3 no
  acesso 1. O SEP de personagem disponível contém apenas duas, índices 0/1.
- Recursos `6a`/`6b`, banco 2, índices 24/20: comandos single-SEQ; falta
  determinar a origem correspondente antes de preparar PCM correto.

Esses casos continuam retornando pendência, sem substituir por outro som.
A posição é definida no início do efeito; vozes contínuas não seguem o
objeto depois de iniciadas. O mixer mono usado nos testes de referência
permanece mono; a distribuição é validada no backend ARM7 gravado.

## Tela inferior

Painel do jogador com vida, vidas, armas/técnicas, energia, corações,
armadura, Giga e tanques a partir do estado real de X/Zero. A atualização
é limitada a 10 Hz e só reescreve linhas alteradas no console existente.
Toque nos 24 pixels superiores da tela inferior alterna painel/diagnóstico.
O painel apenas consulta os dados; não equipa armas nem altera progresso.
Os logs permanecem ativos. O menu original continua na tela superior.

## Teste no DSi

Esta versão exige validação física de áudio, toque e custo do painel.
Compare com a 8.11 na fase 4: disparos e inimigos à esquerda/direita,
opções mono/estéreo, SELECT+X para silenciar/reativar, troca de X/Zero,
menu de pausa e alternância painel/diagnóstico. Confira energia e upgrades
com o menu original e envie o log `native_n0812_*.log`, identificando sons
que ainda faltam ou soam diferentes e em que situação ocorrem.

Não há nova medição de FPS no hardware nem confirmação de fidelidade sonora
integral. Os recursos e o `.nds` ficam locais, fora do repositório público.

## Verificação local

Compilação ARM9 e empacotamento concluídos em 17/11/2026: 236.014.592 bytes,
SHA-256 `d3272964945d03d8aeeca412767c85eeeb51ba55ec959e49c30d6999e32e3957`.

Passaram os 48 percursos de entrada de fase e 51 cenários de sistemas,
incluindo leitura do painel com X/Zero e preservação do estado do jogo.
O teste de áudio percorreu 5.983 descritores nos caminhos de referência
e cache ARM7 (1.818 aceitos, 6 pendentes e 4.159 vazios). O recorder ARM7
confere posição, ganho, mono, som fixo, cache, alinhamento e mute.
Testes do formatter verificam limites das linhas, energia e posse dos tanques.
Também passaram os testes de recursos, música, saves, vídeo e backend GPU.
Os testes de memória usaram ASan/UBSan conforme cada suíte; a detecção de
vazamentos foi desligada nesta execução por restrição do ambiente.
Isso não mede FPS nem confirma fidelidade sonora no DSi.
