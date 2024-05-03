cmake_minimum_required(VERSION 3.16)
project(Calculator)

set(CMAKE_CXX_STANDARD 20)

add_executable(calculator_final calculator.cpp)

enable_testing()

add_executable(test_calculator calculator.cpp)
target_compile_definitions(test_calculator PRIVATE TEST_MODE) 
add_test(NAME CalculatorTest COMMAND test_calculator)
