/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Scrollbar.h"
#include "../GUI_Manager.h"
#include "../Base/AbstractShape.h"
#include "../Base/Listener.h"
#include "ComponentPropertyIds.h"
#include "Button.h"
#include <Util/UI/Event.h>
#include <iostream>

namespace GUI{
/***
 **  Scrollbar ---|> Component
 **/
class ScrollMarker:public Component,public MouseMotionListener,public MouseButtonListener {
		Scrollbar & myScrollbar;
		float dragStartPos;
		float dragStartScroll;
		bool catchDragStartPos;

	public:
	//! (ctor)
	ScrollMarker(GUI_Manager & _gui,Scrollbar & _myScrollbar):
			Component(_gui),myScrollbar(_myScrollbar) {
		addMouseButtonListener(this);
	}
	//! (ctor)
	virtual ~ScrollMarker() {
		getGUI().removeMouseMotionListener(this);
	}

	//! ---|> MouseButtonListener
	listenerResult_t onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) override {
		if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP || buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
			return LISTENER_EVENT_NOT_CONSUMED;
		}
		if (buttonEvent.pressed) {
			select();
			if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
				startDragging();
			}
		}
		return LISTENER_EVENT_CONSUMED;
	}

	//! ---|> MouseMotionListener
	listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override{
		if (!isActive() || motionEvent.buttonMask == Util::UI::MASK_NO_BUTTON) {
			return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
		}
		const float p = myScrollbar.isVertical() ? motionEvent.y : motionEvent.x;
		if(catchDragStartPos){
			catchDragStartPos = false;
			dragStartPos = p;
			dragStartScroll = myScrollbar.getScrollPos();
		}else{
			myScrollbar.updateScrollPos( ((p-dragStartPos) * myScrollbar.getMaxScrollPos() / 
										((myScrollbar.isVertical() ? myScrollbar.getHeight() : myScrollbar.getWidth())-myScrollbar.getMarkerSize())) + dragStartScroll ) ;
		
		}
		return LISTENER_EVENT_CONSUMED;
	}
	void startDragging(){
		activate();
		getGUI().addMouseMotionListener(this);
		catchDragStartPos = true;
	}

	//! ---|> Component
	void doDisplay(const Geometry::Rect & /*region*/) override {
		if(myScrollbar.isVertical())
			getGUI().displayShape(PROPERTY_SCROLLBAR_VERTICAL_MARKER_SHAPE,getLocalRect(),isActive() ? AbstractShape::ACTIVE : 0);
		else
			getGUI().displayShape(PROPERTY_SCROLLBAR_HORIZONTAL_MARKER_SHAPE,getLocalRect(),isActive() ? AbstractShape::ACTIVE : 0);
	}
};

// ----------------------------------------------------

//! (ctor)
Scrollbar::Scrollbar(GUI_Manager & _gui,Util::StringIdentifier _dataName,flag_t _flags/*=0*/):
		Container(_gui,Geometry::Rect(),_flags),MouseButtonListener(),
		maxScrollPos(1),scrollPos(0),dataName(std::move(_dataName)) {
	setFlag(SELECTABLE,true);

	marker=new ScrollMarker(getGUI(),*this);
	_addChild(marker.get());

	addMouseButtonListener(this);
}

//! ---|> Component
void Scrollbar::doLayout(){

	const float pos=getMarkerPosFromScrollPos(getScrollPos());
	if(isVertical()){
		marker->setPosition(Geometry::Vec2(0,pos));
		marker->setSize(getWidth(),getMarkerSize());
	}else{
		marker->setPosition(Geometry::Vec2(pos,0));
		marker->setSize(getMarkerSize(),getHeight());
	}
}

//! ---|> Component
void Scrollbar::doDisplay(const Geometry::Rect & region) {
	if (isVertical()) {
		getGUI().displayShape(PROPERTY_SCROLLBAR_VERTICAL_BAR_SHAPE,getLocalRect(),isSelected() ? AbstractShape::ACTIVE : 0);
	} else {
		getGUI().displayShape(PROPERTY_SCROLLBAR_HORIZONTAL_BAR_SHAPE,getLocalRect(),isSelected() ? AbstractShape::ACTIVE : 0);
	}
	if (isSelected()) {
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,getLocalRect());
	}
	displayChildren(region);
}

//! ---|> MouseButtonListener
listenerResult_t Scrollbar::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if (buttonEvent.pressed && !isLocked()) {
		select();
		if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
			const float p = isVertical() ? localPos.getY() : localPos.getX();
			const float markerPos = getMarkerPosFromScrollPos(getScrollPos());
			// clicked above marker? -> pgUp
			if(p<markerPos){
				updateScrollPos(getScrollPos() - (isVertical() ? getHeight() : getWidth())*0.9 );
			}// clicked below marker? -> pgDown
			else if(p>markerPos+getMarkerSize()){
				updateScrollPos(getScrollPos() + (isVertical() ? getHeight() : getWidth())*0.9 );
			} // marker clicked? -> kame active
			else{
				static_cast<ScrollMarker*>(marker.get())->startDragging();
			}
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP) {
			updateScrollPos(getScrollPos()-(isVertical() ? getHeight() : getWidth())*0.2);
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
			updateScrollPos(getScrollPos()+(isVertical() ? getHeight() : getWidth())*0.2);
		}
	}
	return LISTENER_EVENT_CONSUMED;

}


void Scrollbar::setScrollPos(const uint32_t f) {
	if(f!=scrollPos){
		invalidateRegion();
		invalidateLayout();
		scrollPos = f;
	}
}

void Scrollbar::setMaxScrollPos(const uint32_t _maxScrollPos) {
	if(_maxScrollPos!=maxScrollPos){
		maxScrollPos = _maxScrollPos;
		invalidateLayout();
	}
}

int Scrollbar::getMarkerSize()const{
	const float f = isVertical() ? getHeight() : getWidth();
	return std::max(5,static_cast<int>(f * f / (f+maxScrollPos)));
}

float Scrollbar::getMarkerPosFromScrollPos(float _scrollPos)const {
	const float maxMarkerPos = (isVertical()?getHeight():getWidth()) - getMarkerSize();
	return (maxMarkerPos * _scrollPos) / maxScrollPos;
}

void Scrollbar::updateScrollPos(const int32_t f) {
	setScrollPos( std::min( maxScrollPos, static_cast<uint32_t>(std::max(static_cast<int32_t>(0),f)) ));
	getGUI().componentDataChanged(this,dataName.toString());
}

}
