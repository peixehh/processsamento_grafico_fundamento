# Lista 3 — Criando Triângulos a partir do Clique do Mouse

## Equipe
- João Pedro Araújo Rocha

---

## Descrição do Projeto

> Este programa foi desenvolvido como parte da disciplina *Processamento Gráfico: Fundamentos* com o objetivo de revisar a criação de buffers de geometria (VAO/VBO) e o envio de informações para o programa de shader, a partir da criação interativa de triângulos com o clique do mouse.

O programa é um **único executável independente** (`exercicio1`) que já nasce
pronto para uso: sem menu, sem tecla para trocar de modo.

### Como funciona

- Cada clique com o botão esquerdo do mouse na janela cria **1 vértice** (mostrado como um ponto branco).
- A **cada 3 vértices** criados, um **triângulo** é fechado e desenhado.
- **Cada novo triângulo usa uma cor nova**, tirada de uma paleta fixa (`PALETTE` em `exercicio1.cpp`), voltando ao início da paleta se todas as cores já tiverem sido usadas.
- Cada triângulo é desenhado com seu **próprio VAO/VBO** (uma das duas opções sugeridas pelo enunciado — aqui optei por VAOs separados, um por triângulo).

`ESC` fecha a janela.

---

## Estrutura do Projeto

| Arquivo               | Descrição                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| `src/exercicio1.cpp`   | Programa principal — criação de triângulos por clique do mouse.             |
| `src/GLBase.h`          | Infraestrutura comum (janela GLFW/contexto OpenGL/GLAD, criação de VAO/VBO). |
| `src/Shader.h`           | Classe utilitária para compilação/uso de shaders e envio de matrizes (`glm::mat4`). |
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

## Como rodar no VS Code (passo a passo)

Se você já configurou o ambiente para as Listas 1/2 (Git, CMake, VS Code + extensões C/C++ e CMake Tools, GLFW via vcpkg), pode pular direto para o passo 3 — é o mesmo ambiente.

### 1. Instale as ferramentas de base (uma vez só)

**Windows**
1. Instale o [Visual Studio Build Tools](https://visualstudio.microsoft.com/pt-br/visual-cpp-build-tools/) (marque "Desenvolvimento para desktop com C++") **ou** MSYS2/MinGW-w64.
2. Instale o [CMake](https://cmake.org/download/) (marque "Add CMake to PATH").
3. Instale o [Git](https://git-scm.com/downloads).
4. Instale o [vcpkg](https://github.com/microsoft/vcpkg) e o GLFW:
   ```powershell
   git clone https://github.com/microsoft/vcpkg
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg install glfw3:x64-windows
   ```

**macOS**
```bash
xcode-select --install
brew install cmake glfw
```

**Linux (Ubuntu/Debian)**
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev \
                 libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev gdb
```

### 2. Instale o VS Code e as extensões

1. Baixe o [VS Code](https://code.visualstudio.com/).
2. Vá em **Extensions** (`Ctrl+Shift+X`) e instale **C/C++** (ms-vscode.cpptools) e **CMake Tools** (ms-vscode.cmake-tools).

### 3. Abra a pasta do projeto

**File → Open Folder…** e selecione **esta pasta** (`lista3-processamento-grafico` ou a pasta `Lista3` do repositório — não a pasta raiz que contém `Lista1`/`Lista2`/`Lista3` juntas, senão o CMake não encontra o `CMakeLists.txt`).

**No Windows com vcpkg**, crie `.vscode/settings.json` apontando o toolchain (o `.vscode/` não é versionado no repositório, é config local de máquina):
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    ]
}
```

### 4. Rode

**Painel do CMake Tools:** clique no ícone do CMake na barra lateral, escolha o kit do compilador quando pedido (no Windows com vcpkg, o "Visual Studio Build Tools 2022 - amd64"), e clique no ▶ Run sobre o alvo `exercicio1`.

**Terminal (qualquer SO):**
```bash
cmake -S . -B build
cmake --build build
./build/exercicio1
```

---

## Referências e/ou créditos

- Material e slides da disciplina *Processamento Gráfico: Fundamentos* (Unisinos)
- [GLFW Input Guide](https://www.glfw.org/docs/latest/input_guide.html)
- [Documentação oficial do GLFW](https://www.glfw.org/documentation.html)
- [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)
- [GLAD (gerador do loader)](https://gen.glad.sh/)
