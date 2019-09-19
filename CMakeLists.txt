#
# This file is part of the GUI library.
# Copyright (C) 2009-2013 Benjamin Eikel <benjamin@eikel.org>
# Copyright (C) 2015-2019 Sascha Brandt <sascha@brandt.graphics>
#
# This library is subject to the terms of the Mozilla Public License, v. 2.0.
# You should have received a copy of the MPL along with this library; see the 
# file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
#
cmake_minimum_required(VERSION 3.1.0)
project(GUI VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set up install directories
include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKECONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/GUI)

add_library(GUI SHARED
	Base/BasicColors.cpp
	Base/Draw.cpp
	Base/Fonts/BitmapFont.cpp
	Base/ImageData.cpp
	Base/Layouters/ExtLayouter.cpp
	Base/Layouters/FlowLayouter.cpp
	Base/Properties.cpp
	Base/StyleManager.cpp
	Components/Button.cpp
	Components/Checkbox.cpp
	Components/Component.cpp
	Components/ComponentHoverPropertyFeature.cpp
	Components/ComponentTooltipFeature.cpp
	Components/Container.cpp
	Components/Connector.cpp
	Components/EditorPanel.cpp
	Components/Icon.cpp
	Components/Image.cpp
	Components/Label.cpp
	Components/ListView.cpp
	Components/Menu.cpp
	Components/Panel.cpp
	Components/Scrollbar.cpp
	Components/ScrollableContainer.cpp
	Components/Splitter.cpp
	Components/Slider.cpp
	Components/Tab.cpp
	Components/Textarea.cpp
	Components/Textfield.cpp
	Components/TreeView.cpp
	Components/Window.cpp
	Style/Colors.cpp
	Style/EmbeddedFonts.cpp
	Style/StdShapes.cpp
	Style/Style.cpp
	GUI_Manager.cpp
)
add_subdirectory(examples)

# Dependency to Geometry
if(NOT TARGET Geometry)
	find_package(Geometry 0.3.0 REQUIRED NO_MODULE)
endif()
target_link_libraries(GUI LINK_PUBLIC Geometry)

# Dependency to Util
if(NOT TARGET Util)
	find_package(Util 0.3.0 REQUIRED NO_MODULE)
endif()
target_link_libraries(GUI LINK_PUBLIC Util)

option(GUI_BACKEND_RENDERING "Use the Rendering library for drawing instead of OpenGL (recommended for use with PADrend)" ON)

if(GUI_BACKEND_RENDERING)
	# Dependency to Rendering
	if(NOT TARGET Rendering)
		find_package(Rendering 0.3.0 REQUIRED NO_MODULE)
	endif()
	target_link_libraries(GUI LINK_PUBLIC Rendering)
else(GUI_BACKEND_RENDERING)		
	# Dependency to OpenGL
	find_package(OpenGL REQUIRED)
	if(IS_DIRECTORY ${OPENGL_INCLUDE_DIR})
		target_include_directories(GUI PRIVATE ${OPENGL_INCLUDE_DIR})
	endif()
	target_link_libraries(GUI LINK_PRIVATE ${OPENGL_LIBRARIES})

	# Dependency to GLEW
	find_package(GLEW REQUIRED)
	target_include_directories(GUI PRIVATE ${GLEW_INCLUDE_DIRS})
	target_link_libraries(GUI LINK_PRIVATE ${GLEW_LIBRARIES})
endif(GUI_BACKEND_RENDERING)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
target_include_directories(GUI PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

# Set version of library
set_target_properties(GUI PROPERTIES VERSION ${GUI_VERSION}
																		 SOVERSION ${GUI_VERSION_MAJOR}
																		 LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Install the header files
file(GLOB GUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Base/Fonts/*.h")
install(FILES ${GUI_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI/Base/Fonts COMPONENT headers)
file(GLOB GUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Base/Layouters/*.h")
install(FILES ${GUI_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI/Base/Layouters COMPONENT headers)
file(GLOB GUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Base/*.h")
install(FILES ${GUI_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI/Base COMPONENT headers)
file(GLOB GUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Components/*.h")
install(FILES ${GUI_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI/Components COMPONENT headers)
file(GLOB GUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Style/*.h")
install(FILES ${GUI_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI/Style COMPONENT headers)
file(GLOB GUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES ${GUI_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI COMPONENT headers)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/config.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GUI COMPONENT headers)

# Install the library
install(TARGETS GUI EXPORT LibraryExport
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtimelibraries
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtimelibraries
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT developmentlibraries
)

# Set include directories for users of this library
target_include_directories(GUI
													 INTERFACE
													 "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>"
													 "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

# Library export
install(EXPORT LibraryExport DESTINATION ${CMAKE_INSTALL_CMAKECONFIGDIR} FILE GUITargets.cmake COMPONENT developmentlibraries)

# Create the configuration files
include(CMakePackageConfigHelpers)
configure_package_config_file(GUIConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/GUIConfig.cmake
															INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKECONFIGDIR}
															PATH_VARS CMAKE_INSTALL_CMAKECONFIGDIR
															NO_SET_AND_CHECK_MACRO
															NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/GUIConfigVersion.cmake
																 VERSION ${GUI_VERSION}
																 COMPATIBILITY SameMajorVersion)

# Install the configuration files
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/GUIConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/GUIConfigVersion.cmake
		DESTINATION ${CMAKE_INSTALL_CMAKECONFIGDIR} COMPONENT developmentlibraries)

# Packaging
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "libgui${GUI_VERSION_MAJOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI library")
set(CPACK_PACKAGE_VENDOR "Benjamin Eikel; Claudius Jaehn; Ralf Petring; Sascha Brandt")
set(CPACK_PACKAGE_CONTACT "Benjamin Eikel <benjamin@eikel.org>")
set(CPACK_PACKAGE_VERSION_MAJOR ${GUI_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GUI_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GUI_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME "libGUI")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES ON)

include(CPack)
