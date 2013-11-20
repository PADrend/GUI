#
# This file is part of the GUI library.
# Copyright (C) 2013 Benjamin Eikel <benjamin@eikel.org>
#
# This library is subject to the terms of the Mozilla Public License, v. 2.0.
# You should have received a copy of the MPL along with this library; see the 
# file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
#
cmake_minimum_required(VERSION 2.8.11)

add_executable(TextfieldAndButton
	TextfieldAndButtonMain.cpp
)

target_link_libraries(TextfieldAndButton LINK_PRIVATE GUI)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set_property(TARGET TextfieldAndButton APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++11 ")
elseif(COMPILER_SUPPORTS_CXX0X)
	set_property(TARGET TextfieldAndButton APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++0x ")
else()
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
