# Exercício 3 — Resposta

Usando a câmera 2D do exercício 2 (janela do mundo `xmin=0, xmax=800,
ymin=600, ymax=0`, implementada em `src/exercicio3.cpp` desenhando 5
quadrados em posições diferentes da tela) — o que acontece e por que essa
configuração é útil:

## O que acontece quando posicionamos objetos

Como a janela do mundo tem exatamente as mesmas dimensões da janela da
aplicação (800×600) e o eixo Y está invertido (`ymin=600` embaixo,
`ymax=0` em cima), **1 unidade do mundo passa a valer exatamente 1 pixel
na tela, com a origem (0,0) no canto superior esquerdo** igual a como
imagens, telas e frameworks de UI/2D endereçam pixels.

Um quadrado desenhado com centro em `(60, 60)` aparece perto do canto **superior esquerdo**; um quadrado
centrado em `(740, 60)` aparece no canto **superior direito**; centrado
em `(400, 300)` aparece bem no **meio da tela**. Ou seja, a posição que a
gente usa no código já *é* a posição final do objeto na tela, em pixels 
não precisamos converter nada.

## Por que essa configuração é útil

Sem essa configuração, para posicionar algo teríamos que converter onde
eu quero que apareça, em pixels para coordenadas normalizadas (-1 a 1)
manualmente, com fórmulas como:

```
ndcX = (pixelX / larguraTela) * 2.0 - 1.0
ndcY = 1.0 - (pixelY / alturaTela) * 2.0
```


