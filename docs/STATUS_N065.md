# Mega Man X4 nativo — N06.5

Arquivo local: `dist/native-n065/mmx4_native_n065.nds`. Use o mesmo carregador
em modo DSi da N06.4, que foi preservada. Esta revisão busca 30 FPS; ainda
precisa de medição no console e não garante essa taxa em todas as fases.

## O que mudou

- Conversão das texturas 4bpp por pares de pixels e das 8bpp por uma tabela
  de cores. As tabelas são refeitas quando a paleta muda. O cache de imagens
  conserva suas versões anteriores, transparência e preto opaco. Os dados
  de consulta do cache foram separados dos pixels para reduzir acessos
  desnecessários à memória. Há 192 KiB adicionais de tabelas em RAM.
- Preparação dos comandos gráficos antes da espera por VBlank. A preparação
  trabalha apenas em RAM; as transferências de textura e o envio para a GPU
  continuam no ponto de sincronização usado anteriormente.
- Consulta direta das texturas alteradas. Quadros sem novos tiles não
  transferem texturas nem remapeiam os bancos de VRAM. Isso elimina a busca
  por todas as 1.024 posições a cada apresentação.
- As opções de salvar/continuar usam objetos diferentes dos textos da pausa.
  Esses objetos agora são desenhados acima do fundo do menu. A tela foi
  conferida no teste com X e Zero, mantendo o fluxo original de seleção.

Resolução, recorte 1:1, regras das animações e lógica original foram mantidos.
O ARM7 continua com os oito efeitos de áudio e o ARM9 com lógica, desenho e
um stream de música. Os logs indicam que o próximo gargalo está no desenho;
esta revisão não adiciona tarefas artificiais ao ARM7.

## Evidências e limites

No log físico da N06.4, Magma Dragoon teve médias de 19,84 e 21,82 FPS nas
duas áreas. A preparação do desenho custou em média 28,18 e 24,43 ms por
quadro. O trecho de gelo registrado teve média de 29,50 FPS. São médias das
amostras de gameplay desbloqueado, não resultados da N06.5.

O teste de conversão compara 512 combinações de tamanhos e formatos com a
regra anterior, incluindo transparência, preto opaco e bordas parciais. O
teste gráfico também verifica que a preparação não toca a GPU/VRAM e que
um quadro atendido pelo cache não faz transferências ou remapeamentos.

Um teste isolado com instruções reais ARM946 compara 24 texturas novas com
a N06.4. Após aquecer a tabela de paleta, 4bpp passou de 209.943 para 31.182
instruções; 8bpp, de 181.892 para 66.654. As imagens resultantes têm os mesmos
hashes. O teste exclui o custo de refazer a tabela, latência de memória e
GPU: **instruções não são ciclos e essa redução não é ganho de FPS**.
Em acertos de cache, o custo do código medido ficou praticamente igual.

Passaram as 48 entradas de área e os 22 cenários de sistemas, além dos testes
de componentes. As oito imagens de regressão de pausa/opções/chefes ficaram
idênticas às da N06.4. As duas novas imagens da tela de salvar mostram os
textos e o cursor. Esses resultados são do computador, não do DSi.

## Teste no DSi

1. Repita Magma Dragoon com X, nas duas áreas, com som ligado. Compare locais
   equivalentes aos da N06.4; inclua cenário animado, disparos e a luta.
2. Repita o gelo, especialmente trechos com muitos efeitos.
3. Termine a introdução e confira os textos de salvar/continuar e a seleção
   de fases. Abrir a tela não basta para validar persistência do save.
4. Confira paletas animadas, neve/fogo, cores, transparência, pausa e áudio.

Logs: `/mmx4/native_n065_000.log`, com numeração crescente. A linha `PIPELINE`
acrescenta `prepare_us`; some esse tempo aos demais ao avaliar o trabalho
por quadro. `present_us` agora exclui a montagem dos comandos e não é
comparável sozinho ao valor antigo. `PROFILE` e `FIFO` preservam seus campos.

Controles: B pula/confirma, A dá dash, Y ataca, X usa especial, L/R trocam arma,
START pausa, L+R+SELECT volta à seleção de testes, SELECT+X alterna o som.
Compile com `COMPILAR_JOGO.cmd --no-pause` ou `COMPILAR_N06.cmd --no-pause`.

## Diagnóstico opcional de desempenho

O teste usa apenas dados sintéticos, SDK e histórico Git deste repositório:

```sh
python3 native/tests/benchmark_n065_video.py --build-only
python3 -m pip install --target build/native/test-python unicorn==2.1.4
python3 native/tests/benchmark_n065_video.py --run-only --python-path build/native/test-python
```

Ele mede instruções do componente, sem executar um emulador de console ou
usar dados do jogo. Campanha completa, vídeos e comandos de áudio pendentes
continuam em qualificação.

Referência do envio gráfico e sincronização: [implementação oficial de glCallList no libnds](https://github.com/devkitPro/libnds/blob/master/include/nds/arm9/videoGL.h).
