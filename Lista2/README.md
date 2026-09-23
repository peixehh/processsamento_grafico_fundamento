 # Lista 2 — Matriz de Projeção Ortográfica e Mapeamento com a Viewport

## Equipe
- João Pedro Araújo Rocha

---

## Descrição do Projeto

> Este programa foi desenvolvido como parte da disciplina *Processamento Gráfico: Fundamentos* com o objetivo de aplicar os conceitos de janela do mundo (window), matriz de projeção ortográfica e mapeamento com a viewport em OpenGL moderna (Core Profile) e C++. O projeto explora diferentes configurações da janela do mundo (incluindo coordenadas de pixel com eixo Y invertido), a restrição/repetição do desenho em quadrantes da tela via viewport, e a criação interativa de geometria a partir de cliques do mouse.

Cada item da lista foi implementado como um **programa independente** — um
arquivo `.cpp` com seu próprio `main()`, que já gera diretamente o
resultado pedido.

---

## Estrutura do Projeto

| Arquivo                | Descrição                                                                          |
|-------------------------|--------------------------------------------------------------------------------------|
| `src/exercicio1.cpp`    | Exercício 1 — janela do mundo com limites `xmin=-10, xmax=10, ymin=-10, ymax=10`.    |
| `src/exercicio2.cpp`    | Exercício 2 — janela do mundo `xmin=0, xmax=800, ymin=600, ymax=0` (coordenadas de pixel, eixo Y invertido). |
| `src/exercicio3.cpp`    | Exercício 3 — 5 quadrados em posições diferentes da tela, demonstrando a correspondência pixel ↔ posição |
| `src/exercicio4.cpp`    | Exercício 4 — viewport modificado para desenhar a cena apenas no quadrante superior direito da janela. |
| `src/exercicio5.cpp`    | Exercício 5 — a mesma cena desenhada nos 4 quadrantes da janela.                      |
| `src/exercicio6.cpp`    | Exercício 6 — triângulos criados a partir de cliques do mouse (3 vértices = 1 triângulo, cor nova a cada um). |
| `src/GLBase.h`           | Classe/funções utilitárias para criação da janela GLFW/contexto OpenGL/GLAD e criação de VAO/VBO. |
| `src/Shader.h`            | Classe utilitária para compilação e uso de shaders, incluindo envio de matrizes via uniform. |

---

## Informações Técnicas

- **Linguagem:** C++ (C++17)
- **API Gráfica:** OpenGL 3.3+ (core profile)
- **Dependências:** GLFW, GLAD, GLM
- **IDE/Compilador:** Visual Studio Code / CMake (MSVC, Clang ou GCC)
- **Plataforma-alvo:** Windows / Linux / MacOS

---

## Checklist de Requisitos

- [x] Modificação da janela do mundo (matriz de projeção ortográfica) para limites arbitrários
- [x] Janela do mundo em coordenadas de pixel, com eixo Y invertido
- [x] Posicionamento de objetos por coordenada de tela (pixel) e resposta escrita sobre a utilidade dessa configuração
- [x] Modificação do viewport para restringir o desenho a um quadrante da janela
- [x] Repetição do desenho da mesma cena nos 4 quadrantes da janela
- [x] Criação interativa de triângulos a partir de cliques do mouse, com cor nova a cada triângulo

## Referências e/ou créditos

- Material e slides da disciplina *Processamento Gráfico: Fundamentos* (Unisinos)
- [LearnOpenGL — Coordinate Systems](https://learnopengl.com/Getting-started/Coordinate-Systems)
- [LearnOpenGL — Transformations](https://learnopengl.com/Getting-started/Transformations)
- [LearnOpenGL — In Practice: Rendering Sprites](https://learnopengl.com/In-Practice/2D-Game/Rendering-Sprites)
- [Documentação oficial do GLFW](https://www.glfw.org/documentation.html)
- [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)
- [GLAD (gerador do loader)](https://gen.glad.sh/)

---
