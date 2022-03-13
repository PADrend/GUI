/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Splitter.h"
#include "../GUI_Manager.h"
#include "../Base/AbstractShape.h"
#include "../Base/ListenerHelper.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "ComponentPropertyIds.h"
#include "Container.h"
#include <Util/UI/Event.h>
#include <Util/Numeric.h>
#include <iostream>

namespace GUI {

//! (ctor)
Splitter::Splitter(GUI_Manager & _gui,splittingDirection_t _direction,flag_t _flags/*=0*/):
		Component(_gui,_flags),
		direction(_direction),
		mouseButtonListener(createMouseButtonListener(_gui, this, &Splitter::onMouseButton)),
		optionalMouseMotionListener(createOptionalMouseMotionListener(_gui, this, &Splitter::onMouseMove)) {
	if( direction == HORIZONTAL){
		setExtLayout(
			ExtLayouter::WIDTH_REL|ExtLayouter::HEIGHT_ABS,
			Geometry::Vec2(0,0),Geometry::Vec2(1.0,5));
	}else{
		setExtLayout(
			ExtLayouter::WIDTH_ABS|ExtLayouter::HEIGHT_REL,
			Geometry::Vec2(0,0),Geometry::Vec2(5,1.0));
	
	}
}

//! (dtor)
Splitter::~Splitter() = default;

//! ---|> Component
void Splitter::doDisplay(const Geometry::Rect & /*region*/){
	enableLocalDisplayProperties();
	displayDefaultShapes();			
	getGUI().displayShape(PROPERTY_SPLITTER_SHAPE,getLocalRect(),isSelected() ? AbstractShape::ACTIVE :0);
	disableLocalDisplayProperties();
}

class StackingLayouter : public AbstractLayouter{
	PROVIDES_TYPE_NAME(StackingLayouter)
		bool vertical;
	public:
		StackingLayouter(bool _vertical) : AbstractLayouter(),vertical(_vertical){}
		StackingLayouter(const StackingLayouter & ) = default;
		virtual ~StackingLayouter(){}
	
		
		//! ---|> AbstractLayouter
		void layout(Util::WeakPointer<Component> component) override{
			Container * container = dynamic_cast<Container*>(component.get());
			if(!container){
				throw std::invalid_argument("FlowLayouter can only be applied to Containers.");
			}
			if(vertical){
				float x = 0;
				for(Component * c=container->getFirstChild();c!=nullptr;c=c->getNext()){
					c->setPosition( Geometry::Vec2(x,0));
					x+=c->getWidth();
				}
			}else{
				float y = 0;
				for(Component * c=container->getFirstChild();c!=nullptr;c=c->getNext()){
					c->setPosition( Geometry::Vec2(0,y));
					y+=c->getHeight();
				}
			}
		}

};
//! ---|> Component
void Splitter::doLayout(){
	Component * prevComp=getPrev();
	Component * nextComp=getNext();
	if(prevComp==nullptr || nextComp==nullptr || !hasParent())
		return;

	bool found=false;
	for(auto & layouter:getParent()->getLayouters()){
		if(dynamic_cast<StackingLayouter*>(layouter.get()))
			found = true;
	}
	if(!found){
		getParent()->addLayouter(new StackingLayouter(direction == VERTICAL));
	}
	if( direction == VERTICAL){
		nextComp->setWidth( getParent()->getWidth()-getWidth()-prevComp->getWidth() );
	}else{
		nextComp->setHeight( getParent()->getHeight()-getHeight()-prevComp->getHeight() );
	}

}

bool Splitter::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
		if(buttonEvent.pressed && !isSelected()) {
			select();
			optionalMouseMotionListener.enable();
			return true;
		} else if(!buttonEvent.pressed && isSelected()) {
			unselect();
			optionalMouseMotionListener.disable();
			return true;
		}
	}
	return false;
}

bool Splitter::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
	if( !hasParent() || getPrev()==nullptr || getNext()==nullptr || !(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT)) {
		unselect();
		optionalMouseMotionListener.disable();
		return true;
	}
	if(direction == VERTICAL) {
		getPrev()->setWidth(std::max(0.0f, std::min(getPrev()->getWidth() + motionEvent.deltaX, getParent()->getWidth() - getWidth())));
	} else {
		getPrev()->setHeight(std::max(0.0f, std::min(getPrev()->getHeight() + motionEvent.deltaY, getParent()->getHeight() - getHeight())));
	}
	invalidateLayout();
	getPrev()->invalidateLayout();
	getNext()->invalidateLayout();
	getParent()->invalidateLayout();
	return true;
}

}
