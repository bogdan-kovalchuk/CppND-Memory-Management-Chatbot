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

add_executable(test_exclusive_handle
    test_exclusive_handle.cpp
)
target_include_directories(test_exclusive_handle PRIVATE ${CMAKE_SOURCE_DIR}/../src)
add_test(NAME exclusive_handle COMMAND test_exclusive_handle)

add_executable(test_chatbot_integration
    test_chatbot_integration.cpp
)
target_include_directories(test_chatbot_integration PRIVATE ${CMAKE_SOURCE_DIR}/../src)
add_test(NAME chatbot_integration COMMAND test_chatbot_integration)

add_executable(test_graphnode_move
    test_graphnode_move.cpp
)
add_test(NAME graphnode_move COMMAND test_graphnode_move)
