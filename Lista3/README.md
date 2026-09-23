# Lista 3 — Criando Triângulos a partir do Clique do Mouse

## Equipe
- João Pedro Araújo Rocha

---

## Descrição do Projeto

> Este programa foi desenvolvido como parte da disciplina *Processamento Gráfico: Fundamentos* com o objetivo de revisar a criação de buffers de geometria (VAO/VBO) e o envio de informações para o programa de shader, a partir da criação interativa de triângulos com o clique do mouse.

### Como funciona

- Cada clique com o botão esquerdo do mouse na janela cria **1 vértice** (mostrado como um ponto branco).
- A **cada 3 vértices** criados, um **triângulo** é fechado e desenhado.
- **Cada novo triângulo usa uma cor nova**, tirada de uma paleta fixa, voltando ao início da paleta se todas as cores já tiverem sido usadas.
- Cada triângulo é desenhado com seu **próprio VAO/VBO**.

---

## Estrutura do Projeto

| Arquivo               | Descrição                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| `src/exercicio1.cpp`   | Programa principal — criação de triângulos por clique do mouse.             |
| `src/GLBase.h`          | Infraestrutura comum (janela GLFW/contexto OpenGL/GLAD, criação de VAO/VBO). |
| `src/Shader.h`           | Classe utilitária para compilação/uso de shaders e envio de matrizes. |
| `external/glad/`         | Loader OpenGL 3.3 core já gerado.                                            |
| `external/glm/`          | Biblioteca de matemática (header-only) usada para a projeção ortográfica.   |

---

## Informações Técnicas

- **Linguagem:** C++ (C++17)
- **API Gráfica:** OpenGL 3.3+ (core profile)
- **Dependências:** GLFW, GLAD, GLM
- **IDE/Compilador:** Visual Studio Code / CMake (MSVC, Clang ou GCC)
- **Plataforma-alvo:** Windows / Linux / macOS

---

## Checklist de Requisitos

- [x] Ao clicar na tela, cria-se apenas 1 vértice
- [x] A cada 3 vértices criados, um triângulo é criado
- [x] Cada novo triângulo criado usa uma cor nova
- [x] Buffers de geometria (VAO/VBO) criados dinamicamente a cada triângulo
- [x] Envio de informações (posição dos vértices, cor) para o programa de shader via atributos e uniforms

---

## Referências e/ou créditos

- Material e slides da disciplina *Processamento Gráfico: Fundamentos* (Unisinos)
- [GLFW Input Guide](https://www.glfw.org/docs/latest/input_guide.html)
- [Documentação oficial do GLFW](https://www.glfw.org/documentation.html)
- [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)
- [GLAD (gerador do loader)](https://gen.glad.sh/)
