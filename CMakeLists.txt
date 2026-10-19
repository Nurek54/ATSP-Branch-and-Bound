cmake_minimum_required(VERSION 3.20)
project(ATSP_BranchAndBound LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BNB_CORE
        src/core/TSPInstance.cpp
        src/core/TSPUtilities.cpp
        src/core/MemoryUsage.cpp
        src/algorithms/BranchAndBoundBFS.cpp
        src/algorithms/BranchAndBoundDFS.cpp
        src/algorithms/BranchAndBoundBestFirst.cpp
        src/io/SaveToCSV.cpp)

add_executable(atsp_bnb
        src/main.cpp
        src/app/ConfigReader.cpp
        src/app/TSPSimulation.cpp
        ${BNB_CORE})
target_include_directories(atsp_bnb PRIVATE src)
# Domyslny config i folder z macierzami, gdy w katalogu roboczym ich nie ma
target_compile_definitions(atsp_bnb PRIVATE
        ATSP_CONFIG_FILE="${CMAKE_SOURCE_DIR}/config/config.txt"
        ATSP_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

# Pomiary do README: zawsze z optymalizacja, niezaleznie od typu builda
add_executable(benchmark tools/benchmark.cpp ${BNB_CORE})
target_include_directories(benchmark PRIVATE src)
if (MSVC)
    target_compile_options(benchmark PRIVATE /O2)
else ()
    target_compile_options(benchmark PRIVATE -O2)
endif ()

if (WIN32)
    target_link_libraries(atsp_bnb PRIVATE psapi)
    target_link_libraries(benchmark PRIVATE psapi)
endif ()

# MinGW: libstdc++ i libgcc wbudowane w exe, a libwinpthread-1.dll kopiowany z kompilatora
# obok exe, zeby Windows nie wzial starszej wersji z innego MinGW w PATH.
# (Pelne -static nie dziala z MinGW dolaczonym do CLion: brak __intrinsic_setjmpex)
if (MINGW)
    get_filename_component(MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach (target atsp_bnb benchmark)
        target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
        if (EXISTS "${MINGW_BIN}/libwinpthread-1.dll")
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN}/libwinpthread-1.dll" $<TARGET_FILE_DIR:${target}>)
        endif ()
    endforeach ()
endif ()
