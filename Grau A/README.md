# TERRARIA — Trabalho Grau A (Processamento Gráfico: Fundamentos)

Protótipo do jogo terraria (meu fav) de exploração e mineração
feito em **C++17 + OpenGL 3.3 core**. O jogador anda por um mundo de blocos gerado proceduralmente,
cava, coleta recursos, constrói e se defende de slimes e zumbis, com ciclo de dia e noite.

## Componentes

- João Pedro — Ciência da Computação (Unisinos) — trabalho individual

## Vídeo de apresentação

Link: https://youtu.be/wAAd5dNeOZA

## O que o jogo tem

- Mundo de 300 x 100 blocos gerado por código: relevo com colinas, camadas de grama/terra/pedra,
  cavernas, minérios (carvão e ferro), árvores e bedrock indestrutível nas bordas.
- Personagem com spritesheet animada (parado, andando, pulando, minerando com picareta).
- Minerar blocos (cada tipo tem uma dureza; aparece a animação de rachadura) e colocar blocos de volta.
- Barra de itens (canto superior esquerdo): picareta, espada e 6 tipos de bloco com contador.
- **Sistema de batalha**: espada (animação com rotação e hitbox), recuo nos inimigos, barra de vida dos inimigos e números de dano.
- Inimigos: **slimes** (pulam em direção ao jogador) e **zumbis** (andam atrás do jogador à noite e pulam). Ao morrer soltam moedas e, às vezes, corações que curam.
- Corações de vida no canto superior direito, com regeneração lenta; **dano de queda** (cair de muito alto tira corações).
- **Texto do HUD com FreeType**: fonte TrueType (DejaVu Sans Mono Bold) lida em tempo de execução, em três tamanhos, com acentos em português.
- Tela inicial e tela de game over.
- Ciclo dia/noite: o céu escurece, as estrelas aparecem, as cavernas são mais escuras e à noite surgem zumbis e mais slimes.
- Fundo com 6 camadas de **parallax** (céu, estrelas, nuvens, montanhas, colinas distantes e próximas).

## Controles

| Ação | Tecla / mouse |
|---|---|
| Iniciar / recomeçar | ENTER |
| Andar | A / D ou setas |
| Pular | W, seta para cima ou ESPAÇO |
| Minerar bloco (picareta/blocos) | Botão esquerdo (segurar) |
| Golpear com a espada (slot 2) | Botão esquerdo, na direção do mouse |
| Colocar bloco | Botão direito (com um bloco selecionado) |
| Escolher item da barra | 1 a 8 ou roda do mouse |
| Sair | ESC |

Cair de mais de 6 blocos de altura causa dano (quanto mais alto, mais corações). O alcance para minerar e construir é de 5 blocos ao redor do personagem (o bloco sob o mouse é destacado quando está ao alcance).
Só é possível colocar um bloco encostado em outro bloco.

## Como compilar e executar

Dependências: **CMake 3.15+**, compilador C++17, **GLFW 3** e **FreeType** (instaladas via vcpkg, igual às listas).
GLAD, GLM e stb_image já estão na pasta `external/` (não é preciso instalar).

### Windows (VS Code + CMake Tools + vcpkg)

1. Instale as bibliotecas: `vcpkg install glfw3 freetype` (triplet `x64-windows`).
2. Abra a **pasta do projeto** (a que contém o `CMakeLists.txt`) no VS Code.
3. Escolha o kit *Visual Studio ... amd64* e use o toolchain do vcpkg, como nas listas
   (`-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake`).
4. Configure, compile e execute o alvo `jogo`.

Pela linha de comando (PowerShell):

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
.\build\Release\jogo.exe
```

## Estrutura do projeto

```
assets/
  config.txt            arquivo de configuração (janela, mundo, jogador, fonte, spritesheets, camadas de parallax)
  shaders/sprite.vert   vertex shader dos sprites        sprite.frag  fragment shader dos sprites
  shaders/text.vert     vertex shader do texto           text.frag    fragment shader do texto
  fonts/                fonte TrueType do HUD (DejaVu Sans Mono Bold) e sua licença
  *.png                 texturas (tiles, personagem, slime, zumbi, itens, drops, HUD e camadas do fundo)
src/
  main.cpp              ponto de entrada
  Game.h                a classe Game (laço e regras), implementada em três arquivos, um por fase do laço:
    Game.cpp              inicialização, laço principal (run) e processInput
    GameUpdate.cpp        update e as regras: dano, drops, mineração, espada, inimigos
    GameRender.cpp        render: fundo (parallax), mundo e HUD
  Timer.h               Timer da aula (tempo do quadro e espera para limitar o FPS)
  Input.h               "foto" do teclado e do mouse em um quadro
  Camera.h/.cpp         janela ortográfica do mundo: segue o jogador, limites do mapa, mouse -> mundo
  DayNight.h/.cpp       hora do dia e iluminação
  Inventory.h/.cpp      barra de itens
  Player.h/.cpp         jogador: movimento, queda, regeneração, golpe da espada, animação
  Enemy.h/.cpp          slime e zumbi: criação e IA
  Drop.h/.cpp           moedas e corações (e os números flutuantes de dano)
  Body.h/.cpp           corpo físico e moveAndCollide (colisão com os blocos)
  AABB.h                caixa de colisão (hitbox) e checkCollision
  World.h/.cpp          mapa de blocos, geração procedural, consulta de colisão
  SpriteRenderer.h      quad (VAO/VBO/EBO), matrizes de projeção e de modelo, desenho de qualquer sprite
  Shader.h              compila/linka shaders e envia uniforms
  Texture.h             carrega imagens (stb_image) e cria texturas OpenGL
  Texturas.h            todas as texturas do jogo
  Sprite.h              sprite animado a partir de spritesheet
  Layer.h               camada de fundo com parallax
  TextRenderer.h/.cpp   texto com FreeType (uma textura por caractere, VBO dinâmico)
  Hud.h/.cpp            telas de início e de game over, barra de itens, corações e contadores
  Config.h              leitor do arquivo de configuração
tests/                  testes sem janela, roteiro de entradas e rastro de execução
external/               GLAD (GL 3.3 core), GLM e stb_image
tools/gerar_assets.py   script que gerou a arte (opcional rodar)
```
