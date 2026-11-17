# Retorno físico — beta 8.11

O usuário confirmou que a 8.11 está rodando muito bem no DSi. O log contém
127 amostras de X equipado na fase 4. Nas 94 amostras de gameplay filtrado,
as médias foram de 49,53/54,12 FPS por área, mínimo 30 e máximo 59; nenhuma
ficou abaixo de 30. Sem erros registrados de desenho/áudio ou atraso de
restauração de VRAM. Houve 26 intervalos acima de dois campos de vídeo
entre 4.892 quadros medidos; a regularidade de todos os quadros e as outras
fases ainda exigem medição. A 8.11 passa a ser a base com retorno físico
positivo. Veja [dados, tempos e limites](TESTE_N0811_FASE4.md).

# Retorno físico — beta 8.10

O usuário confirmou o fim das piscadas no DSi. O log de X na fase 4 contém
202 amostras; no gameplay filtrado, médias de 20,31 e 27,91 FPS por área.
Sem atrasos de restauração de VRAM, underflow, overflow ou recusas de desenho.
A correção visual fica encerrada; 30 FPS sustentados continuam pendentes.
Veja [tempos e dados](TESTE_N0810_FASE4.md).

# Retorno físico — beta 8.9

O usuário confirmou queda forte de FPS e uma faixa preta intermitente no
alto da tela, em todas as fases. No log de X na fase 4, médias de 19,25 e
11,23 FPS por área; nenhuma recusa de desenho foi registrada. O vídeo
confirma a piscada no alto da imagem. A beta 8.10 trata cache e janela de
transferência como problemas distintos; a validação física dela está pendente.
Veja [dados, vídeo e limites](TESTE_N089_FASE4.md).

# Retorno físico — beta 8.8

256 amostras de X equipado nas fases 4 e 1. Médias por área entre 32,35 e
45,06 FPS, mínimo 23. O usuário confirmou sprites/efeitos sumindo ou piscando
na fase 4; o log acumulou três recusas de desenho. Essa falha nova motivou
a beta 8.9. Continue permanece corrigido. Veja [dados e limites](TESTE_N088_FASES_1_4.md).

# Retorno físico — beta 8.7

Log recebido em 16/11/2026: 438 amostras, build `8c80a93087b3ce08d6b0`,
modo DSi. Contém X e Zero na fase 1 e X na fase 4, todos no debug equipado.
Zero na fase 1 registrou médias de 37,11/45,08 FPS por área; X na primeira
área da fase 4 teve média 30,08 e mínimo 23. Contadores amostrados de
desenho/áudio/overflow ficaram em zero, sem relato visual adicional.

Ainda há quedas abaixo de 30. O teste da fase 4 na 8.6 foi com Zero;
não é comparação direta com X na 8.7. Veja [dados, custos e limites](TESTE_N087_FASES_1_4.md).

# Retorno físico — beta 8.6

Em 16/11/2026, o usuário testou as fases 1 e 4 e confirmou a correção de
Continue no DSi. `native_n086_000.log` contém 399 amostras e duas sessões
de Zero/debug, build `c62e6a2364a0c83f2b08`. No gameplay filtrado, as
médias por área foram de 32,53 a 43,75 FPS; houve quedas até 25.
Desenho, áudio, efeitos pendentes e overflow amostrado ficaram em zero.
A falha de Continue está encerrada; a meta de 30 FPS sustentados continua.
Veja [tabela e limites](TESTE_N086_FASES_1_4.md).

# Retorno físico — beta 8.5

Em 16/11/2026, o usuário relatou ter chegado à luta com Double usando X,
com 60 FPS nos trechos leves e cerca de 30 nos que têm efeitos. O log
`native_n085_000.log` contém 3.245 amostras em modo DSi, build
`726ddbc98cf94e34ca11`. No filtro de gameplay, 131 de 2.142 amostras
ficaram abaixo de 30 FPS (mínimo 25); ainda não há 30 FPS sustentados.

Após Game Over/Continue, o usuário viu cores erradas na sombra, tiros e
barra de vida. Os contadores de desenho/áudio/overflow ficaram em zero.
A falha da paleta de menu foi reproduzida no computador e motivou a beta
8.6, cuja correção foi posteriormente confirmada pelo usuário no DSi. Consulte
[tabela por área, causa e limites](TESTE_N085_CAMPANHA.md).

