/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "ScrollableContainer.h"
#include "../GUI_Manager.h"
#include "../Base/AnimationHandler.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "Scrollbar.h"
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>
#include <algorithm>
#include <iostream>

// ------------------------------------------------------------------------------
namespace GUI {


static const Util::StringIdentifier dataId_verticallScrollPos("scroll");


//! (ctor)
ScrollableContainer::ScrollableContainer(GUI_Manager & _gui,flag_t _flags/*=0*/)
		: Container(_gui,_flags),DataChangeListener(),MouseButtonListener(),MouseMotionListener(),contentContainer(new Container(_gui)){

	_addChild(contentContainer.get());
	init();
	//ctor
}

void ScrollableContainer::init(){
	contentContainer->setFlag(IS_CLIENT_AREA,true);
	addMouseButtonListener(this);
	setFlag(USE_SCISSOR,true);
}

//! (dtor)
ScrollableContainer::~ScrollableContainer() {
	getGUI().removeMouseMotionListener(this);
	//dtor
}
//! ---|> Component
void ScrollableContainer::doDisplay(const Geometry::Rect & region) {
	displayChildren(region,true);
	if(scrollPos.y()>0)
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE, getLocalRect(), 0);
	if(scrollPos.x()>0)
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_LEFT_SHAPE, getLocalRect(), 0);
	if(scrollPos.y()<maxScrollPos.y())
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE, getLocalRect(), 0);
	if(scrollPos.x()<maxScrollPos.x())
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_RIGHT_SHAPE, getLocalRect(), 0);
}

//! ---|> Component
void ScrollableContainer::doLayout() {
	maxScrollPos = Geometry::Vec2(contentContainer->getWidth()-getWidth() , contentContainer->getHeight()-getHeight() );
	if (maxScrollPos.y()<=0){
		maxScrollPos.y(0);
		if(vScrollBar.isNotNull()){
			Component::destroy(vScrollBar.get());
			vScrollBar = nullptr;
		}
	}else{
		if(vScrollBar.isNull()){
			vScrollBar=new Scrollbar(getGUI(),dataId_verticallScrollPos,Scrollbar::VERTICAL);
			vScrollBar->addDataChangeListener(this);
			vScrollBar->setExtLayout( 	ExtLayouter::POS_X_ABS|ExtLayouter::REFERENCE_X_RIGHT|ExtLayouter::ALIGN_X_RIGHT|
									ExtLayouter::POS_Y_ABS|ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::ALIGN_Y_TOP |
									ExtLayouter::WIDTH_ABS|ExtLayouter::HEIGHT_ABS,
									Geometry::Vec2(1,2),Geometry::Vec2(getGUI().getGlobalValue(PROPERTY_SCROLLBAR_WIDTH),-4));

			_addChild(vScrollBar.get());
		}
		vScrollBar->setMaxScrollPos(maxScrollPos.y());
		vScrollBar->setScrollPos(scrollPos.y());
	 }

	if (maxScrollPos.x()<=0){
		maxScrollPos.x(0);
	}

	if(scrollPos.y()<0 )	scrollPos.y(0);
	if(scrollPos.y()>maxScrollPos.y() )scrollPos.y(maxScrollPos.y());

	if(scrollPos.x()<0 )	scrollPos.x(0);
	if(scrollPos.x()>maxScrollPos.x() )scrollPos.x(maxScrollPos.x());

	// set scrollPosition
	contentContainer->setPosition(-scrollPos);
}


//! ---|> DataChangeListener
void ScrollableContainer::handleDataChange(Component *,const Util::StringIdentifier & actionName){
	if(vScrollBar.isNotNull() && actionName==dataId_verticallScrollPos){
		if(scrollPos.y()!=vScrollBar->getScrollPos()){
			invalidateRegion();
			invalidateLayout();
			scrollPos.y(vScrollBar->getScrollPos());
		}
	}
	
}

//! ---|> MouseButtonListener
listenerResult_t ScrollableContainer::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent){
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_MIDDLE) {
		if(maxScrollPos.x()<=0 && maxScrollPos.y()<=0)
			return LISTENER_EVENT_NOT_CONSUMED;
		else if(buttonEvent.pressed){
			getGUI().addMouseMotionListener(this);
		} else {// !pressed
			getGUI().removeMouseMotionListener(this);
		}
		return LISTENER_EVENT_CONSUMED;
	}
	return LISTENER_EVENT_NOT_CONSUMED;

}

//! ---|> MouseMotionListener
listenerResult_t ScrollableContainer::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent){
	if(!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_MIDDLE)) {
		return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
	}
	const Geometry::Vec2 delta(motionEvent.deltaX, motionEvent.deltaY);
	scrollTo(scrollPos - delta * 2.0);
	return LISTENER_EVENT_CONSUMED;
}

void ScrollableContainer::scrollTo(const Geometry::Vec2 & pos){
	const Geometry::Vec2 oldScrollPos = scrollPos;
	scrollPos.x( std::max( std::min( pos.x(), maxScrollPos.x() ) , 0.0f ) );
	scrollPos.y( std::max( std::min( pos.y(), maxScrollPos.y() ) , 0.0f ) );
	if(scrollPos!=oldScrollPos){
		invalidateLayout();
	}
}

//! ScrollableContainer_ScrollAnimation ---|> AnimationHandler
class ScrollableContainer_ScrollAnimation:public AnimationHandler{
	public:
		Geometry::Vec2 targetPos;

		ScrollableContainer_ScrollAnimation(ScrollableContainer * p,const Geometry::Vec2 & _targetPos, float _duration) :
			AnimationHandler(p,_duration),targetPos(_targetPos){
		}
		virtual ~ScrollableContainer_ScrollAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float t) override{
			if(t > getEndTime()) {
				finish();
				return false;
			}
			ScrollableContainer * p= dynamic_cast<ScrollableContainer *>(getComponent());

			for(float f=0.0;f<t-getStartTime();f+=0.01) // for every 10 ms
				p->scrollTo( (p->getScrollPos()*99.0f + targetPos)*0.01f);
			return true;
		}

		// ---|> AnimationHandler
		void finish() override{
			dynamic_cast<ScrollableContainer *>(getComponent())->scrollTo(targetPos);
		}
};


void ScrollableContainer::scrollTo(const Geometry::Vec2 & pos,float duration){
	getGUI().stopAnimations(this);
	getGUI().addAnimationHandler(new ScrollableContainer_ScrollAnimation(this,pos,duration));
}

}
