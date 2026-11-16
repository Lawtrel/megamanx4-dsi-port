# Mega Man X4 nativo — beta 8.3

Saída local: `dist/native-n083/mmx4_native_n083.nds`. Requer modo DSi.
Compile com `COMPILAR_JOGO.cmd` ou `COMPILAR_N083.cmd`.

Atualização de 16/11/2026: o usuário confirmou que as ocorrências gráficas
estavam corrigidas no teste da beta 8.3. Esta é uma confirmação visual
relatada pelo usuário, sem qualificar a campanha inteira ou todos os FPS.

Pacote de 15/11/2026: 236.006.400 bytes (aproximadamente 225 MiB), build
`c0da08be8754468257a2`. A beta 8.2 permanece preservada separadamente.

## Correção e otimizações

- Corrigida a seleção de textura de elementos de diálogo que reutilizam
  objetos com um ponteiro antigo no campo `unk38`. Páginas de fontes e
  interface passam pela leitura do atlas, em vez do decodificador de poses.
  A falha foi reproduzida em teste com Sigma, todas as armas e armadura.
  O log da 8.2 não contém o objeto original: não é possível garantir que
  todas as nove ocorrências físicas tinham esta mesma causa.
- Consultas repetidas de limites e recursos usam cache associado à geração
  do conteúdo. As verificações de limites permanecem ativas. O cache é
  descartado em trocas de recursos e fechamento da sessão.
- Simplificado o cálculo das coordenadas das texturas, evitando arrays
  temporários e laços de inversão para cada quad. Mantém sequência de
  comandos, profundidade, ordem, cores, inversões, fade e transparência.
- Novos registros `RENDER_FAULT` identificam objeto, pose, textura e motivo
  caso uma falha reapareça. `RENDER_CACHE` registra reutilização das consultas.

Não há redução da escala ou resolução das texturas do jogo. Permanecem o
equipamento debug completo, áudio no ARM7 e cutscenes desativadas da 8.2.

## Verificação e teste

A regressão inclui 48 entradas de área e 46 cenários de sistemas. Os oito
cenários adicionais exercitam os checkpoints 5/7/9 das revanches e Sigma
com X/Zero, equipamento completo, armas carregadas e transição de diálogo.
Eles usam invulnerabilidade/energia cheia e preparação de estados apenas
no teste de computador para ampliar a cobertura; não comprovam campanha
completa ou vitória jogando normalmente.

O benchmark ARM946 compara instruções executadas na preparação gráfica e
hash dos comandos com a 8.2, incluindo cenário, HUD, fade e quads coloridos.
Ele não mede ciclos, acesso à memória ou FPS no DSi. O pacote é entregue
somente após a verificação das fontes, resultados e arquivos internos.

Na comparação com a beta 8.2, foram executadas menos instruções na preparação
de 600 quads: 4,2% no cenário, 4,3% com HUD, 2,2% durante fade e 4,9% em quads
coloridos. Os hashes dos comandos ficaram iguais em todos os casos. Esses
percentuais não representam aumento de FPS do jogo.

É necessário medir o novo `.nds` no console para avaliar o efeito nos FPS.
Priorize a fase 12, checkpoints 5/7/9 com X e o diálogo entre as formas de
Sigma. Envie `native_n083_000.log`, ou o próximo número livre.

No seletor, SELECT alterna campanha/teste; START inicia e L+R+SELECT retorna.
O teste inicia com armas/técnicas e upgrades completos, 48 HP e tanques.
A campanha mantém a progressão normal. O `.nds` contém recursos privados
do dump e não deve ser publicado.