# Retorno físico — beta 8.3

Em 16/11/2026, o usuário informou que a 8.3 rodou bem num teste aleatório e
enviou `native_n083_000.log`, build `c0da08be8754468257a2`, modo DSi.
O teste cobre trechos de Zero na fase 11 e X na fase 5, introdução e trem.
Há quedas abaixo de 30 FPS e nove ocorrências de desenho na sessão de X,
com detalhe de erro de tile durante pausa na introdução. Áudio não registrou
erro. Não é uma medição controlada contra a 8.2, nem teste de Sigma.
Em esclarecimento posterior no mesmo dia, o usuário confirmou que as
ocorrências gráficas estavam corrigidas no teste da beta 8.3. A validação
visual fica confirmada por esse relato; os contadores do log permanecem
registrados sem manter, por si só, uma pendência de defeito visual.
Veja [tabela, origem e limites](TESTE_N083_ALEATORIO.md).

# Retorno físico — beta 8.2

Em 15/11/2026, recebido `native_n082_000.log`, build `f876e6285bcd22521168`,
confirmado em modo DSi. O usuário descreveu a fase 12 contra os oito chefes
e depois Sigma. O log cobre X nas duas áreas e Zero diretamente em Sigma.

No gameplay desbloqueado, vivo e fora do menu, os FPS mín./média/máx. foram
27/47,21/60 para X nas revanches, 30/49,81/60 para X em Sigma e 33/51,16/60
para Zero em Sigma. Houve 13 amostras abaixo de 30 na primeira área com X.
Áudio e overflow ficaram em zero; X acumulou nove falhas de desenho em Sigma,
sem falha visual percebida pelo usuário. A causa permanece pendente.

Veja [metodologia, custos e limites deste teste](TESTE_N082_FASE12.md).
Este retorno não qualifica a campanha completa nem confirma todos os finais.

# Retorno físico — N08

Atualização em 15/11/2026: o usuário decidiu não testar a N08.1 separadamente.
A retirada dos filmes será conferida junto com a futura beta 8.2. A N08.1
continua sem qualificação física; ver [o plano da próxima beta](PLANO_BETA_82.md).

Em 15/11/2026, o usuário informou que o `.nds` tinha aproximadamente 785 MiB,
os vídeos estavam lentos e o áudio fora de sincronia. Pediu retirar as
cutscenes. Não foi fornecido um novo log nesta mensagem. A N08.1 remove
filmes e seu áudio do pacote; esta revisão ainda requer teste no console.

# Retorno físico — N07

Em 14/11/2026, o usuário confirmou que a N07 está funcionando corretamente.
Não foi enviado novo log nesse retorno. A validação completa da campanha foi
reservada para o final a pedido do usuário.

# Último teste físico — N06.7

Log recebido em 14/11/2026: `native_n067_000.log`, identificador
`bf41f25d60a76c48a974`, modo DSi. SHA-256 do log:
`9998768a7ed17941d73be847c685b762edd88d9d2ebea431bc48b8b7f0585da0`.
O usuário confirmou funcionamento e relatou que o vagão mostra explosões,
mas permanece intacto na segunda área do trem.

| Gameplay desbloqueado | Amostras | FPS mín./média/máx. |
| --- | ---: | --- |
| X, Web Spider, área 1 | 40 | 26 / 38,98 / 59 |
| X, trem, área 1 | 43 | 30 / 47,63 / 60 |
| Zero, trem, área 2 | 178 | 28 / 39,19 / 59 |

As três sessões não registraram erros de imagem, áudio ou efeitos pendentes.
As médias são aritméticas das amostras `engine=6/0`, `locked=0`; há quedas
abaixo de 30 FPS. Percursos e personagens diferem: não são uma comparação
controlada entre versões. O relato não qualifica toda a campanha.

A N07 corrige o mapa separado que impedia a alteração dos vagões e integra
o título e seus menus de novo jogo/carregamento. Falta testar esses caminhos
no DSi; as verificações locais são registradas separadamente.

# Histórico — N06.5

Log recebido: `native_n065_001.log`, identificador `94aaed7d38aeff58e9d9`.
SHA-256: `b7473c4e0606acef884204de1ca808655e77550d424a17237bc8bc1750cba4f7`.
O arquivo contém 15 sessões de teste, com mudanças de área dentro de algumas
sessões. As fotos e o relato apontam fundo ausente na fase interna 8 e X com
aparência básica após adquirir uma peça na cápsula da fase interna 6.

