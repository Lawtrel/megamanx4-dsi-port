# Mega Man X4 nativo — N06.3

Arquivo local: `dist/native-n063/mmx4_native_n063.nds`. Inicie em modo DSi,
pelo mesmo carregador usado nas versões anteriores. A N06.2 foi preservada.

## Mudanças

- **Efeitos pelo ARM7 e pelo hardware de som.** O ARM9 envia comandos ao
  driver Calico que já existe no ARM7. Oito canais, de 8 a 15, ficam reservados
  para os efeitos; a música continua no stream mono do Maxmod, no canal 0.
- O ARM9 deixa de abrir/ler até oito arquivos e somar seus samples a cada
  atualização de áudio. Cada efeito é carregado na primeira utilização e
  reaproveitado em um cache de PCM com limite de 4 MiB. A música continua
  sendo lida pelo ARM9, como um único stream.
- As amostras em RAM compartilhada são alinhadas, publicadas com limpeza
  de cache e mantidas imutáveis durante a reprodução. A reserva de canais
  e o cancelamento usam confirmação do ARM7. Nenhum buffer é liberado ou
  reutilizado antes da confirmação de parada. Arquivos, objetos do jogo e
  estados de lógica não são compartilhados entre os processadores.
- **Correção do crash ao iniciar outra sessão.** Ao liberar os gráficos, a
  N06.2 mantinha tamanho e tipo antigos do banco. A próxima sessão tentava
  copiar esse banco a partir de um ponteiro nulo. O teste reproduziu a falha
  com AddressSanitizer; a correção limpa os metadados junto com os ponteiros.
- Consulta de paletas por hash reduz buscas repetidas durante o desenho.
  Coordenadas, escala 1:1, cores, HUD e animações permanecem com a mesma regra.

O ARM7 não substitui o ARM9 para lógica e desenho. Esta revisão remove um
gargalo específico de áudio e otimiza uma parte do desenho; **30 FPS
sustentados ainda dependem de medição no DSi**. A primeira reprodução de um
efeito pode ter custo de leitura. O cache é limitado e preserva amostras em uso.

## Testes e limites

O teste de reinício executa seis sessões no mesmo processo, alternando X/Zero
e as duas áreas da fase interna 1. Ele falhava na segunda sessão antes da
correção. Os testes do novo controle de áudio verificam canais reservados,
alinhamento, tamanho em palavras, oito vozes, mute, cache sem acesso ao arquivo
e confirmação de parada antes da liberação da memória. São testes no
computador, incluindo compilação do controlador com os cabeçalhos reais do SDK.
Não substituem a audição e a validação física da comunicação entre CPUs.

O jogo continua em desenvolvimento. Campanha completa, vídeos, alguns
comandos de áudio e fidelidade total dos efeitos ainda estão em qualificação.
O PCM é o mesmo já preparado na versão anterior; loops e mistura final do
hardware precisam ser conferidos no console.

## Teste no DSi

1. Jogue com Zero, volte à seleção com L+R+SELECT e inicie X na outra área.
   Repita a troca de personagem e área algumas vezes.
2. Compare FPS no mesmo trecho e com som ligado. Atire, dê dash e use o sabre
   repetidamente. Observe se os sons cortam, deixam de terminar ou ficam presos.
3. Repita o olho de gelo com Zero, o dragão da introdução e a pausa.
4. SELECT+X alterna o som. Compare o mesmo trecho para identificar o custo
   de áudio que ainda restar.

Controles mantidos: B pula/confirma, A dá dash, Y ataca, X usa especial,
L/R trocam arma e START pausa. B avança diálogos após a introdução.

## Diagnóstico e compilação

Logs: `/mmx4/native_n063_000.log`, com numeração crescente. As linhas `ARM7`
registram vozes em uso, bytes de PCM em cache, acertos, faltas e reproduções.
As linhas `PROFILE` continuam separando lógica, desenho, áudio e envio gráfico.
Uma exceção também mostra R0–R11 para facilitar o diagnóstico por fotografia.

`COMPILAR_JOGO.cmd --no-pause` e `COMPILAR_N06.cmd --no-pause` geram a N06.3.
O `.nds` contém recursos privados do dump e não deve ser publicado no GitHub.

Referências primárias: [interface ARM9 do som Calico](https://github.com/devkitPro/calico/blob/master/include/calico/nds/arm9/sound.h),
[stream Maxmod e reserva dos canais](https://github.com/devkitPro/maxmod/blob/master/source_calico/mm_stream.c),
[confirmação de comandos Maxmod](https://github.com/devkitPro/maxmod/blob/master/source_calico/mm_comms9.c).
