# Mega Man X4 nativo — beta 8.4

Saída local: `dist/native-n084/mmx4_native_n084.nds`. Requer modo DSi.
Compile com `COMPILAR_JOGO.cmd` ou `COMPILAR_N084.cmd`.

Pacote de 16/11/2026: 236.006.400 bytes (aproximadamente 225 MiB), build
`e810861dfaf8aba10308`. A beta 8.3 foi preservada separadamente.

## SELECT para Giga

- Durante o jogo, SELECT envia a ação original de Giga/R2: Giga Attack ou
  Nova Strike de X e Rakuhouha de Zero, conforme equipamento e habilidades.
- Permanecem os requisitos, estados de ação e consumo de energia originais.
  Segurar o botão não gera uma nova pressão a cada quadro.
- As combinações completas SELECT+X (som) e L+R+SELECT (voltar ao seletor)
  têm prioridade e consomem seus botões. Para alternar o som sem pressionar
  SELECT sozinho primeiro, segure X e toque SELECT com a combinação já formada.
- No seletor inicial, SELECT continua alternando campanha/seleção de teste.

O equipamento debug completo, as cutscenes desativadas e as otimizações da
8.3 permanecem. A alteração de controle não concede habilidades à campanha.

## Verificações

São 48 entradas de área e 48 cenários de sistemas, incluindo dois novos
cenários com o botão físico SELECT convertido e entregue ao motor original.
Os testes verificam ativação de X/Zero, gasto de energia, botão mantido,
combinações consumidas e ausência da habilidade quando não adquirida.
Nos testes, a invulnerabilidade isola o gasto do ataque da recarga por dano;
isso não altera a execução do jogo no console.

O cabeçalho do controle é conferido contra os bits de botões do SDK durante
a compilação ARM9 e entra na identificação do build. O pacote valida
fontes, relatórios e hashes internos antes da entrega.

Esta revisão ainda precisa de teste no DSi. A campanha completa continua
reservada para o final. O teste aleatório da 8.3 foi descrito pelo usuário
como funcionando bem, e ele confirmou em 16/11/2026 que as ocorrências
gráficas estavam corrigidas nessa versão. A 8.4 mantém essas correções;
seus novos controles ainda precisam de teste no DSi. Os contadores antigos
continuam documentados, sem serem tratados como uma falha visual pendente.

Controles: B pula, A dash, Y ataque, X especial, L/R troca de arma, START
abre o menu e SELECT aciona Giga. L+R+SELECT retorna ao seletor.
Log: `/mmx4/native_n084_000.log`, ou o próximo número livre.

O `.nds` contém recursos do dump e permanece local, fora do GitHub público.
