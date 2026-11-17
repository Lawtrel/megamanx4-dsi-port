# Mega Man X4 nativo — beta 8.15, preparação para beta 9

Saída local: `dist/native-n0815/mmx4_native_n0815.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `python3 native/compile.py`.
Log: `/mmx4/native_n0815_000.log`, ou próximo número livre.

## Painel e debug

Painel inferior com fundo azul escuro, cartões identificados por nomes curtos,
energia numérica e barras, contraste e aba selecionada destacados. Mantém
os ícones originais preparados do dump e a memória de vídeo H. O mapa
identifica o alcance da janela. A tela superior não recebeu alterações
nesta revisão; seu polimento precisa de alvos visuais concretos.

A aba Debug contém FASE, SIGMA1H e CREDITOS. FASE abre o seletor de teste.
SIGMA1H liga/desliga a redução da vida de Sigma para um único acerto válido,
respeitando suas introduções e o fluxo de morte original. CREDITOS abre
o encerramento original para o personagem atual, sem concluir nem gravar
artificialmente o progresso da campanha. FASE e CREDITOS encerram a sessão
atual; o progresso não salvo não é preservado. O título original permanece
como entrada padrão. Logs registram as ações de debug.

## X5 e limite desta entrega

O usuário pediu X5 jogável após os créditos como objetivo da beta 9.
Esta revisão ainda não entrega X5 nem se apresenta como beta 9 concluída.
No workspace há somente o dump de X4 e uma amostra de pesquisa de X5,
sem motor X5 integrado. É necessário o dump de X5 e adaptar seu motor
para ARM9, gráficos, colisão, áudio e recursos do DSi.

A descompilação de referência reconstrói o executável de PS1 e 55 overlays;
não é um alvo NDS pronto. A integração deve ocorrer no término real do
encerramento de X4 e iniciar um backend X5 jogável. Uma imagem de título
ou tela de espera não equivale a essa integração.

Referência: https://github.com/anzaldoivan/mmx5

## Verificação física pendente

Testar painel, nomes e energia, abas, Sigma 1 golpe em suas formas, acesso
aos créditos com X/Zero e custo da interface no DSi. O retorno físico das
campanhas de X/Zero e do radar na 8.13 permanece registrado e não é revogado.

## Créditos: resultado do teste no computador

O atalho percorreu 30.000 quadros com X e mais 30.000 com Zero, cobrindo
os oito subestados. Ambos chegaram a 11/7 e permaneceram com fade preto
(`alpha=255`), sem erros de desenho/áudio no teste. Isso reproduz o limite
pós-créditos relatado no DSi, mas não determina sozinho sua causa física.
O ponto de integração com X5 continua documentado em [X5 pós-créditos](X5_POS_CREDITOS.md).

## Pacote e verificações

Compilação ARM9 e pacote concluídos: 236,040,192 bytes, build `56e332940041529da82c`.
SHA-256 `b2bf36abf640526262b903bf5ffef4d0f0a88a2ec83dd7010e15cfa0a22d64e9`. Passaram os 48 percursos de fases e 53 cenários
de sistemas, além dos testes de UI, toque, áudio, conteúdo, saves, vídeo
e GPU. ASan/UBSan foram usados conforme cada suíte, com detecção de
vazamentos desligada no ambiente local. O polimento realizado nesta
revisão é do painel inferior; não se declara polimento completo da tela
superior. Esta versão ainda precisa de teste físico no DSi.
