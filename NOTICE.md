# Créditos e dependências

## Base do motor

O port adapta o trabalho de engenharia reversa de
[sozud/mmx4](https://github.com/sozud/mmx4), base
`4e16dfc7c06597fd5719dd112654d8cf85d343c2`, sob AGPL-3.0.
Os arquivos que derivam desse código mantêm atribuições nos comentários.
A licença está em `LICENSE`.

As fontes upstream são obtidas separadamente em `referencias/mmx4-decomp/`,
fora do Git deste projeto. `patches/0001-native-boss-safety.patch` contém as
alterações locais sobre os dois despachantes de chefe. O setup verifica a
revisão e aplica os patches de forma idempotente.

## Ferramentas e bibliotecas

- devkitARM/devkitPro: compilador e ferramentas obtidos do container oficial
  `devkitpro/devkitarm`, com camada fixada e verificada por SHA-256.
- libnds, Calico e Maxmod: APIs do console, tarefas e transporte de áudio.
  As bibliotecas são mantidas no SDK local, com suas próprias licenças.
- [sozud/psyz](https://github.com/sozud/psyz): revisão fixada pelo submódulo
  da base, usada na preparação e verificação de áudio no computador.
  O sintetizador de preparação não é incluído no runtime nativo do DSi.
- O formato do template de Makefile segue a organização usual dos exemplos
  devkitPro/libnds. As ferramentas e o SDK completos não são redistribuídos aqui.

## Recursos originais

Este repositório não concede direitos sobre Mega Man X4 ou sobre gráficos,
sons, músicas, vídeos, executáveis originais e firmware. Esses materiais não
fazem parte do código publicável e devem ser fornecidos localmente pelo usuário.
O executável `.nds` gerado contém recursos do dump e não é distribuído aqui.
