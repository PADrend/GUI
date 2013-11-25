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
#include <memory>

namespace GUI {

/**
 * Register a HandleMouseMotionFunction.
 * 
 * @param gui The object containing the function registry.
 * @param optionalHandle Pointer to a handle that will be used to store the
 * handle returned by the registry. If the pointer already points to a handle,
 * nothing will be done.
 * @param function Function that will be registered as mouse motion handler.
 * @param component Component on which the @p function will be called.
 */
template<typename function_t, typename component_t>
static void startListeningOnMouseMove(GUI_Manager & gui,
									  std::unique_ptr<MouseMotionListenerHandle> & optionalHandle,
									  function_t function,
									  component_t component) {
	if(optionalHandle) {
		return;
	}
	optionalHandle.reset(new MouseMotionListenerHandle(
		gui.addGlobalMouseMotionListener(std::bind(function,
												   component, 
												   std::placeholders::_1,
												   std::placeholders::_2))));
}

/**
 * Cancel the registration of a HandleMouseMotionFunction.
 * 
 * @param gui The object containing the function registry.
 * @param optionalHandle Pointer to a handle that will be given to the registry
 * to cancel the registration of a function. If the pointer does not point to a
 * handle, nothing will be done.
 */
static void stopListeningOnMouseMove(GUI_Manager & gui,
									 std::unique_ptr<MouseMotionListenerHandle> & optionalHandle) {
	if(!optionalHandle) {
		return;
	}
	gui.removeGlobalMouseMotionListener(std::move(*optionalHandle.get()));
	optionalHandle.reset();
}

}

#endif /* GUI_LISTENERHELPER_H */
