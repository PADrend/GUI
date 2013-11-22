/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_EditorPanel_H
#define GUI_EditorPanel_H

#include "Container.h"
#include "../Base/Listener.h"
#include "../GUI_Manager.h"
#include <set>

namespace GUI {

/***
 **     EditorPanel ---|> Container ---|> Component
 **/
class EditorPanel: public Container, public MouseMotionListener {
		PROVIDES_TYPE_NAME(EditorPanel)
		
		enum state_t{
			CLICK_SELECTING, DRAG_SELECTING, MOVING
		} state;

	public:

		EditorPanel(GUI_Manager & gui,flag_t flags=0);
		virtual ~EditorPanel();

		// ---|> MouseMotionListener
		virtual listenerResult_t onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent) override;

		// ---|> Container
		virtual void removeContent(const Ref & child) override	{
			unmarkChild(child.get());
			Container::removeContent(child);
		}
		virtual void clearContents() override 	{
			unmarkAll();
			Container::clearContents();
		}
		
	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

		Geometry::Vec2 dragStartPos;
		Geometry::Vec2 dragPos;

		GUI_Manager::MouseButtonListenerHandle mouseButtonListenerHandle;

		void rectSelect_start(const Geometry::Vec2 & pos);
		void rectSelect_break();
		void rectSelect_finish(const Geometry::Vec2 & pos);

		void move_start(const Geometry::Vec2 & pos);
		void move_execute(const Geometry::Vec2 & delta);
		void move_break(const Geometry::Vec2 & pos);
		void move_finish();
		
	// -----------

	/*! @name Marking */
	// @{
	public:
		typedef std::set<Component *> markedChildrenSet_t;
		/*! Add a Component to the set of marked children, if @p c is a direct child.
			@note Does not call markingChanged()
			@return true iff c was marked. */
		bool markChild(Component * c);
		/*! Remove a Component from the marking list.
			@note Does not call markingChanged()
			@return true iff c was removed. */
		bool unmarkChild(Component * c);
		/*!	Remove all components from the marking.
			@note Does not call markingChanged() */
		void unmarkAll();
		const markedChildrenSet_t & getMarkedChildren() const	{	return markedChildren;	}
		markedChildrenSet_t & getMarkedChildren() 				{	return markedChildren;	}

		/*! Notifies the GUI-Manager of a changed marking via gui.componentDataChanged(...)
			Should be called, whenever the marking has changed. */
		void markingChanged();
	private:
		markedChildrenSet_t markedChildren;
	// @}


};
}
#endif // GUI_EditorPanel_H
