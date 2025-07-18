
option(TEST_INLINE "test inline" OFF)
option(TEST "test" OFF)
option(USE_COLOR "print with colors" ON)
option(USE_COLORS "alias" ON)
if(NOT USE_COLORS)
    set(USE_COLOR OFF)
endif()
include(${CMAKE_SOURCE_DIR}/cmake/color.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

if(TEST_INLINE)
    message(STATUS "${_clr_Yellow} Running with -DTEST_INLINE=ON${_clrReset}")
    set(CMAKE_C_FLAGS "-Winline -O2 -pthread -Wall -Wextra -Wpedantic")
else()
    set(CMAKE_C_FLAGS "-g -Og -pthread -Wall -Wextra -Wpedantic")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)



option(USE_PYPY "use pypy instead of cpython" OFF)
if(USE_PYPY)
    set(_USE_PYPY "true")
    message(STATUS "Using ${_clr_Orange}pypy${_clrReset}")
else()
    set(_USE_PYPY "false")
    message(STATUS "Using ${_clr_Orange}python${_clrReset}")
endif()


