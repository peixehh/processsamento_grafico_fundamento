cmake_minimum_required(VERSION 3.15)
project(ListaExercicios2_PG LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 3.3 REQUIRED)

add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external)

function(add_exercicio name source)
    add_executable(${name} ${source})
    target_include_directories(${name} PRIVATE src)
    target_link_libraries(${name} PRIVATE glad glfw glm)

    if(UNIX AND NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(${name} PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
    endif()

    if(APPLE)
        target_link_libraries(${name} PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    endif()

    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()

add_exercicio(exercicio1 src/exercicio1.cpp)
add_exercicio(exercicio2 src/exercicio2.cpp)
add_exercicio(exercicio3 src/exercicio3.cpp)
add_exercicio(exercicio4 src/exercicio4.cpp)
add_exercicio(exercicio5 src/exercicio5.cpp)
add_exercicio(exercicio6 src/exercicio6.cpp)
