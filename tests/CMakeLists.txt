add_executable(test_assembler test_assembler.c)
target_link_libraries(test_assembler PRIVATE waicore)
target_compile_options(test_assembler PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME assembler COMMAND test_assembler)

add_executable(test_vm test_vm.c)
target_link_libraries(test_vm PRIVATE waicore)
target_compile_options(test_vm PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME vm COMMAND test_vm)

add_executable(test_bytecode test_bytecode.c)
target_link_libraries(test_bytecode PRIVATE waicore)
target_compile_options(test_bytecode PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME bytecode COMMAND test_bytecode)

add_executable(test_disassembler test_disassembler.c)
target_link_libraries(test_disassembler PRIVATE waicore)
target_compile_options(test_disassembler PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME disassembler COMMAND test_disassembler)

add_executable(test_debugger test_debugger.c)
target_link_libraries(test_debugger PRIVATE waicore)
target_compile_options(test_debugger PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME debugger COMMAND test_debugger)


add_executable(test_verifier test_verifier.c)
target_link_libraries(test_verifier PRIVATE waicore)
target_compile_options(test_verifier PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME verifier COMMAND test_verifier)

add_executable(test_trace test_trace.c)
target_link_libraries(test_trace PRIVATE waicore)
target_compile_options(test_trace PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME trace COMMAND test_trace)

add_executable(test_malformed_bytecode test_malformed_bytecode.c)
target_link_libraries(test_malformed_bytecode PRIVATE waicore)
target_compile_options(test_malformed_bytecode PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)
add_test(NAME malformed_bytecode COMMAND test_malformed_bytecode)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(
    NAME golden_examples
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/golden/run_golden.py $<TARGET_FILE:waivm> ${CMAKE_SOURCE_DIR}
)
