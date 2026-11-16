# Possibilidade de armas de X5 no debug de X4

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
