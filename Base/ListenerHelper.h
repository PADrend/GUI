/*
	This file is part of the GUI library.
	Copyright (C) 2013 Benjamin Eikel <benjamin@eikel.org>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_LISTENERHELPER_H
#define GUI_LISTENERHELPER_H

#include "Listener.h"
#include "../GUI_Manager.h"
#include <Util/RegistryHelper.h>
#include <functional>
#include <utility>

namespace GUI {

typedef Util::RegistryHandleHolder<KeyListenerHandle> KeyListener;
template<typename component_t, typename function_t>
KeyListener createKeyListener(GUI_Manager & gui, component_t component, function_t function) {
	return KeyListener(
		std::bind(&GUI_Manager::addKeyListener,
				  &gui,
				  component,
				  static_cast<HandleKeyFun>(std::bind(function, 
													  component, 
													  std::placeholders::_1))),
		std::bind(&GUI_Manager::removeKeyListener,
				  &gui,
				  component,
				  std::placeholders::_1)
	);
}



typedef Util::RegistryHandleHolder<MouseButtonListenerHandle> MouseButtonListener;
inline MouseButtonListener createGlobalMouseButtonListener(GUI_Manager & gui, HandleMouseButtonFun function) {
	return MouseButtonListener(
		std::bind(&GUI_Manager::addGlobalMouseButtonListener, &gui, std::move(function)),
		std::bind(&GUI_Manager::removeGlobalMouseButtonListener, &gui, std::placeholders::_1)
	);
}
template<typename component_t, typename function_t>
MouseButtonListener createMouseButtonListener(GUI_Manager & gui, component_t component, function_t function) {
	return MouseButtonListener(
		std::bind(&GUI_Manager::addMouseButtonListener,
				  &gui,
				  component,
				  static_cast<HandleMouseButtonFun>(std::bind(function, 
															  component, 
															  std::placeholders::_1,
															  std::placeholders::_2))),
		std::bind(&GUI_Manager::removeMouseButtonListener,
				  &gui,
				  component,
				  std::placeholders::_1)
	);
}



typedef Util::RegistryHandleHolder<MouseClickListenerHandle> MouseClickListener;
inline MouseClickListener createMouseClickListener(GUI_Manager & gui, Component * component, HandleMouseClickFun function) {
	return MouseClickListener(
		std::bind(&GUI_Manager::addMouseClickListener, &gui, component, std::move(function)),
		std::bind(&GUI_Manager::removeMouseClickListener, &gui, component, std::placeholders::_1)
	);
}



typedef Util::RegistryHandleHolder<MouseMotionListenerHandle> MouseMotionListener;
inline MouseMotionListener createMouseMotionListener(GUI_Manager & gui, HandleMouseMotionFun function) {
	return MouseMotionListener(
		std::bind(&GUI_Manager::addGlobalMouseMotionListener, &gui, std::move(function)),
		std::bind(&GUI_Manager::removeGlobalMouseMotionListener, &gui, std::placeholders::_1)
	);
}
template<typename component_t, typename function_t>
MouseMotionListener createMouseMotionListener(GUI_Manager & gui, component_t component, function_t function) {
	return createMouseMotionListener(gui, std::bind(function,
													component, 
													std::placeholders::_1,
													std::placeholders::_2));
}

typedef Util::OptionalRegistryHandleHolder<MouseMotionListenerHandle> OptionalMouseMotionListener;
template<typename component_t, typename function_t>
OptionalMouseMotionListener createOptionalMouseMotionListener(GUI_Manager & gui, component_t component, function_t function) {
	return OptionalMouseMotionListener(
		std::bind(&GUI_Manager::addGlobalMouseMotionListener,
				  &gui,
				  static_cast<HandleMouseMotionFun>(std::bind(function, 
															  component, 
															  std::placeholders::_1,
															  std::placeholders::_2))),
		std::bind(&GUI_Manager::removeGlobalMouseMotionListener,
				  &gui,
				  std::placeholders::_1)
	);
}

}

#endif /* GUI_LISTENERHELPER_H */
