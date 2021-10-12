cmake_minimum_required(VERSION 3.11.3)
project(ChatbotTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

enable_testing()

add_executable(test_ownership_characterization
    test_ownership_characterization.cpp
    levenshtein.cpp
)
add_test(NAME ownership_characterization COMMAND test_ownership_characterization)

add_executable(test_ownership_edge_cases
    test_ownership_edge_cases.cpp
    levenshtein.cpp
)
add_test(NAME ownership_edge_cases COMMAND test_ownership_edge_cases)
