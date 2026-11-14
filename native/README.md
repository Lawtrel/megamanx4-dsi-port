# Motor nativo ARM9

O alvo atual é a N07, experimental. Estado do produto e resultado físico
estão em [README](../README.md) e [testes no console](../docs/HARDWARE_TESTS.md).

## Componentes

- `dsi/n05_flow.c`: estado original do jogo e entrada nativa por fase/campanha.
- `dsi/n05_flow_loading.c`: recursos locais, arena e carregamentos adicionais.
- `dsi/n05_tasks.c`: tarefas cooperativas com Calico e guardas de pilha.
- `dsi/n05_render.c`: composição de sprites, cenário e HUD.
- `dsi/n05_flow_ui.c`: pausa, paletas e elementos de interface.
- `dsi/n06_gpu.c`: comandos gráficos nativos enviados por DMA à GPU do DS.
- `src/x4_video.c`: cache de texturas e proteção dos quadros em uso.
- `src/x4_audio.c`, `x4_music.c`, `x4_sfx.c`: reprodução e mistura PCM nativas.
- `tests/`: verificações de memória, fases, sistemas, cache e comandos gráficos.

O código original C é compilado para ARM9. A ferramenta de áudio roda no
computador para converter dados; ela não é vinculada ao executável do DSi.
Os endereços e nomes históricos do código descompilado não representam uma
CPU de PlayStation interpretada durante a execução.

## Dependências e geração

`tools/setup_native.py` fixa a revisão de `sozud/mmx4`, verifica o submódulo
`psyz` e aplica os arquivos de `patches/` sem descartar mudanças desconhecidas.
`build_gamecore.py` gera os cabeçalhos com os ajustes de layout 32 bits,
preservando as verificações de tamanho relevantes.

`compile.py` prepara os recursos privados, verifica os componentes e chama
`build_n05_flow.py` e `package_n05_flow.py`. O modelo de Makefile faz parte do
código em `Makefile.nds`; `build/` é sempre gerado.

A qualificação física é indispensável. A N07 mantém os efeitos pelo ARM7
em `dsi/n063_arm7_audio.c` e publica o PCM imutável apenas na entrada do cache.
As tabelas de pixels em `x4_video.c` aceleram a conversão de tiles novos.
`x4_gpu_prepare` monta os comandos em RAM antes da espera de vídeo;
`x4_gpu_present` agrupa texturas alteradas adjacentes sem cruzar posições
limpas ou bancos de VRAM. Paletas estáticas são comparadas com uma cópia
das cores originais antes de reconstruir as tabelas.
`benchmark_n065_video.py` aceita uma revisão-base e um cenário de teste;
a N07 usa a N06.5 como base. Ele não mede FPS. A correção anterior do dragão permanece coberta
pelo diagnóstico `build_n064_arm_abi.py` e `run_n064_arm_abi.py`.

A N07 recompõe as peças equipadas usando as tabelas originais de X e a
mesma pose já descomprimida. A atualização de retorno das câmeras de fundo,
que antes era feita no cache gráfico original, agora é executada pela ponte
de desenho. O mapa também é amostrado dos dois lados da faixa repetida.

Na N07, `x4_flow_title` inicia a tarefa original do título, com as texturas de
página completa que ele usa. O carregamento passa pelos menus e pela rotina
original de restauração do personagem. Cenário e colisão usam o mesmo mapa
mutável `D_80141BE8` modificado pelos eventos; não há cópia paralela do mapa.
Os testes de integração de menus e eventos estão em `tests/n07_systems.h`.
