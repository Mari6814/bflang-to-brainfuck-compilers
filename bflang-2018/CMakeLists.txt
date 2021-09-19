cmake_minimum_required(VERSION 3.5)
project(bflang CXX)

set(CMAKE_CXX_STANDARD 14)

set(DEPENDENCY_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/bf.h ${CMAKE_CURRENT_SOURCE_DIR}/print.h)

set(BISON_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bf.ypp)
set(BISON_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bf.tab.cpp)

set(LEX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bf.lpp)
set(LEX_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp)

set(SOURCE_FILES main.cpp bf.cpp)
set(OBJECT_FILES ${CMAKE_CURRENT_BINARY_DIR}/bf.tab.o ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.o)

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_executable(bfc ${SOURCE_FILES} ${OBJECT_FILES})
target_link_libraries(bfc ${Boost_LIBRARIES})

add_custom_command(
        OUTPUT ${BISON_OUTPUT}
        DEPENDS ${BISON_SOURCE_FILES} ${DEPENDENCY_HEADERS}
        COMMAND bison -y -d ${BISON_SOURCE_FILES}
        COMMAND mv y.tab.c ${BISON_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
        OUTPUT ${LEX_OUTPUT}
        DEPENDS ${LEX_SOURCE_FILES} ${DEPENDENCY_HEADERS}
        COMMAND flex ${LEX_SOURCE_FILES}
        COMMAND mv lex.yy.c ${LEX_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
        OUTPUT ${OBJECT_FILES}
        DEPENDS ${LEX_OUTPUT} ${BISON_OUTPUT}
        COMMAND rm -f ${OBJECT_FILES}
        COMMAND c++ -c ${LEX_OUTPUT} ${BISON_OUTPUT} -Wno-write-strings -std=c++14 -I${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


set(INTERPRETER_SOURCE interpreter.cpp interpreter.h)
add_executable(bfi ${INTERPRETER_SOURCE})
target_link_libraries(bfi ${Boost_LIBRARIES})