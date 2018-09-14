cmake_minimum_required (VERSION 2.8.11)
project (dicemaster11)

add_executable(${CMAKE_PROJECT_NAME}/CMakeFiles/*.wasm ${CMAKE_CURRENT_SOURCE_DIR}/src/${CMAKE_PROJECT_NAME}.cpp)
target_include_directories(${CMAKE_PROJECT_NAME}.wasm
   PUBLIC 
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(${CMAKE_PROJECT_NAME}.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(${CMAKE_PROJECT_NAME}.abi ALL)

set(ABI_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.abi")

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}.abi
    COMMAND eosiocpp -g  ${ABI_FILE}   "${CMAKE_CURRENT_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}.hpp"
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}.abi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${ABI_FILE} "${CMAKE_CURRENT_SOURCE_DIR}/abi/${CMAKE_PROJECT_NAME}.abi"
)