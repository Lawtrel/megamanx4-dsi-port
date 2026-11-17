# Mega Man X4 nativo — beta 8.14 em teste

Saída: `dist/native-n0814/mmx4_native_n0814.nds`, em modo DSi.
Compilar: `python3 native/compile.py` ou `COMPILAR_JOGO.cmd`.
Log: `/mmx4/native_n0814_000.log`, ou próximo número livre.

## Início no título original

O jogo agora abre diretamente o título original, sem passar pela seleção
experimental ou exigir START para iniciá-lo. Novo jogo e carregar continuam
nos menus originais. A campanha normal não recebe equipamentos do debug.

Painel, radar, áudio e diagnósticos da 8.13 permanecem. Dentro da aba Debug,
toque em TESTE DE FASE para abrir o seletor experimental. A tela avisa que
isso encerra a partida atual; o progresso não salvo não é preservado.
L+R+SELECT mantém o atalho. No seletor, SELECT alterna teste/menu original
e START confirma. Apenas a entrada explícita numa fase de teste concede
o equipamento de debug.

## Retorno do usuário e próximos passos

O usuário confirmou em 17/11/2026 que concluiu o jogo com X e com Zero,
derrotou Sigma e viu os créditos, sem problemas percebidos na campanha.
Também aprovou a tela inferior e não percebeu quedas com o radar ativo.
É validação física por relato; não foi fornecido novo log de tempos.

A tela preta após os créditos permanece relatada. O usuário propôs uma
futura integração com Mega Man X5 para o pós-créditos. Essa direção fica
registrada para pesquisa; esta versão não adiciona X5 nem altera o encerramento.
O áudio incompleto e configurações persistentes continuam no roadmap.

## Teste desta versão

Confirmar no DSi que a inicialização chega ao título sem o seletor, que
Novo jogo/Carregar funcionam, que o painel e radar continuam acessíveis e
que TESTE DE FASE só abre pela ação explícita no debug/atalho. Testar a
volta ao título pelo seletor e encerrar/reabrir uma sessão.

## Verificação local

Compilação e empacotamento concluídos: 236,039,168 bytes;
build `584fc083e5c0ae384ea4`; SHA-256 `35d9a97250ebf037ca9a61f5bc6793496d87eca014506c8dfcd00784ff4a706e`.
Passaram 48 percursos de fases e 51 cenários de sistemas, incluindo título,
Novo jogo/Carregar e reset da campanha normal após testes com equipamentos.
Também passaram os testes de toque (ação de seletor exclusiva da aba Debug),
renderer inferior com ASan/UBSan, conteúdo, áudio, saves, vídeo e GPU.
A entrada direta do frontend foi compilada para ARM9; a confirmação no DSi
continua pendente para esta revisão. Os pacotes anteriores ficam preservados.
