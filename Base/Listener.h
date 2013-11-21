/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_LISTENER_H
#define GUI_LISTENER_H
#include <cstdint>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <Geometry/Vec2.h>
#include <Util/StringIdentifier.h>

namespace Util {
namespace UI {
struct ButtonEvent;
struct KeyboardEvent;
struct MotionEvent;
}
}

namespace GUI {

class Component;

/*! Generic container for storing the associations from Listeners to Components.
	Internally used by the Listener classes. */
template<typename Listener_t>
class RegisteredListenerRegistry {
public:
	// Component --> Listener*
	typedef typename std::list< Listener_t * > listenerList_t;
	typedef std::unordered_map<Component *, listenerList_t > listenerMap_t;
	listenerMap_t listenerMap;

	/*! Register a Listener to the given Component.	*/
	void add( Component * c, Listener_t * l){
		if(c==nullptr || l==nullptr)
			return;
		listenerList_t & listener=listenerMap[c];
		typename listenerList_t::iterator it=std::find(listener.begin(), listener.end(), l);
		if(it==listener.end())
			listener.push_back(l);
	}

	/*! Unregister a Listener from the given Component.	*/
	void remove( Component * c, Listener_t * l){
		if(c==nullptr || l==nullptr)
			return;
		listenerList_t & listener=listenerMap[c];
		listener.remove(l);
		if( listener.empty() )
			listenerMap.erase(c);
	}

	/*! Get the list of all listeners for the given component */
	listenerList_t * getListeners(Component * c){
		typename listenerMap_t::iterator it = listenerMap.find(c);
		return it==listenerMap.end() ? nullptr : &it->second;
	}

	/*! Remove all listeners of the given component */
	void removeListeners(Component * c){
		listenerMap.erase(c);
	}
};

// -------------------------------------------------
/**
 * @brief Return type for event handling functions
 * 
 * The enumerator that is returned by the event handling function determines
 * how to proceed (e.g. call other event handlers, remove the event handling
 * function).
 */
enum listenerResult_t : uint32_t {
	//! The listener returning this constant has not processed the event.
	LISTENER_EVENT_NOT_CONSUMED 	= 0,
	//! The listener returning this constant has processed the event.
	LISTENER_EVENT_CONSUMED 		= 1 << 0,
	/**
	 * Remove the listener returning this constant. This enumerator should
	 * never be returned alone, but instead one of the combinations below.
	 */
	LISTENER_REMOVE_LISTENER 		= 1 << 1,
	//! Do not consume the event and remove the listener.
	LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER = 
					LISTENER_EVENT_NOT_CONSUMED | LISTENER_REMOVE_LISTENER,
	//! Consume the event and remove the listener.
	LISTENER_EVENT_CONSUMED_AND_REMOVE_LISTENER = 
					LISTENER_EVENT_CONSUMED | LISTENER_REMOVE_LISTENER
};
// -------------------------------------------------

struct ActionListener {
	static RegisteredListenerRegistry<ActionListener> & getListenerRegistry();
	virtual listenerResult_t handleAction(Component *,const Util::StringIdentifier & actionName)=0;
	virtual ~ActionListener() {}
};
struct DataChangeListener {
	static RegisteredListenerRegistry<DataChangeListener> & getListenerRegistry();
	virtual void handleDataChange(Component *,const Util::StringIdentifier & actionName)=0;
	virtual ~DataChangeListener() {}
};
struct MouseMotionListener {
	static RegisteredListenerRegistry<MouseMotionListener> & getListenerRegistry();
	virtual listenerResult_t onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent) = 0;
	virtual ~MouseMotionListener() {}
};
struct MouseButtonListener {
	static RegisteredListenerRegistry<MouseButtonListener> & getListenerRegistry();
	virtual listenerResult_t onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent) = 0;
	virtual ~MouseButtonListener() {}
};
/**
 * \note In order to receive mouse clicks on a component, it has to get activated via MouseButtonListener first.
 */
struct MouseClickListener {
	static RegisteredListenerRegistry<MouseClickListener> & getListenerRegistry();
	virtual bool onMouseClick(Component * component, unsigned int button,const Geometry::Vec2 &pos)=0;
	virtual ~MouseClickListener() {}
};
struct KeyListener {
	static RegisteredListenerRegistry<KeyListener> & getListenerRegistry();
	virtual bool onKeyEvent(Component * component, const Util::UI::KeyboardEvent & keyEvent) = 0;
	virtual ~KeyListener() {}
};

}
#endif // GUI_LISTENER_H
