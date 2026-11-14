# Mega Man X4 nativo — N07

Arquivo local: `dist/native-n07/mmx4_native_n07.nds`. Requer modo DSi.
A N06.7 foi preservada. Esta revisão prioriza campanha, menus e salvamento,
conforme a escolha do usuário em 14/11/2026.

## Integrado

- **Vagões da fase 8/área 2:** scripts, desenho e colisão compartilham o mapa
  original mutável `D_80141BE8`. Antes o evento alterava esse mapa enquanto
  o port usava uma cópia. As explosões apareciam, mas o cenário ficava intacto.
  A troca agora ocorre no instante previsto pelo evento original. Outros
  eventos que alteram esse mapa diretamente, como efeitos do gelo, também
  passam a ser refletidos no port.
- **Título e menus originais:** a entrada de campanha abre a animação do
  título e o menu de novo jogo/carregamento, seguido da escolha de personagem.
  O renderizador resolve as páginas completas de textura do título, diferentes
  dos bancos das fases. O seletor de fases de teste continua disponível.
- **Menus de slots:** os rótulos e ícones dos cartões ficam acima do fundo,
  incluindo o preview do personagem e os campos de progresso.
- **Campanha salva:** o carregamento fica acessível no menu original. Usa
  a rotina original para restaurar personagem, progresso, armadura, melhorias,
  vida máxima e botões. O armazenamento nativo mantém dois arquivos alternados
  com CRC, preservando a cópia anterior em caso de gravação incompleta.

Os arquivos ficam em `/mmx4/mmx4_save0_a.dat` e `mmx4_save0_b.dat` para o
primeiro cartão lógico, e nos equivalentes `save1` para o segundo. O formato
e os nomes existentes são mantidos; os saves anteriores não são apagados.

## Verificação e limites

Passaram **48 entradas de área e 34 cenários de sistemas** com verificação
de memória no computador. As dez imagens anteriores de chefes, pausa/opções
e salvar ficaram idênticas às da N06.6. O pacote inclui 33 imagens de
diagnóstico, incluindo título, slots de carregar e vagões alterados.

Os testes dos sete tipos de vagão começam depois do acerto confirmado e
executam a contagem original das explosões. Conferem o instante da troca e
o mapa consultado por desenho e colisão. O golpe com a Ride Armor precisa
ser repetido no DSi.

O teste de save prepara progresso conhecido, navega pelo menu original para
gravar em uma pasta temporária, fecha a sessão, limpa a referência do save
recente em memória e reabre o título. Carrega pelo menu e confere personagem,
progresso e melhorias no gameplay. Não escreve no SD do usuário.

A campanha e os chefes usam o código C original compilado para ARM9. A N07
integra esses caminhos; **não equivale à campanha inteira concluída e
qualificada**. Vídeos, comandos sonoros restantes, fidelidade de alguns efeitos
e 30 FPS sustentados continuam pendentes. Os testes no computador não
substituem a validação física no DSi.

## Teste no DSi

1. Na abertura, pressione START para abrir o título e START novamente para
   chegar ao menu original. B confirma e as setas navegam.
2. Na fase 8/área 2, golpeie o vagão com a Ride Armor, aguarde as explosões
   e confira a passagem aberta e a nova colisão.
3. No fim de uma fase, escolha salvar, selecione o cartão lógico e o slot
   e confirme. Aguarde a mensagem de conclusão antes de desligar.
4. Abra novamente e escolha carregar no menu original. Confira X/Zero,
   as peças equipadas, fases vencidas e melhorias.
5. Repita pausa/opções, gelo e transições para conferir regressões.

Log: `/mmx4/native_n07_000.log`, ou próximo número livre.
B pula/confirma, A dash, Y ataque, X especial, L/R arma e START pausa.
L+R+SELECT retorna à abertura de testes; SELECT+X alterna o som.

Compile com `COMPILAR_JOGO.cmd --no-pause` ou `COMPILAR_N07.cmd --no-pause`.
`COMPILAR_N06.cmd` continua como atalho de compatibilidade para o alvo atual.
O `.nds` contém recursos do dump e permanece local; somente o código é publicado.
