# Último teste físico — N06.2

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
