# Possibilidade de armas de X5 no debug de X4

## Atualização de 16/11/2026 — descompilação de anzaldoivan

O usuário indicou [anzaldoivan/mmx5](https://github.com/anzaldoivan/mmx5).
Inspecionada a revisão `120cb521f8b08b3e056d9738945cf950d004d07a`, publicada
em 14/11/2026. A consulta foi somente de código e documentação: nenhum script
do projeto foi executado e nenhum conteúdo de X5 foi integrado ao `.nds`.

Esta referência é mais adequada à investigação do banco nativo de armas:
há funções C em `src/slus`, `src/rockx5` e `src/shared`, estruturas de objetos
em `include/types.h` e documentação de formatos e recursos. Na amostra lida
há corpos de funções C, acessos por deslocamento e referências de memória
do PS1 que ainda precisam ser substituídas pelos serviços do port.

O [mapa de nomes compartilhados com X4](https://github.com/anzaldoivan/mmx5/blob/120cb521f8b08b3e056d9738945cf950d004d07a/docs/oracle/x4-names.md)
inclui pistas relevantes como `player_spawn_weapon`, `player_fire_weapon`
e `player_set_animation_shooting`. O próprio mapa diferencia funções já
compiladas em C (`gate`), assinaturas cujo corpo ainda é assembly (`bytes`)
e pistas de chamadas (`slot`). Os nomes são aliases de pesquisa; semelhança
de bytes não demonstra por si só que o papel de um objeto é igual.

Conclusão da inspeção: usar esta descompilação como referência principal
para localizar uma arma específica, suas tabelas e dependências. O projeto
reconstrói binários do PS1; isso não equivale a um port pronto para ARM9.
Não foi verificado que todas as armas já tenham rotinas C identificadas e
prontas para adaptação, nem foi executada a reconstrução com um dump de X5.

O próximo trabalho de integração continua sendo delimitar uma arma, mapear
seus tiros normal/carregado, adaptar atualização/colisão/energia, preparar
sprites/sons do dump do usuário e adicionar o banco experimental do debug.
Não é necessário incorporar o runtime de PC do MegaManX5Recomp para isso.

## Referência anterior — MegaManX5Recomp

Consulta de 15/11/2026: [MegaManX5Recomp](https://github.com/mstan/MegaManX5Recomp).
O README descreve configuração e recompilação automática para PC sobre
PSXRecomp. Informa que o repositório não inclui o disco, código de jogo
gerado ou uma descompilação C. O arquivo `game.toml` também configura
carregamento, BIOS, overlays e renderização próprios desse ambiente.

É tecnicamente possível desenvolver armas adicionais como extensão nativa
do debug deste port. Não é uma importação direta de arquivos desse repo.
Seria necessário adaptar movimento, colisão, dano, energia e animações ao
motor de X4, integrar texturas/sons e ampliar a seleção de armas do debug.

Proposta: um banco separado de armas experimentais, acessível apenas pelo
debug. O seletor original de X usa índices 0..8 e os oito indicadores dos
chefes; simplesmente acrescentar novos números nessa estrutura causaria
acessos a tabelas incompatíveis. O banco extra precisaria de atualização,
desenho e colisão próprios, usando os serviços nativos já existentes.

Para fidelidade aos sprites e sons originais, a preparação precisaria de um
dump de X5 fornecido pelo usuário. Nenhum dump de X5 foi fornecido nesta etapa.
Uma primeira implementação deve delimitar uma arma, seus tiros normal e
carregado e os efeitos contra inimigos/chefes. As demais viriam depois da
validação, mantendo a campanha normal e seus saves compatíveis.

Esta integração não faz parte da beta 8.3. O repositório foi examinado como
referência; não foi incorporado um segundo runtime de PlayStation ao port.
