/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "ScrollableContainer.h"
#include "../GUI_Manager.h"
#include "../Base/AnimationHandler.h"
#include "../Base/ListenerHelper.h"
#include "../Base/StyleManager.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "Scrollbar.h"
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>
#include <algorithm>
#include <iostream>

// ------------------------------------------------------------------------------
namespace GUI {

//! (ctor)
ScrollableContainer::ScrollableContainer(GUI_Manager & _gui,flag_t _flags/*=0*/) :
		Container(_gui,_flags),
		contentContainer(new Container(_gui)),
		mouseButtonListener(createMouseButtonListener(_gui, this, &ScrollableContainer::onMouseButton)),
		optionalMouseMotionListener(createOptionalMouseMotionListener(_gui, this, &ScrollableContainer::onMouseMove)) {
	_addChild(contentContainer.get());
	contentContainer->setFlag(IS_CLIENT_AREA,true);
	setFlag(USE_SCISSOR,true);
}

//! (dtor)
ScrollableContainer::~ScrollableContainer() {
	if(optionalScrollBarListener) {
		getGUI().removeDataChangeListener(vScrollBar.get(),
										  std::move(*optionalScrollBarListener.get()));
		optionalScrollBarListener.reset();
	}
}

//! ---|> Component
void ScrollableContainer::doDisplay(const Geometry::Rect & region) {
	enableLocalDisplayProperties();
	displayDefaultShapes();		
	auto scrollableMarker_top = scrollPos.y()>0 ? getGUI().getStyleManager().getShape(PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE) : nullptr;
	auto scrollableMarker_left = scrollPos.x()>0 ? getGUI().getStyleManager().getShape(PROPERTY_SCROLLABLE_MARKER_LEFT_SHAPE) : nullptr;
	auto scrollableMarker_bottom = scrollPos.y()<maxScrollPos.y() ? getGUI().getStyleManager().getShape(PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE) : nullptr;
	auto scrollableMarker_right = scrollPos.x()<maxScrollPos.x() ? getGUI().getStyleManager().getShape(PROPERTY_SCROLLABLE_MARKER_RIGHT_SHAPE) : nullptr;
	disableLocalDisplayProperties();
	
	displayChildren(region,true);
	
	if(scrollableMarker_top)
		scrollableMarker_top->display( getLocalRect() );
	if(scrollableMarker_left)
		scrollableMarker_left->display( getLocalRect() );
	if(scrollableMarker_bottom)
		scrollableMarker_bottom->display( getLocalRect() );
	if(scrollableMarker_right)
		scrollableMarker_right->display( getLocalRect() );
}

//! ---|> Component
void ScrollableContainer::doLayout() {
	maxScrollPos = Geometry::Vec2(contentContainer->getWidth()-getWidth() , contentContainer->getHeight()-getHeight() );
	if (maxScrollPos.y()<=0){
		maxScrollPos.y(0);
		if(vScrollBar.isNotNull()){
			getGUI().removeDataChangeListener(vScrollBar.get(), 
											  std::move(*optionalScrollBarListener.get()));
			optionalScrollBarListener.reset();
			Component::destroy(vScrollBar.get());
			vScrollBar = nullptr;
		}
	}else{
		if(vScrollBar.isNull()){
			vScrollBar=new Scrollbar(getGUI(), Scrollbar::VERTICAL);
			optionalScrollBarListener.reset(new DataChangeListenerHandle(getGUI().addDataChangeListener(
												vScrollBar.get(),
												[this](Component *) {
													if(vScrollBar.isNotNull()) {
														if(scrollPos.y() != vScrollBar->getScrollPos()) {
															invalidateRegion();
															invalidateLayout();
															scrollPos.y(static_cast<float>(vScrollBar->getScrollPos()));
														}
													}
												})));
			vScrollBar->setExtLayout( 	ExtLayouter::POS_X_ABS|ExtLayouter::REFERENCE_X_RIGHT|ExtLayouter::ALIGN_X_RIGHT|
									ExtLayouter::POS_Y_ABS|ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::ALIGN_Y_TOP |
									ExtLayouter::WIDTH_ABS|ExtLayouter::HEIGHT_ABS,
									Geometry::Vec2(1,2),Geometry::Vec2(getGUI().getGlobalValue(PROPERTY_SCROLLBAR_WIDTH),-4));

			_addChild(vScrollBar.get());
		}
		vScrollBar->setMaxScrollPos(static_cast<uint32_t>(maxScrollPos.y()));
		vScrollBar->setScrollPos(static_cast<uint32_t>(scrollPos.y()));
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

bool ScrollableContainer::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_MIDDLE) {
		if(maxScrollPos.x()<=0 && maxScrollPos.y()<=0)
			return false;
		else if(buttonEvent.pressed){
			optionalMouseMotionListener.enable();
		} else {// !pressed
			optionalMouseMotionListener.disable();
		}
		return true;
	}
	return false;

}

bool ScrollableContainer::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
	if(!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_MIDDLE)) {
		optionalMouseMotionListener.disable();
		return false;
	}
	const Geometry::Vec2 delta(motionEvent.deltaX, motionEvent.deltaY);
	scrollTo(scrollPos - delta * 2.0);
	return true;
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

		ScrollableContainer_ScrollAnimation(ScrollableContainer * p,Geometry::Vec2 _targetPos, float _duration) :
			AnimationHandler(p,_duration),targetPos(std::move(_targetPos)){
		}
		virtual ~ScrollableContainer_ScrollAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float t) override{
			if(t > getEndTime()) {
				finish();
				return false;
			}
			ScrollableContainer * p= dynamic_cast<ScrollableContainer *>(getComponent());

			for(float f=0.0f;f<t-getStartTime();f+=0.01f) // for every 10 ms
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
