"""InstagramStories - protótipo de editor de efeitos para imagens e vídeo (estilo stories).

Módulos:
    filtros    - base de filtros (suavização, realce, morfologia, cor, estilos)
    operacoes  - operações aritméticas entre duas imagens + texturas geradas
    pipeline   - sequência de etapas (combinação de filtros/operações)
    stickers   - stickers com canal alfa (estáticos e animados por spritesheet)
    rosto      - detecção de face (Haar Cascade / YuNet) e filtros de rosto
    webcam     - captura de vídeo (webcam ou arquivo) em thread separada
    console    - mensagens coloridas (azul) no terminal
    gui        - interface gráfica (Tkinter)
"""

NOME_APP = "InstagramStories"
__version__ = "1.1.0"
