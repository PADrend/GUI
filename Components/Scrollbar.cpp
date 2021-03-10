/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Scrollbar.h"
#include "../GUI_Manager.h"
#include "../Base/AbstractShape.h"
#include "../Base/ListenerHelper.h"
#include "../Base/ListenerHelper.h"
#include "ComponentPropertyIds.h"
#include "Button.h"
#include <Util/UI/Event.h>
#include <iostream>

namespace GUI{
/***
 **  Scrollbar ---|> Component
 **/
class ScrollMarker : public Component {
		Scrollbar & myScrollbar;
		float dragStartPos;
		float dragStartScroll;
		bool catchDragStartPos;

		MouseButtonListener mouseButtonListener;
		OptionalMouseMotionListener optionalMouseMotionListener;

	public:
	//! (ctor)
	ScrollMarker(GUI_Manager & _gui,Scrollbar & _myScrollbar):
			Component(_gui),myScrollbar(_myScrollbar),
			mouseButtonListener(createMouseButtonListener(_gui, this, &ScrollMarker::onMouseButton)),
			optionalMouseMotionListener(createOptionalMouseMotionListener(_gui, this, &ScrollMarker::onMouseMove)) {
	}
	//! (ctor)
	virtual ~ScrollMarker() = default;

	bool onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
		if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP || buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
			return false;
		}
		if (buttonEvent.pressed) {
			select();
			if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
				startDragging();
			}
		}
		return true;
	}

	bool onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
		if (!isActive() || motionEvent.buttonMask == Util::UI::MASK_NO_BUTTON) {
			optionalMouseMotionListener.disable();
			return false;
		}
		const float p = myScrollbar.isVertical() ? motionEvent.y : motionEvent.x;
		if(catchDragStartPos){
			catchDragStartPos = false;
			dragStartPos = p;
			dragStartScroll = static_cast<float>(myScrollbar.getScrollPos());
		}else{
			myScrollbar.updateScrollPos( static_cast<int32_t>(((p-dragStartPos) * myScrollbar.getMaxScrollPos() / 
										((myScrollbar.isVertical() ? myScrollbar.getHeight() : myScrollbar.getWidth())-myScrollbar.getMarkerSize())) + dragStartScroll) ) ;
		
		}
		return true;
	}
	void startDragging(){
		activate();
		optionalMouseMotionListener.enable();
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
Scrollbar::Scrollbar(GUI_Manager & _gui, flag_t _flags):
		Container(_gui, Geometry::Rect(), _flags),
		maxScrollPos(1),scrollPos(0),
		mouseButtonListener(createMouseButtonListener(_gui, this, &Scrollbar::onMouseButton)) {
	setFlag(SELECTABLE,true);

	marker=new ScrollMarker(getGUI(),*this);
	_addChild(marker.get());
}

Scrollbar::~Scrollbar() = default;

//! ---|> Component
void Scrollbar::doLayout(){

	const float pos=getMarkerPosFromScrollPos(static_cast<float>(getScrollPos()));
	if(isVertical()){
		marker->setPosition(Geometry::Vec2(0,pos));
		marker->setSize(getWidth(),static_cast<float>(getMarkerSize()));
	}else{
		marker->setPosition(Geometry::Vec2(pos,0));
		marker->setSize(static_cast<float>(getMarkerSize()),getHeight());
	}
}

//! ---|> Component
void Scrollbar::doDisplay(const Geometry::Rect & region) {
	enableLocalDisplayProperties();
	displayDefaultShapes();			
	if (isVertical()) {
		getGUI().displayShape(PROPERTY_SCROLLBAR_VERTICAL_BAR_SHAPE,getLocalRect(),isSelected() ? AbstractShape::ACTIVE : 0);
	} else {
		getGUI().displayShape(PROPERTY_SCROLLBAR_HORIZONTAL_BAR_SHAPE,getLocalRect(),isSelected() ? AbstractShape::ACTIVE : 0);
	}
	if (isSelected()) {
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,getLocalRect());
	}
	disableLocalDisplayProperties();
	displayChildren(region);
}

//! ---|> MouseButtonListener
bool Scrollbar::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if (buttonEvent.pressed && !isLocked()) {
		select();
		if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
			const float p = isVertical() ? localPos.getY() : localPos.getX();
			const float markerPos = getMarkerPosFromScrollPos(static_cast<float>(getScrollPos()));
			// clicked above marker? -> pgUp
			if(p<markerPos){
				updateScrollPos(static_cast<int32_t>(getScrollPos() - (isVertical() ? getHeight() : getWidth())*0.9f) );
			}// clicked below marker? -> pgDown
			else if(p>markerPos+getMarkerSize()){
				updateScrollPos(static_cast<int32_t>(getScrollPos() + (isVertical() ? getHeight() : getWidth())*0.9f) );
			} // marker clicked? -> kame active
			else{
				static_cast<ScrollMarker*>(marker.get())->startDragging();
			}
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP) {
			updateScrollPos(static_cast<int32_t>(getScrollPos()-(isVertical() ? getHeight() : getWidth())*0.2f));
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
			updateScrollPos(static_cast<int32_t>(getScrollPos()+(isVertical() ? getHeight() : getWidth())*0.2f));
		}
	}
	return true;

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
	getGUI().componentDataChanged(this);
}

}
