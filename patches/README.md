# Patches do código de referência

Base: `sozud/mmx4` em `4e16dfc7c06597fd5719dd112654d8cf85d343c2`.

`0001-native-boss-safety.patch` limita a seleção de blocos do olho de gelo
quando o gerador está num estado degenerado e protege a alocação/espera do
efeito da morte do dragão. A semente inicial é ajustada pelo próprio adaptador
em `native/dsi/n05_flow.c`.

O setup usa `git apply --check` antes de aplicar e `--reverse --check` para
reconhecer um patch já aplicado. Ele não faz reset de alterações locais.
As verificações de layout 32 bits são geradas por `native/build_gamecore.py`.
