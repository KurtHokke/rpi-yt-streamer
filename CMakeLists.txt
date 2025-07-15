cmake_minimum_required(VERSION 3.10)

project(server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TEST "test" OFF)



if(NOT TEST)
    set(EXE server)
    add_executable(${EXE} 
                    main.c)
else()
    set(EXE test)
    add_executable(${EXE} 
                    main.c)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "Installation prefix" FORCE)
endif()
message(STATUS "install prefix: ${CMAKE_INSTALL_PREFIX}\n")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env bash "${CMAKE_SOURCE_DIR}/setup.sh" "${CMAKE_INSTALL_PREFIX}/bin/server_venv"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE script_result
    OUTPUT_VARIABLE script_output
    ERROR_VARIABLE script_error
)

if(NOT "${script_result}" STREQUAL "0")
    message(FATAL_ERROR "Script failed: ${script_error}")
else()
    message(STATUS "Script output: ${script_output}")
endif()

if(TEST)
    return()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(libvlc REQUIRED IMPORTED_TARGET libvlc>=3.0.21)
pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson>=2.14)

target_compile_definitions(${EXE} PRIVATE "INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
target_link_libraries(${EXE} PRIVATE PkgConfig::libvlc PkgConfig::jansson)

install(TARGETS ${EXE} DESTINATION bin)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")





#find_package(PkgConfig REQUIRED)
#pkg_check_modules(libvlc REQUIRED IMPORTED_TARGET libvlc>=3.0.21)
#pkg_check_modules(json-c REQUIRED IMPORTED_TARGET json-c>=0.18)

#pkg_check_modules(libwebsockets REQUIRED IMPORTED_TARGET libwebsockets>=4)
#find_library(RPCRT4_LIB Rpcrt4)
#if(NOT RPCRT4_LIB)
#    message(FATAL_ERROR "Rpcrt4 library not found. Ensure MinGW-w64 is installed.")
#endif()


#target_link_libraries(${EXE} PRIVATE PkgConfig::libcurl PkgConfig::json-c)


# Optional: Specify output directory for the executable
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)