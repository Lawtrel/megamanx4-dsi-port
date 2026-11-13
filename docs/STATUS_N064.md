# Mega Man X4 nativo — N06.4

Arquivo local: `dist/native-n064/mmx4_native_n064.nds`. Inicie em modo DSi,
pelo carregador usado na N06.3. As versões anteriores foram preservadas.

## Correções

- A morte do dragão chamava a remoção dos filtros através de uma assinatura
  incompatível. No ARM9, isso enviava +130, em vez do identificador negativo
  -126. Os filtros da fase continuavam ativos, podendo impedir o efeito de
  morte de alcançar o branco final e liberar a sequência seguinte. A chamada
  agora respeita o tipo original. Um teste isolado com as instruções reais
  ARM946 reproduziu a diferença: a chamada antiga preserva dois filtros, a
  corrigida remove ambos. O teste completo de saída da intro também passa
  com X na posição observada no log. A confirmação final é no DSi.
- Ao voltar das opções, o port restaura a paleta do cenário antes de desenhar
  a pausa e o gameplay. Textos e cursores das opções agora ficam acima do
  fundo do menu. Testes de X e Zero verificam entrada, saída, paleta e retorno
  ao jogo. A inversão relatada ainda precisa ser conferida no console.

## Desempenho e os dois processadores

O ARM7 e os canais de som continuam reproduzindo os oito efeitos. A música
permanece em um único stream do ARM9. Agora cada amostra imutável é publicada
na memória compartilhada apenas ao entrar no cache; repetir um efeito não
limpa novamente o mesmo PCM. A confirmação de parada continua obrigatória
antes de liberar ou reutilizar o buffer.

A composição gráfica agrupa comandos e reutiliza os estados de cor, textura
e polígono. O teste de 1.500 blocos uniformes passou de 31.500 para 18.379
palavras enviadas, redução de 41,65%. Isso mede tráfego de comandos, **não
ganho de FPS**. O cenário também descarta blocos fora da área visível antes
de consultar mapa e atributos. Escala, recorte 1:1 e resolução foram mantidos.

O log da N06.3 fornecido pelo usuário registrou média de 27,87 FPS nas amostras
desbloqueadas de X e 26,63 nas de Zero. São trechos diferentes; não constituem
benchmark entre versões. A N06.4 ainda não tem medição física. Não há garantia
de 30 FPS sustentados ou de 60 FPS.

## Teste no DSi

1. Com X, mate o dragão na segunda área da introdução. Espere o fim dos
   efeitos e avance os diálogos com B. Confira a saída para a seleção de fase.
2. Com Zero em Web Spider, abra pausa, entre nas opções, volte à pausa e ao
   jogo. Confira orientação, cores, textos e controles. Repita com X.
3. Compare o mesmo trecho com a N06.3, com som ligado. Atire, dê dash e use
   o sabre. Confira cortes de som, efeitos presos e FPS.
4. Repita a troca de personagem e o miniboss olho de gelo.

Logs: `/mmx4/native_n064_000.log`, com numeração crescente. `BOSS` registra os
estados de morte, filtro, diálogo e menu. `FIFO` mede palavras enviadas e o
custo antigo equivalente. `ARM7` e `PROFILE` mantêm as medições anteriores.

Controles: B pula/confirma, A dá dash, Y ataca, X usa especial, L/R trocam arma,
START pausa, L+R+SELECT volta à seleção de testes e SELECT+X alterna o som.

## Compilar e reproduzir o diagnóstico

`COMPILAR_JOGO.cmd --no-pause` e `COMPILAR_N06.cmd --no-pause` geram a N06.4.
O NDS contém os recursos do dump e permanece fora do GitHub.

O teste de instruções ARM é separado e não precisa de recursos do jogo.
Depois de preparar o SDK e gerar os cabeçalhos com `native/build_gamecore.py`:

```sh
python3 native/tests/build_n064_arm_abi.py
python3 -m pip install --target build/native/test-python unicorn==2.1.4
python3 native/tests/run_n064_arm_abi.py --python-path build/native/test-python
```

Campanha completa, vídeos e alguns comandos de áudio continuam em qualificação.
Passaram 48 entradas de área, 20 cenários de sistemas e os testes de componentes.
As seis imagens de regressão de cenário/pausa/chefes são idênticas às da N06.3;
as opções têm imagens novas, com os textos e cursores visíveis.
Referência da correção: [AAPCS32, extensão dos argumentos menores que uma palavra](https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst).
