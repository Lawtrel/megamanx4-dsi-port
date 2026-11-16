# Beta 8.2 — teste físico da fase 12

Registro recebido em 15/11/2026: `native_n082_000.log`, 712.873 bytes.
SHA-256: `51f10032e60048ff68d4ef6af77f98b8818745d6434612d1701a69ed29bc4b1b`.
Compilação: `f876e6285bcd22521168`, modo DSi (`DSi=1`). O arquivo original
e a análise detalhada permanecem locais, fora do Git.

## Origem e alcance

O usuário descreveu o percurso como fase 12, contra os oito chefes e depois
Sigma. O log contém duas sessões em seleção de teste (`campaign=0`,
`debug_loadout=1`): X começa na área interna 0 e chega à área 1; Zero começa
diretamente na área 1. Portanto, a sessão de Zero não registra as oito revanches.
Foram registradas 1.179 amostras no total.

Este retorno é uma validação parcial da fase final. Não confirma campanha
completa, todos os finais, salvamento/carregamento ou vitória sobre todas
as formas de Sigma. O fim do arquivo mostra Zero vivo após reaparecer na
área 1, sem marcador de conclusão do jogo.

## Desempenho observado

Filtro: `engine=6/0`, `locked=0`, HP do jogador maior que zero e
`menu=0/0/0`. A área interna 0 corresponde à área 1 do seletor; a interna 1,
à área 2. As linhas `fps` agregam aproximadamente um segundo; não são
tempos individuais de cada quadro.

| Trecho | Amostras | FPS mín./média/máx. | Amostras abaixo de 30 |
| --- | ---: | --- | ---: |
| X, área 1 — revanches | 421 | 27 / 47,21 / 60 | 13 |
| X, área 2 — Sigma | 183 | 30 / 49,81 / 60 | 0 |
| Zero, área 2 — Sigma | 156 | 33 / 51,16 / 60 | 0 |

São médias aritméticas das amostras filtradas, incluindo deslocamento e
combate. Considerando apenas amostras com chefe ativo, as médias são 45,48
para X nas revanches, 45,61 para X em Sigma e 48,47 para Zero em Sigma.
Os percursos e ações diferem; estes valores não comprovam vantagem de um
personagem nem ganho percentual sobre versões anteriores.

As 13 amostras de gameplay abaixo de 30 FPS concentram-se nos checkpoints
5, 7 e 9 da primeira área com X. O menor registro foi 27 FPS, com a simulação
a 60 ticks/s. Nas amostras de combate filtradas, a simulação ficou entre
58 e 63 ticks/s, próxima de 60; intervalos e contadores têm arredondamento.
Isso não demonstra que todos os quadros individuais cumpriram o orçamento.

## Custos medidos e próximo alvo

Médias por quadro nas amostras filtradas com chefe ativo:

| Trecho | Lógica | Composição/desenho | Preparação gráfica | Envio | Áudio |
| --- | ---: | ---: | ---: | ---: | ---: |
| X, revanches | 1,57 ms | 7,21 ms | 5,34 ms | 1,23 ms | 0,59 ms |
| X, Sigma | 1,75 ms | 7,28 ms | 5,00 ms | 1,21 ms | 0,59 ms |
| Zero, Sigma | 1,48 ms | 6,79 ms | 5,09 ms | 1,18 ms | 0,55 ms |

O maior custo medido está na composição e preparação gráfica. A próxima
investigação deve reproduzir os checkpoints 5/7/9 com X, separando cenário,
sprites, armadura/efeitos e montagem dos comandos. `present_us` mede o tempo
da rotina de envio, não a ocupação completa da GPU. A espera pelo VBlank e
o custo da gravação do log precisam ser considerados ao explicar quedas;
não basta somar as médias de CPU e prometer uma taxa de FPS.

## Contadores e retorno visual

- `audioerr=0`, `missingfx=0` e `overflow=0` em todas as amostras.
  Isso não cobre os seis comandos de sequência pendentes fora deste percurso.
- O contador `gfxerr` de X sobe de 0 para 9 em uma única janela, na área de
  Sigma, `frame=51419`, posição X=1381/Y=443, chefe com HP=8 e estado `1/2/3`.
  Depois permanece em 9; o de Zero permanece em 0. O contador é cumulativo
  por sessão, não nove falhas novas em cada linha posterior.
- Questionado sobre esse trecho, o usuário respondeu que não percebeu falha
  visual. O log não identifica o objeto ou motivo dessas nove ocorrências;
  permanecem pendentes de diagnóstico, sem causa atribuída por suposição.
- Não há linha de falha fatal registrada. Isso não prova ausência de qualquer
  travamento fora do trecho gravado.
- O cache PCM chegou a 1.798.752 bytes. O log confirma uso de vozes no ARM7;
  não apresenta uma medição de utilização percentual de cada processador.

A meta de 30 FPS sustentados ainda não está cumprida em todas as amostras
da primeira área com X. A revisão completa da campanha continua para o final,
conforme decisão anterior do usuário.
