/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_H
#define GUI_H

namespace GUI{

}
#define GUI_VERSION "0.2.8"

#include "Components/Component.h"
#include "Components/Container.h"
#include "Components/Connector.h"
#include "Components/Window.h"
#include "Components/Button.h"
#include "Components/Checkbox.h"
#include "Components/Menu.h"
#include "Components/Panel.h"
#include "Components/EditorPanel.h"
#include "Components/Slider.h"
#include "Components/Splitter.h"
#include "Components/Tab.h"
#include "Components/Textfield.h"
#include "Components/TreeView.h"
#include "Components/Image.h"
#include "Components/Icon.h"
//
//#include "Fonts/GLFont.h"
//#include "Fonts/BitmapFont.h"

#include "GUI_Manager.h"

/**
 * @mainpage
 * 
 * The <a href="https://github.com/PADrend/GUI">GUI</a> library provides an API
 * to create a graphical user interface that is displayed using OpenGL. It is
 * written in C++11 and is licensed under the Mozilla Public License
 * version 2.0.
 * 
 * @section building Building your application with GUI
 * The easiest way to use GUI in your own application is using
 * <a href="http://www.cmake.org/">CMake</a> (at least version 2.8.11 is
 * required). CMake sets the include directories and linker options for your
 * build. If you want to build a target called @c MyApp, add the following to
 * your @c CMakeLists.txt to build against the required libraries:
 * @code
	find_package(Geometry 0.1.3 REQUIRED NO_MODULE)
	target_link_libraries(MyApp LINK_PRIVATE Geometry)

	find_package(Util 0.1.3 REQUIRED NO_MODULE)
	target_link_libraries(MyApp LINK_PRIVATE Util)

	find_package(GUI 0.1.2 REQUIRED NO_MODULE)
	target_link_libraries(MyApp LINK_PRIVATE GUI)
   @endcode
 * 
 * @section usage Using GUI in your code
 * A simple example application can be found in
 * TextfieldAndButtonMain.cpp . It shows how to create a
 * GUI::Window, then adds a GUI::Textfield and GUI::Button to it and listens
 * for a click onto the button to clear the text 
 * field.
 *
 * @section lib_dependencies Library Dependencies
 * - <a href="https://github.com/PADrend/Geometry">Geometry</a>
 * - <a href="https://github.com/PADrend/Util">Util</a>
 *
 * @section third_party_deps Third party dependencies
 * - <a href="http://www.opengl.org/">OpenGL</a>
 * - <a href="http://glew.sourceforge.net/">GLEW</a>
 * 
 * See also the
 * <a href="https://github.com/PADrend/ThirdParty">third party repository</a>
 * that can be used to build external dependencies.
 */

#endif // GUI_H