| Gameplay desbloqueado | Amostras | FPS mín./média/máx. |
| --- | ---: | --- |
| X, gelo, área 1 | 123 | 25 / 29,95 / 49 |
| X, Magma Dragoon, área 1 | 44 | 23 / 26,34 / 42 |
| X, Magma Dragoon, área 2 | 66 | 24 / 30,15 / 50 |
| X, fase interna 6, área 1 | 83 | 27 / 38,41 / 59 |
| Zero, trem, área 1 | 47 | 41 / 58,40 / 60 |
| Zero, trem, área 2 | 24 | 31 / 54,71 / 60 |

São médias aritméticas das amostras com `engine=6/0`, `locked=0`. A média de
uma fase não significa 30 FPS sustentados. Os trechos diferem dos registros
anteriores; não são uma comparação controlada entre versões. Os resultados
do trem não qualificam o cenário completo, devido à falha de fundo.

Em Magma Dragoon, a simulação ficou próxima de 60 ticks/s nas amostras,
enquanto a primeira área ainda ficou abaixo de 30 FPS de desenho. O log
separa preparação e envio gráfico; os dois tempos devem ser considerados.

As correções da N06.7 foram verificadas no computador: repetição da câmera
e do cenário do trem, composição das peças de armadura e evento original de
aquisição. A confirmação física dessas correções ainda está pendente.

## Histórico — N06.4

Registro de 13/11/2026, fornecido pelo usuário: `native_n064_000.log` e
`native_n064_001.log`, ambos com identificador `b2962dcbbcbe9fa8ff9d`.
Hashes SHA-256 dos arquivos:

- 000: `9a7239830ed5e29d381ec56748df820803f5ef762b7bd030d9e798a5a490bd98`.
- 001: `72840789a31cb15bddaf83b2e612ae2a530cc385a495dd94e907646052501398`.

O usuário confirmou a derrota do dragão, o diálogo com Colonel, a passagem
pela base e seleção de fases, a derrota de Magma Dragoon e a apresentação
de Rising Fire. A tela posterior à introdução ainda omitia os textos de
salvar/continuar. As fotos mostram essa tela e o recebimento da arma.

| Gameplay desbloqueado | Amostras | FPS mín./média/máx. | Desenho médio | Envio médio |
| --- | ---: | --- | ---: | ---: |
| Web Spider, área 2, log 000 | 15 | 24 / 27,73 / 34 | 14,52 ms | 7,83 ms |
| Introdução, área 2, log 001 | 71 | 25 / 29,08 / 49 | 13,01 ms | 7,47 ms |
| Magma Dragoon, área 1 | 49 | 16 / 19,84 / 27 | 28,18 ms | 8,66 ms |
| Magma Dragoon, área 2 | 91 | 15 / 21,82 / 32 | 24,43 ms | 8,51 ms |
| Gelo, área 1 | 50 | 27 / 29,50 / 32 | 11,36 ms | 9,19 ms |

Recorte: `engine=6/0`, `locked=0`. As médias são aritméticas das amostras,
sem ponderação pela duração do intervalo. Os contadores de comandos/misses
descrevem o último quadro do intervalo; os tempos de perfil são médias por
quadro nesse intervalo. Trechos e versões diferentes não são benchmarks
controlados. Os 59 FPS das fotos pertencem a menus, não às fases.

Em Magma Dragoon, o custo de desenho domina, com picos médios acima de 36 ms
em um intervalo. O áudio fica perto de 1 ms por quadro. A simulação também
cai em parte desses trechos, apesar da recuperação limitada de ticks.
A N06.5 busca reduzir conversão de tiles, buscas de uploads e o trabalho
gráfico feito depois da espera de vídeo. Ainda não há resultado físico dela.

## Histórico — N06.3

Registro de 13/11/2026, fornecido pelo usuário: `native_n063_000.log`.
Identificador confirmado no cabeçalho: `0c98703e416352b68028`.
SHA-256 do log: `4122b544f2ede434472fdc8368cfaf2c3bed308f9cb78fb88de221042aa472a8`.

