# Mega Man X4 nativo — N06.7

Arquivo local: `dist/native-n067/mmx4_native_n067.nds`. Use o mesmo carregador
em modo DSi. As versões anteriores foram preservadas. Esta revisão mantém
a base de otimização da N06.6 e corrige o fundo do trem e o desenho da armadura.

## Correções

**Fase 8, trem:** a lógica original atualizava a posição das camadas repetidas
enquanto preenchia o cache de tiles. O port desenha diretamente do mapa e
pulava essa atualização. As camadas continuavam avançando para fora do mapa,
deixando o céu preto. O port agora executa a mesma regra de retorno da câmera
e lê o cenário dos dois lados da emenda da faixa repetida.

**Armadura de X:** o jogo já registrava a peça adquirida, mas o renderizador
nativo desenhava somente a montagem básica do personagem. Agora ele compõe
as peças equipadas a partir das tabelas originais, compartilhando a pose
descomprimida e respeitando a ordem das peças, orientação e paleta dos braços.
Zero mantém seu desenho próprio. A regra de aquisição e os dados de save
continuam sendo os do motor original.

## Verificações

- Trem nas duas áreas, com X e Zero, por 3.600 ticks em cada cenário. O teste
  observa camadas desenhadas durante milhares de ticks e repetidas voltas da
  câmera. A regra de retorno também é comparada à função C original em dez
  posições de limite, incluindo coordenadas negativas.
- As 16 combinações de peças de X, nos dois sentidos e com os dois tipos de
  braço, em 64 combinações de desenho. Os movimentos/armas do teste anterior
  também passam a exercitar o desenho de armadura.
- Cápsula da fase interna 6, checkpoint 5: aproximação, diálogo, entrada,
  aquisição, retorno do controle e presença da peça no desenho. O teste usa
  o evento original para adquirir a peça, sem atribuir a máscara da armadura.
  A posição inicial e a vida são preparadas pelo roteiro de teste.

Os testes e imagens do computador não substituem a conferência dessas
correções no DSi. Ao voltar a desenhar o fundo completo, a fase 8 passa a
processar mais gráficos; os 60 FPS da imagem anterior não validavam o cenário
completo. A meta continua sendo 30 FPS com os gráficos presentes.

Passaram 48 entradas de área e 28 cenários de sistemas. As dez imagens
anteriores de chefes, pausa/opções e salvar ficaram idênticas às da N06.6.
O pacote acrescenta imagens das duas áreas do trem e de X com e sem peças.

## Resultado físico recebido — N06.5

O log `native_n065_001.log`, com identificador `94aaed7d38aeff58e9d9`, registrou
média de 29,95 FPS no trecho de gelo, 26,34 na primeira área de Magma Dragoon
e 30,15 na segunda. São médias de amostras de gameplay desbloqueado, em trechos
distintos; não são um benchmark controlado entre versões. Os problemas de
fundo e armadura foram relatados nessa revisão. Ainda não há resultado físico
da N06.7 nem confirmação de 30 FPS sustentados na campanha completa.

## Teste no DSi

1. Com X ou Zero, jogue as duas áreas da fase 8 e confira o céu e o cenário
   distante por pelo menos um minuto, incluindo as transições.
2. Com X, adquira a peça na cápsula da fase 6. Saia dela, ande, pule, atire e
   entre na pausa. Confira a aparência equipada e, ao trocar de fase, sua
   permanência. Repita com as outras peças quando possível.
3. Repita Magma Dragoon/gelo para comparar FPS em trechos equivalentes.
4. Envie `/mmx4/native_n067_000.log`, ou o próximo número disponível.

A linha `BOSS` também registra `bg=a/b/c`, `armor` e `mask`: quantidades
desenhadas nas camadas e na armadura e a máscara equipada, referentes ao
último quadro. Os demais campos de desempenho são mantidos.

Controles: B pula/confirma, A dá dash, Y ataca, X usa especial, L/R trocam arma,
START pausa, L+R+SELECT volta à seleção de testes, SELECT+X alterna o som.
Compile com `COMPILAR_JOGO.cmd --no-pause` ou `COMPILAR_N06.cmd --no-pause`.

Vídeos, alguns comandos de áudio, fidelidade de efeitos e campanha completa
continuam em qualificação. O `.nds` inclui recursos do dump e permanece local.
