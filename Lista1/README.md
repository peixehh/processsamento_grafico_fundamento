# Lista 1 — Primitivas Gráficas, Shaders e Buffers

## Equipe
- João Pedro Araújo Rocha

---

## Descrição do Projeto

> Este programa foi desenvolvido como parte da disciplina *Processamento Gráfico: Fundamentos* com o objetivo de aplicar conceitos iniciais de OpenGL moderna (Core Profile) e C++ na criação de primitivas gráficas 2D. O projeto explora a utilização de shaders programáveis, buffers (VAO/VBO), a equação paramétrica da circunferência para gerar geometria e o desenho de múltiplos objetos a partir de diferentes tipos de primitivas.

Cada item da lista foi implementado como um **programa independente** — um
arquivo `.cpp` com seu próprio `main()`, que já gera diretamente o
resultado pedido, sem menu nem tecla para trocar de item.

---

## Estrutura do Projeto

| Arquivo                  | Descrição                                                                 |
|--------------------------|---------------------------------------------------------------------------|
| `src/exercicio1_a.cpp`    | Exercício 1, letra a) — dois triângulos, polígono preenchido.             |
| `src/exercicio1_b.cpp`    | Exercício 1, letra b) — dois triângulos, apenas contorno.                 |
| `src/exercicio1_c.cpp`    | Exercício 1, letra c) — dois triângulos, apenas pontos.                   |
| `src/exercicio1_d.cpp`    | Exercício 1, letra d) — dois triângulos, preenchido + contorno + pontos.  |
| `src/exercicio2_a.cpp`    | Exercício 2, letra a) — octógono (equação paramétrica do círculo).        |
| `src/exercicio2_b.cpp`    | Exercício 2, letra b) — pentágono.                                        |
| `src/exercicio2_c.cpp`    | Exercício 2, letra c) — pac-man.                                          |
| `src/exercicio2_d.cpp`    | Exercício 2, letra d) — fatia de pizza.                                   |
| `src/exercicio2_e.cpp`    | Exercício 2, letra e) DESAFIO 1 — estrela.                                |
| `src/exercicio2_f.cpp`    | Exercício 2, letra f) DESAFIO 2 — espiral.                                |
| `src/exercicio3.cpp`      | Exercício 3 — triângulo com cor interpolada por vértice (VBO/VAO).        |
| `src/exercicio4.cpp`      | Exercício 4 — desenho no papel quadriculado com múltiplos VAOs (exemplo: casinha). |
| `src/GLBase.h`             | Classe/funções utilitárias para criação da janela GLFW/contexto OpenGL/GLAD e criação de VAO/VBO. |
| `src/Shader.h`             | Classe utilitária para compilação e uso de shaders.                       |

---

## Informações Técnicas

- **Linguagem:** C++ (C++17)
- **API Gráfica:** OpenGL 3.3+ (core profile)
- **Dependências:** GLFW, GLAD (outras, se utilizadas)
- **IDE/Compilador:** Visual Studio Code / CMake (MSVC, Clang ou GCC)
- **Plataforma-alvo:** Windows / Linux / MacOS

---

## Checklist de Requisitos

- [x] Criação de janela e contexto OpenGL
- [x] Configuração de shaders e pipeline programável
- [x] Desenho de primitivas com diferentes modos (preenchido, contorno, pontos)
- [x] Geração de geometria pela equação paramétrica do círculo
- [x] Configuração de VBO/VAO com múltiplos atributos por vértice (posição + cor)
- [x] Renderização de múltiplos objetos com múltiplos VAOs e chamadas de desenho

---

## Referências e/ou créditos

- Material e slides da disciplina *Processamento Gráfico: Fundamentos* (Unisinos)
- [LearnOpenGL](https://learnopengl.com/) — referência geral de OpenGL moderna
- [Documentação oficial do GLFW](https://www.glfw.org/documentation.html)
- [GLAD (gerador do loader)](https://gen.glad.sh/)

---

