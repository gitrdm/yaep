# YAEP (Yet Another Earley Parser)
#
# Copyright (c) 1997-2018  Vladimir Makarov <vmakarov@gcc.gnu.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.15)

project(yaep LANGUAGES C CXX)
enable_testing()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to enable strict warnings for C/C++ builds. This helps find
# portability and correctness issues under C17/C++17.  Enabled by
# default so the CI/devs see and can fix warnings; toggle off if you
# need a noiseless build temporarily.
option(YAEP_STRICT_WARNINGS "Enable strict compiler warnings (GCC/Clang)" ON)

if (YAEP_STRICT_WARNINGS)
	if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		set(_yaep_c_warnings
			-Wall
			-Wextra
			-Wpedantic
			-Wconversion
			-Wsign-conversion
			-Wshadow
			-Wcast-qual
			-Wcast-align
			-Wformat=2
			-Wmissing-prototypes
			-Wmissing-declarations
			-Wstrict-prototypes
		)
		string (REPLACE ";" " " _yaep_c_warnings_str "${_yaep_c_warnings}")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_yaep_c_warnings_str}")
	endif()

	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		set(_yaep_cxx_warnings
			-Wall
			-Wextra
			-Wpedantic
			-Wconversion
			-Wsign-conversion
			-Wshadow
			-Wold-style-cast
			-Wcast-qual
			-Wcast-align
			-Wformat=2
			-Wmissing-declarations
		)
		string (REPLACE ";" " " _yaep_cxx_warnings_str "${_yaep_cxx_warnings}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_yaep_cxx_warnings_str}")
	endif()
endif()

include( GNUInstallDirs )

add_subdirectory( src )
add_subdirectory( test )
add_subdirectory( doc )
