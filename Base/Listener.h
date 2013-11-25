/*
	This file is part of the GUI library.
	Copyright (C) 2013 Benjamin Eikel <benjamin@eikel.org>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_LISTENER_H
#define GUI_LISTENER_H

#include <functional>

namespace Geometry {
template<typename T_> class _Vec2;
typedef _Vec2<float> Vec2f;
}
namespace Util {
namespace UI {
struct ButtonEvent;
struct KeyboardEvent;
struct MotionEvent;
}
class StringIdentifier;
}

namespace GUI {

class Component;

/**
 * Type of functions reacting on an action. The function receives the component
 * that caused the action as parameter, and the associated action name. If it
 * returns @c true, the callee signalizes that is processed the action and no
 * other action handlers will be called. If it returns @c false, the callee
 * signalizes that it did not process the action and the next action handler
 * will be called.
 */
typedef std::function<bool (Component *, 
							const Util::StringIdentifier &)> HandleActionFun;


/**
 * Type of functions reacting on a change of a component's data. The function
 * receives the component for which the data has changed as parameter.
 */
typedef std::function<void (Component *)> HandleDataChangeFun;

/**
 * Type of functions reacting on a mouse button event onto a component. The
 * function receives the component onto which the mouse button event was
 * detected, and the mouse button event.
 * If it returns @c true, the callee signalizes that is processed the event and
 * no other event handlers will be called. If it returns @c false, the callee
 * signalizes that it did not process the event and the next event handler
 * will be called.
 */
typedef std::function<bool (Component *, 
							const Util::UI::ButtonEvent &)> HandleMouseButtonFun;

/**
 * Type of functions reacting on a mouse click (mouse button down and mouse 
 * button up) onto a component. The function receives the component onto which
 * the mouse click was detected, the mouse button, and the local position of
 * the click inside the component.
 * If it returns @c true, the callee signalizes that is processed the click and
 * no other click handlers will be called. If it returns @c false, the callee
 * signalizes that it did not process the click and the next click handler
 * will be called.
 * 
 * @note In order to receive mouse clicks on a component, the component has to
 * get activated via MouseButtonListener first.
 */
typedef std::function<bool (Component *, 
							unsigned int, 
							const Geometry::Vec2f &)> HandleMouseClickFun;

/**
 * Type of functions reacting on a mouse motion event onto a component. The
 * function receives the component onto which the mouse motion event was
 * detected, and the mouse motion event.
 * If it returns @c true, the callee signalizes that is processed the event and
 * no other event handlers will be called. If it returns @c false, the callee
 * signalizes that it did not process the event and the next event handler
 * will be called.
 */
typedef std::function<bool (Component *, 
							const Util::UI::MotionEvent &)> HandleMouseMotionFun;

}

#endif // GUI_LISTENER_H
