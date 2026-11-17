# Mega Man X4 nativo — beta 8.13 em teste

Saída: `dist/native-n0813/mmx4_native_n0813.nds`, em modo DSi.
Compilar: `python3 native/compile.py --n0813` ou `COMPILAR_N0813.cmd`.
Log: `/mmx4/native_n0813_000.log`, ou próximo número livre.

## Tela inferior

Três abas por toque no rodapé: Painel, Debug e Mapa. O jogo continua rodando;
START mantém a pausa original. A tela superior e a simulação mantêm seu caminho.

O painel usa imagens do menu original preparadas localmente a partir do dump.
Em X, toque numa arma adquirida para solicitar sua seleção, respeitando os
bloqueios originais; poderes não adquiridos não são equipados. Em Zero, técnicas
mostram detalhes. Upgrades são consultivos; tocar num tanque não o consome.
Vida, vidas, energia e aquisição vêm do estado real da partida.

O radar acompanha uma janela de 512×288 unidades da área, com terreno
simplificado, jogador, inimigos e itens ativos visíveis. Não adiciona marcadores
de itens ocultos nem provoca spawn. O mapa usa o terreno vivo, inclusive alterações de blocos. Fora da fase,
mostra indisponibilidade. O debug mantém os diagnósticos e os logs permanecem
ativos nas três abas. A atualização da interface é limitada a 10 Hz.

Os recursos gráficos inferiores usam o banco H. Os bancos A–G permanecem
reservados ao renderer superior. Ícones são reduzidos a uma paleta comum de
16 cores apenas na tela inferior; os gráficos da tela superior não mudam.

## Áudio e teste físico

Preserva o áudio da 8.12: panning espacial de efeitos diretos no ARM7,
mono/estéreo por reprodução e ganho preservado ao silenciar/reativar.
Os seis comandos pendentes, loops aproximados e reverberação continuam
pendentes; esta revisão não encerra o passo 2 do roadmap.

Necessário testar no DSi: legibilidade dos ícones, toque nas abas/armas,
consulta de upgrades, mapa nos limites da área, mudança de personagem,
pausa original, entrada/saída de debug e custo da interface na fase 4.
Compare com a 8.12 e envie `native_n0813_*.log`. Testes locais não confirmam
FPS nem estabilidade física.

## Verificação local

Pacote de 17/11/2026: 236,039,168 bytes; build `2afc447962d1ad5041b6`.
SHA-256 `0cf11a3305faab3b1e8e3fa99bcb8adbeca4229cf890bb6d3089980bfa818610`.

Passaram os 48 percursos de entrada e 51 cenários de sistemas, incluindo
limites da área no radar, leitura sem alterar estado, terreno vivo, marcadores
e troca de armas por toque com X/Zero. O teste do Aiming Laser verifica a
inicialização dos objetos da arma, além do índice selecionado.

O renderer inferior passou ASan/UBSan, testes de toque e compilação ARM9.
Foram inspecionadas prévias com os ícones reais e a fonte do libnds.
Também passaram os testes de áudio, conteúdo, saves, vídeo e GPU.
A detecção de vazamentos foi desligada na execução local por restrição do ambiente.
O log `LOWER_SCREEN` informa modo, atualizações, tiles enviados e tempo médio/máximo.
A confirmação de estabilidade e desempenho continua dependendo do DSi.
