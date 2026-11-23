# Megaman X4 Ds · beta 9

Abra `dist/native-n09/mmx4_ds_beta.nds` no modo DS ou DSi. O jogo começa no
título original. Os logs ficam em `/mmx4/native_n09_000.log` ou no próximo
número livre.

## Como iniciar no DS, no DSpico e no emulador

O mesmo arquivo contém o jogo e os recursos de áudio e vídeo. No DSpico, copie o
`.nds` para o microSD e abra-o pelo Pico Launcher com o Pico Loader. No DSi,
escolha o modo DSi/TWL no carregador para usar o clock de 134 MHz, o cache de
24 poses e até 4 MiB para efeitos sonoros. Em modo DS/NTR, o ARM9 usa 67 MHz,
com 4 poses e até 96 KiB para efeitos. Os efeitos que excedem o cache são
reproduzidos por leitura do arquivo. O log identifica o perfil em `PROFILE_HW`.

Para reproduzir os testes de emulação, use o núcleo **melonDS DS 1.4.0**,
inicialização direta, renderização por software e JIT desativado. Selecione DS
para emular o console original. O cartão virtual de homebrew (DLDI) permite
gravar os logs e saves. Para DSi, selecione esse console e configure a BIOS,
o firmware e a NAND correspondentes.

Na compilação `203399c63432e6717b90`, X saiu do título e alcançou o checkpoint 1
da introdução nos dois modos. O log registrou 67 e 134 MHz, respectivamente.
O teste em DS abriu as 13 fases pelo seletor com X e Zero e 4 MiB de RAM.
Foram 48 entradas em fase, incluindo a segunda área das 11 fases que a oferecem. O teste em DSi usou 16 MiB emulados e FreeBIOS na região
inferior da BIOS ARM7 de teste. A cópia local original tem os vetores iniciais
preenchidos com FF e travou antes de iniciar o jogo.

O driver oficial DSpico DLDI v1.0.1 coube na área reservada do binário em um
teste de aplicação com dlditool. A execução no cartucho e os tempos de quadro
desta compilação no hardware continuam pendentes. O DeSmuME testado também
ficou pendente. O emulador validado para jogar é o melonDS DS.

## O que mudou

- A tela superior volta ao título de X4 depois do fade dos créditos. O
  escalonador mantém a tarefa do título ativa quando a rotina do final termina.
- Na tela inferior, a faixa de vida ganhou um fundo, os poderes disponíveis
  ficam em cartões destacados e uma linha divide a área de detalhes.
- No Debug, **SIGMA1H** deixa Sigma com 1 HP quando ele pode receber dano.
  **CRÉDITOS** abre o final original do personagem atual e encerra a partida.
  **FASE** abre o seletor de testes.

## Teste no DSi

O usuário aprovou o visual e confirmou que os créditos terminaram sem
problemas e voltaram ao título. O log `native_n09_000.log` identifica a
compilação `0a425555a7f050e40731` em modo DSi. O resultado se refere a
esse pacote. Uma recompilação recebe outro identificador. O percurso do final foi
com X, na segunda área da fase 12 pelo seletor de debug. O registro passa
da etapa final dos créditos (`11/7`) ao título (`14/0`, faixa `32`).

| Trecho com X | Amostras | FPS mín. / média / máx. |
| --- | ---: | ---: |
| Introdução, área 1 | 49 | 30 / 45,71 / 59 |
| Introdução, área 2 | 26 | 30 / 39,88 / 51 |
| Fase 12, área 2 | 85 | 27 / 43,12 / 60 |
| Créditos | 212 | 52 / 59,91 / 60 |

O recorte de gameplay usa `engine=6/0`, jogador vivo e controle livre.
A fase 12 teve uma amostra abaixo de 30 FPS. As 640 amostras do log
registraram zero erros de imagem, áudio e efeitos pendentes. O retorno ao
título com Zero e a abertura do menu com START foram verificados no computador.

SHA-256 do log `41be1154f6b0ca6a75944d3bc139d7b4c84d79f87c3750afd8128cf47de9575a`.

## Limites desta compilação

A versão mantém as cutscenes desativadas. O polimento visual da tela superior
nesta compilação se limita ao retorno do final. Mega Man X5 ainda não está
jogável neste pacote. Veja [X5 após os créditos](X5_POS_CREDITOS.md).

## Compilar outra cópia

Execute `python3 native/compile.py --n09` no Linux ou no WSL. Guarde o `.nds` localmente, pois ele contém recursos do seu dump.
Consulte [o guia de compilação](COMO_COMPILAR.md).
