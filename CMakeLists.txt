cmake_minimum_required(VERSION 3.10)

project(server C)

include(${CMAKE_SOURCE_DIR}/cmake/config.cmake)


set(EXE server)
add_executable(${EXE} src/main.c
                        src/ytdlp.c)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "Installation prefix" FORCE)
endif()
message(STATUS "${_clrBoldGreen}install prefix${_clrReset}: ${CMAKE_INSTALL_PREFIX}\n")

message(STATUS "${_clr_Yellow}Executing setup.sh...${_clrReset}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env bash "${CMAKE_SOURCE_DIR}/setup.sh" 
                                            "${_USE_PYPY}"
                                            "${CMAKE_INSTALL_PREFIX}" 
                                            "${CMAKE_SOURCE_DIR}/3rd"
    #COMMAND_ECHO STDOUT
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE script_result
    #OUTPUT_VARIABLE script_output
    ERROR_VARIABLE script_error
)
if(NOT "${script_result}" STREQUAL "0")
    message(FATAL_ERROR "Script errors:\n${script_error}")
else()
    message(STATUS "Script ran successfully")
endif()

if(TEST)
    message(FATAL_ERROR "test")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Embed)
message(STATUS "Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
message(STATUS "Python3_VERSION: ${Python3_VERSION}")
message(STATUS "Python3_VERSION_MAJOR: ${Python3_VERSION_MAJOR}")
message(STATUS "Python3_VERSION_MINOR: ${Python3_VERSION_MINOR}")
message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(libvlc REQUIRED IMPORTED_TARGET libvlc>=3.0.21)
pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson>=2.14)


target_compile_definitions(${EXE} PRIVATE "INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
target_compile_definitions(${EXE} PRIVATE
    "VENV_SITE_PACKAGES=\"${CMAKE_INSTALL_PREFIX}/bin/_venv/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages\""
)

target_include_directories(${EXE} PRIVATE ${Python3_INCLUDE_DIRS})

target_link_libraries(${EXE} PRIVATE PkgConfig::libvlc PkgConfig::jansson ${Python3_LIBRARIES})

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