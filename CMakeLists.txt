cmake_minimum_required(VERSION 3.16)
project(SnakeOverdrive)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск заголовочных файлов и библиотек SFML внутри папки проекта
find_path(SFML_INCLUDE_DIR "SFML/Graphics.hpp"
    PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/SFML/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/SFML-2.6.2/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/SFML-2.6.1/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_path(SFML_LIB_DIR "sfml-graphics-s-d.lib" "sfml-graphics-s.lib" "sfml-graphics.lib"
    PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/SFML/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/SFML-2.6.2/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/SFML-2.6.1/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

if(NOT SFML_INCLUDE_DIR OR NOT SFML_LIB_DIR)
    message(FATAL_ERROR 
        "\nНе найдены заголовочные файлы или библиотеки SFML!\n"
        "SFML_INCLUDE_DIR: ${SFML_INCLUDE_DIR}\n"
        "SFML_LIB_DIR: ${SFML_LIB_DIR}\n"
        "Убедитесь, что внутри папки SFML находятся подпапки 'include' и 'lib'."
    )
endif()

message(STATUS "SFML Headers: ${SFML_INCLUDE_DIR}")
message(STATUS "SFML Libs: ${SFML_LIB_DIR}")

include_directories(${SFML_INCLUDE_DIR})
link_directories(${SFML_LIB_DIR})

# Статическая линковка и зависимости SFML для Windows
add_definitions(-DSFML_STATIC)

set(SFML_STATIC_DEPS
    sfml-graphics-s-d
    sfml-window-s-d
    sfml-system-s-d
    opengl32
    freetype
    winmm
    gdi32
)

# Сборка исполняемых файлов
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main_full_game.cpp")
    add_executable(snake_full_game main_full_game.cpp)
    target_link_libraries(snake_full_game PRIVATE ${SFML_STATIC_DEPS})
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/snake_ui_member1.cpp")
    add_executable(snake_ui snake_ui_member1.cpp)
    target_link_libraries(snake_ui PRIVATE ${SFML_STATIC_DEPS})
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/snake_player_member2.cpp")
    add_executable(snake_player snake_player_member2.cpp)
    target_link_libraries(snake_player PRIVATE ${SFML_STATIC_DEPS})
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/snake_world_member3.cpp")
    add_executable(snake_world snake_world_member3.cpp)
    target_link_libraries(snake_world PRIVATE ${SFML_STATIC_DEPS})
endif()