| Sessão e recorte | Amostras | Mínimo | Máximo | Média aritmética |
| --- | ---: | ---: | ---: | ---: |
| X, intro área 2, gameplay desbloqueado | 69 | 21 FPS | 37 FPS | 27,87 FPS |
| Zero, Web Spider, gameplay desbloqueado | 257 | 16 FPS | 47 FPS | 26,63 FPS |

O recorte usa `engine=6/0` e `locked=0`. São médias de intervalos registrados,
não medições ponderadas nem uma comparação controlada com a N06.2.

O usuário relatou maior estabilidade e melhora com o ARM7. Contudo, a morte
do dragão ficou em repetição: X permaneceu próximo de x=4468, y=443, bloqueado,
com a música parada. O vídeo confirma os raios de morte repetidos. O segundo
vídeo mostra pausa/opções e o retorno à luta; o usuário relatou inversão do
sprite após entrar nas opções. A orientação exata não fica clara nos quadros
inspecionados; a falha de paleta e os textos ocultos foram investigados no código.

A N06.4 corrige uma chamada incompatível que impedia remover filtros no ARM9,
a restauração da paleta ao sair das opções e a ordem de desenho dos textos.
A reprodução isolada em ARM e os testes de fluxo são evidências do computador;
o encerramento real da luta e a correção visual permanecem pendentes no DSi.

## Histórico — N06.2

Registro consolidado em 13/11/2026 a partir do log e da foto enviados pelo usuário.
O arquivo bruto e a foto ficam apenas no arquivo privado local.

## Identificação

- Versão: N06.2, ARM9 nativo, modo DSi.
- Build ID: `48459b969f389429f314`.
- SHA-256 do `.nds`: `7ad1c739299428e8fab9eca8d808da0af4fa775e3f736efecf36163d4111e758`.
- SHA-256 do log recebido: `760b3174a92b43cdf63f4a0c0db82b678dc645337ed30062e17ccfe2d614656f`.

A identificação do executável corresponde ao arquivo preservado localmente
para resolver os endereços de falha.

## Desempenho observado

| Recorte | Amostras | Mínimo | Máximo | Média aritmética |
| --- | ---: | ---: | ---: | ---: |
| Todas as amostras | 251 | 7 FPS | 42 FPS | 19,55 FPS |
| Gameplay com `engine=6/0`, `locked=0` | 209 | 7 FPS | 30 FPS | 17,70 FPS |
| Pausa com `engine=6/2` | 8 | 29 FPS | 39 FPS | 31,38 FPS |

São médias das amostras registradas, não medições ponderadas de toda a campanha.
A taxa de simulação também cai em trechos lentos; o log acumula ticks descartados.

Maiores tempos médios por quadro registrados em um intervalo do perfil:

- Lógica: 15.618 µs.
- Desenho: 42.564 µs.
- Áudio: 57.584 µs.
- Envio gráfico: 11.667 µs.

Os picos pertencem a intervalos possivelmente diferentes; não devem ser somados
como se fossem o mesmo quadro. A próxima otimização precisa medir desenho e
áudio no console, preservando a aparência do jogo.

## Falha ARM9

A última linha do log inicia uma sessão de teste com X na fase interna 1,
área interna 1. A foto mostra:

```text
X4 N06.2 - falha ARM9
Fase execucao:2 flags:2
PC:02073f10 LR:00000000
SP:020bfe88 CPSR:0000003f
Fase:1/1 X:0 Y:0
Tick:0
```

O endereço do PC foi resolvido usando o ELF dessa mesma build. Ele pertence
a `memcpy`; a instrução nesse endereço lê uma palavra a partir de `[r2 + 4]`.
Isso localiza a falha na cópia de memória durante a lógica inicial, mas não
identifica sozinho qual chamada forneceu o endereço. A foto não inclui todos
os registradores, e o LR mostrado não fornece um chamador útil nesse ponto.

**Situação: falha confirmada no hardware e ainda sem correção comprovada.**
A N06.2 não deve ser apresentada como estável ou como port completo.

## Testes no computador

Na base N06.2 passaram 48 entradas de área e 16 cenários de sistemas, além
dos testes de componentes. O roteiro do olho de gelo usa Zero com vida
protegida e confirmou dano, perda de blocos, derrota e passagem para a área
seguinte. A introdução usa preparação controlada do checkpoint e da morte do
chefe. Esses resultados verificam caminhos específicos e não anulam o crash
observado no console.
