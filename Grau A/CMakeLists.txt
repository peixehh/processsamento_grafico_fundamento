cmake_minimum_required(VERSION 3.15)

if(NOT CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  endif()
endif()

project(CavernasGrauA LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# GLAD (carregador do OpenGL 3.3 core) e compilado como biblioteca
add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

# GLFW e FreeType vem do vcpkg (vcpkg install glfw3 freetype)
find_package(glfw3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)

# Codigo do jogo uma classe por arquivo, listados um a um
set(FONTES_JOGO
  src/Body.cpp
  src/Camera.cpp
  src/DayNight.cpp
  src/Drop.cpp
  src/Enemy.cpp
  src/Game.cpp
  src/GameRender.cpp
  src/GameUpdate.cpp
  src/Hud.cpp
  src/Inventory.cpp
  src/Player.cpp
  src/TextRenderer.cpp
  src/World.cpp)

add_executable(jogo src/main.cpp ${FONTES_JOGO})

target_include_directories(jogo SYSTEM PRIVATE external)
target_include_directories(jogo PRIVATE src)
target_link_libraries(jogo PRIVATE glad glfw Freetype::Freetype)
if(UNIX)
  target_link_libraries(jogo PRIVATE m)
endif()

target_compile_definitions(jogo PRIVATE GAME_ROOT="${CMAKE_SOURCE_DIR}")

if(MSVC)
  target_compile_options(jogo PRIVATE /utf-8 /W3)
else()
  target_compile_options(jogo PRIVATE -Wall -Wextra)
endif()

option(BUILD_TESTS "Compila os testes de tests/" OFF)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
