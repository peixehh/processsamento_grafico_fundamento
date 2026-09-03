cmake_minimum_required(VERSION 3.15)
project(ListaExercicios1_PG LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 3.3 REQUIRED)

add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

function(add_exercicio name source)
    add_executable(${name} ${source})
    target_include_directories(${name} PRIVATE src)
    target_link_libraries(${name} PRIVATE glad glfw)

    if(UNIX AND NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(${name} PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
    endif()

    if(APPLE)
        target_link_libraries(${name} PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    endif()

    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()

add_exercicio(exercicio1_a src/exercicio1_a.cpp)
add_exercicio(exercicio1_b src/exercicio1_b.cpp)
add_exercicio(exercicio1_c src/exercicio1_c.cpp)
add_exercicio(exercicio1_d src/exercicio1_d.cpp)

add_exercicio(exercicio2_a src/exercicio2_a.cpp)
add_exercicio(exercicio2_b src/exercicio2_b.cpp)
add_exercicio(exercicio2_c src/exercicio2_c.cpp)
add_exercicio(exercicio2_d src/exercicio2_d.cpp)
add_exercicio(exercicio2_e src/exercicio2_e.cpp)
add_exercicio(exercicio2_f src/exercicio2_f.cpp)

add_exercicio(exercicio3 src/exercicio3.cpp)
add_exercicio(exercicio4 src/exercicio4.cpp)
