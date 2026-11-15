# Mega Man X4 nativo — N08

Registro histórico: substituído pela [N08.1 sem cutscenes](STATUS_N081.md).
Após teste no DSi, o usuário relatou vídeos lentos e áudio fora de sincronia
e pediu sua retirada em 15/11/2026. Os resultados abaixo são os testes no
computador da versão anterior, não confirmação da reprodução no hardware.

Saída local: `dist/native-n08/mmx4_native_n08.nds`. Requer modo DSi.
A N07 permanece preservada. A qualificação completa da campanha fica para
o final, conforme a orientação do usuário em 14/11/2026.

## Vídeos e áudio

- Integradas as **11 cenas de vídeo**, incluindo o logo inicial, cenas de
  história e encerramentos, aos pedidos originais do motor.
- Cada cena usa imagem completa de 256×192, RGB555 e 15 quadros/s, com áudio
  PCM mono a 18.900 Hz. O formato evita cortar o conteúdo do vídeo. A escala
  e o recorte do gameplay permanecem os mesmos da N07.
- O computador decodifica os STR do dump com FFmpeg. O DSi lê quadros bitmap
  comprimidos em LZ10 e PCM; não executa um decodificador MDEC ou emulador de PS1.
- A reprodução cede a execução ao laço nativo, respeita o fim e o botão de
  pular previstos no jogo e devolve o desenho à GPU. O fluxo de áudio é
  reiniciado nas trocas entre filme e gameplay para descartar o buffer anterior.
- Corrigidos **quatro efeitos** cujos instrumentos pertencem aos bancos do
  personagem mantidos em memória: três no banco da interface e um na rota
  de Zero. A conversão registra a origem dos instrumentos usados.

O áudio continua no transporte nativo que usa o ARM7/hardware. Permanecem
pendentes seis comandos de sequência, panning/reverb e fidelidade dos loops
de alguns efeitos. As transparências do gameplay continuam com aproximações
do compositor DS. Não são anunciados como concluídos nesta revisão.

Os vídeos acrescentam aproximadamente 587 MB aos recursos locais. O `.nds`
tem 823.203.840 bytes (cerca de 785 MiB). Não publique o arquivo, pois contém
recursos do dump. A identificação desta compilação é `1d1ec79f89e9f54b549f`.

## Verificações

Passaram **48 entradas de área e 36 cenários de sistemas**, com controle de
memória no computador. As dez imagens anteriores de chefes, pausa/opções e
salvar ficaram idênticas às da N06.6. O pacote contém 35 imagens diagnósticas.

Foram convertidos e conferidos **13.492 quadros**: cada quadro comprimido é
decodificado pelo mesmo código usado no jogo e comparado com o bitmap de
entrada. O teste de reprodução percorre integralmente os 11 clipes, verifica
a ordem do PCM, término natural, interrupção e reabertura, além de entradas
LZ10 truncadas ou inválidas. Há compilação ARM9 e verificação de memória no
computador. Esses testes não medem velocidade nem sincronismo no DSi.

Os roteiros de regressão do jogo pulam vídeos com START; a reprodução
integral é verificada separadamente. Há também um cenário que deixa o logo
terminar naturalmente e confere o retorno da tarefa original ao título.
A campanha completa e seus finais permanecem para validação posterior.

## Teste desta revisão no DSi

1. Abra a campanha, deixe o logo inicial terminar e confira som e imagem.
2. Use START para pular uma cena e confira o retorno ao título/jogo, cores,
   gráficos e áudio. Repita após salvar e carregar.
3. Confira uma cena de história com fala. Observe atraso do som, pausas ou
   travamentos. O filme tem 15 quadros/s; o contador do laço do jogo não é
   uma medida da taxa de quadros do vídeo.
4. Envie `native_n08_000.log`, ou próximo número livre. A linha `MOVIE`
   registra cena ativa, identificador, quadro e erro.

Compilação: `COMPILAR_JOGO.cmd` ou `COMPILAR_N08.cmd`.
Além dos requisitos anteriores, a preparação dos vídeos requer FFmpeg no
computador (`ffmpeg` no PATH ou `X4_FFMPEG` apontando para o executável).
O conversor é usado somente durante a preparação, nunca incluído no `.nds`.

Referências primárias do conversor: [demuxador STR do FFmpeg](https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/psxstr.c),
[formatos de pixel](https://github.com/FFmpeg/FFmpeg/blob/master/libavutil/pixfmt.h)
e [opções do FFmpeg](https://ffmpeg.org/ffmpeg.html). A tabela de cenas,
limites e botões vem do executável USA verificado do usuário e do código
fixado do upstream; os recursos originais continuam privados.
