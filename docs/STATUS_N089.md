# Mega Man X4 nativo — beta 8.9

Saída local: `dist/native-n089/mmx4_native_n089.nds`, em modo DSi.
Compilar: `COMPILAR_JOGO.cmd` ou `COMPILAR_N089.cmd`.
Log: `/mmx4/native_n089_000.log`, ou o próximo número livre.

Pacote de 16/11/2026: 236.007.424 bytes (aproximadamente 225 MiB),
build `ebeca042224a8e5a3d86`, SHA-256
`160157c83839c443374002fdb1cdf765727d4730381e4406b0694151186634d9`.

## Sprites e efeitos recusados na fase 4

O [teste físico da 8.8](TESTE_N088_FASES_1_4.md) confirmou uma nova falha
visual. Uma reprodução com X no mesmo corredor encontrou o cache de
1.024 texturas cheio: os sprites seguintes eram recusados. As texturas da
imagem anterior precisam continuar protegidas enquanto a GPU as utiliza.

A 8.9 procura um bloco já convertido com **todos os pixels RGBA e todos
os índices iguais** antes de ocupar outro slot. O hash apenas escolhe onde
procurar; uma comparação completa decide o compartilhamento. Uma colisão
no hash não autoriza usar outra imagem. Aliases de recursos repetidos
são validados por uma sequência de reutilização do slot físico, evitando
referências antigas após a sua substituição.

Escala, montagem dos sprites, animações, cores, transparência, comandos,
prioridades e ritmo da lógica são preservados. Não há redução de resolução,
remoção de efeitos, aumento da VRAM ou relaxamento da proteção de duas
imagens. Blocos diferentes continuam diferentes, mesmo que tenham o mesmo
endereço de origem em outro carregamento ou usem uma paleta animada.

## Diagnóstico

`TEXTURE_SHARING` informa compartilhamentos por conteúdo e acertos de alias
no quadro amostrado. Uma recusa grava também `VIDEO_FAULT`, com motivo,
quantidade de comandos e ocupação protegida no instante da falha. O último
registro persiste entre quadros até a próxima escrita normal do log.

Motivos: 1 argumentos, 2 comandos, 3 paletas, 4 texturas, 5 índice de cor.
O limite físico ainda existe; o diagnóstico não transforma uma recusa em
sucesso nem substitui a verificação visual.

## Verificação e limites

O novo teste de carga usa a lógica original da fase de gelo, X e as nove
armas, com vida, checkpoint e posição controlados apenas no teste de
computador. A 8.8 falha no tick 2.354; a 8.9 completa 9.000 ticks e 4.500
desenhos sem recusar objetos. Não é uma reprodução exata dos comandos do
usuário, nem um teste de campanha completa ou medição de FPS.

Os testes sintéticos verificam igualdade com a regra escalar, índices e
cores transparentes, paletas animadas, aliases, slots protegidos, limites
reais com pixels únicos e envio à GPU. Os 48 testes de entrada em áreas
e 51 de sistemas incluem Continue, menus, vagões, upgrades e chefes.

A comparação com uma referência de cache ampliado **somente no computador**
produziu os mesmos hashes de geometria, RGBA e índices para os 61 quadros
completos ao redor da falha. O NDS continua com os mesmos 1.024 slots. Os 39.680 bytes de tabelas
auxiliares ficam na seção de memória exclusiva do DSi, inicializada pelo
SDK; não aumentam a área de 3,5 MiB reservada à compatibilidade DS.

O benchmark ARM946 também mede o custo da correção, sem medir FPS:

| Caso de 24 consultas 4bpp | Instruções na 8.8 | Na 8.9 |
| --- | ---: | ---: |
| Blocos novos distintos | 29.769 | 48.873 |
| Acertos no cache | 6.559 | 6.847 |
| Paleta estática entre quadros | 11.936 | 12.224 |
| Paleta animada | 170.240 | 224.258 |

São consultas sintéticas, não tempos de quadro do jogo. A busca por igualdade
adiciona trabalho em blocos novos; em compensação, blocos compartilhados não
ocupam outro slot nem exigem outro upload. Na carga da fase 4 foram 225.607
compartilhamentos por conteúdo e 150.549 acertos de alias, em 4.500 desenhos.
O saldo de desempenho e a ausência de piscadas precisam de teste no console.

A montagem da lista gráfica da 8.8 permanece. Compartilhar blocos evita
uploads repetidos, mas procurar conteúdo também tem custo; esta revisão
não promete um ganho de FPS sem medição física. A meta de 30 FPS
sustentados permanece aberta.

## Próximo teste no DSi

Repetir a fase 4 com X, especialmente o corredor com X em aproximadamente
4.500–4.700, usando tiros, dash e as armas. Conferir sprites, sombras,
efeitos, barra de vida e qualquer piscada. Depois repetir um trecho pesado
da fase 1 e comparar FPS e intervalos completos com personagem e
equipamento equivalentes. Enviar o novo log.

Continue permanece corrigido. Cutscenes continuam desativadas; os efeitos
seguem pelo ARM7. O `.nds` e os recursos extraídos ficam locais, fora do
GitHub público. Qualificação visual e FPS desta revisão dependem do DSi.
