# YAEP (Yet Another Earley Parser) - Python Wrapper Tests
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

# Find Python3 interpreter
find_package(Python3 COMPONENTS Interpreter)

if(Python3_FOUND)
    # Check if pytest is available
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import pytest"
        RESULT_VARIABLE PYTEST_IMPORT_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(PYTEST_IMPORT_RESULT EQUAL 0)
        # Install Python package in development mode before running tests
        add_custom_target(yaep_python_install
            COMMAND ${Python3_EXECUTABLE} -m pip install -e ${CMAKE_CURRENT_SOURCE_DIR} --quiet
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Installing yaep_python package in development mode"
            DEPENDS yaep_shared
        )
        
        # Add test that runs only the passing tests
        # All 11 tests now pass after fixing the double-free bug in yaep.c pl_fin()
        add_test(
            NAME yaep-python-passing
            COMMAND ${Python3_EXECUTABLE} -m pytest tests/ -v
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        set_tests_properties(yaep-python-passing PROPERTIES
            ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/src:$ENV{PYTHONPATH};LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}/src:$ENV{LD_LIBRARY_PATH}"
            LABELS "python"
        )
        
        # The yaep-python-all test is kept for backwards compatibility
        # but now runs the same tests as yaep-python-passing
        add_test(
            NAME yaep-python-all
            COMMAND ${Python3_EXECUTABLE} -m pytest tests/ -v
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        set_tests_properties(yaep-python-all PROPERTIES
            ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/src:$ENV{PYTHONPATH};LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}/src:$ENV{LD_LIBRARY_PATH}"
            LABELS "python;optional"
        )
        
        message(STATUS "Python tests enabled (pytest found)")
    else()
        message(WARNING "pytest not found - Python tests will be skipped. Install with: pip install pytest")
    endif()
else()
    message(WARNING "Python3 not found - Python tests will be skipped")
endif()
