/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Button.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "../Base/AbstractShape.h"
#include "../Base/Properties.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>
#include <iostream>

namespace GUI {
	
const Util::StringIdentifier Button::ACTION_Button_click("Button_click");

static ExtLayouter * getDefaultLabelLayouter(){
	static Util::Reference<ExtLayouter> l;
	if(l.isNull()){
		l = new ExtLayouter;
		l->setFlags(ExtLayouter::WIDTH_REL|ExtLayouter::HEIGHT_REL);
		l->setSize(Geometry::Vec2(1.0f,1.0f));
	}
	return l.get();
}

//! (ctor)
Button::Button(GUI_Manager & _gui,flag_t _flags/*=0*/)
		:Container(_gui,_flags),MouseMotionListener(),
		actionName(ACTION_Button_click),switchedOn(false),hover(false),actionListener(),
		keyListenerHandle(_gui.addKeyListener(this, std::bind(&Button::onKeyEvent, 
															  this, 
															  std::placeholders::_1))),
		mouseButtonListenerHandle(_gui.addMouseButtonListener(this, std::bind(&Button::onMouseButton, 
																			  this, 
																			  std::placeholders::_1,
																			  std::placeholders::_2))),
		mouseClickListenerHandle(_gui.addMouseClickListener(	this,
																[this](Component *, unsigned int, const Geometry::Vec2 &) {
																	if(!isLocked()) {
																		action();
																	}
																	return true;
																})) {
	setFlag(SELECTABLE,true);

	// create Label
	textLabel=new Label(getGUI(),"");
	textLabel->addLayouter(getDefaultLabelLayouter());
	textLabel->setTextStyle(Draw::TEXT_ALIGN_CENTER|Draw::TEXT_ALIGN_MIDDLE);
	addContent(textLabel.get());
	addMouseMotionListener(this);
}

//! (dtor)
Button::~Button() {
	getGUI().removeMouseMotionListener(this);
	getGUI().removeMouseClickListener(this, std::move(mouseClickListenerHandle));
	getGUI().removeMouseButtonListener(this, std::move(mouseButtonListenerHandle));
	getGUI().removeKeyListener(this, std::move(keyListenerHandle));
}

//! ---|> Component
void Button::doDisplay(const Geometry::Rect & region){

	if( (!getFlag(FLAT_BUTTON)) || hover || isSwitchedOn() || isActive()){
		getGUI().displayShape(PROPERTY_BUTTON_SHAPE,getLocalRect(),(isActive()||isSwitchedOn())?AbstractShape::ACTIVE : 0);
	}


	if (isActive()){
		Draw::moveCursor(Geometry::Vec2(1,1));
		displayChildren(region);
		Draw::moveCursor(-Geometry::Vec2(1,1));
	}else{
		if(isSelected()) {
			Geometry::Rect r = getLocalRect();
			r.changeSizeCentered(-4, -4);
			getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,r);
		}

		if(hover){
			Util::Reference<AbstractProperty> c = new ColorProperty(PROPERTY_TEXT_COLOR,getGUI().getActiveColor(PROPERTY_BUTTON_HOVERED_TEXT_COLOR));
			Util::Reference<AbstractProperty> c2 = new ColorProperty(PROPERTY_ICON_COLOR,getGUI().getActiveColor(PROPERTY_BUTTON_HOVERED_TEXT_COLOR));
			getGUI().enableProperty(c);
			getGUI().enableProperty(c2);

			displayChildren(region);

			getGUI().disableProperty(c2);
			getGUI().disableProperty(c);
		}else if(isSwitchedOn()){
			Util::Reference<AbstractProperty> c = new ColorProperty(PROPERTY_TEXT_COLOR,getGUI().getActiveColor(PROPERTY_BUTTON_ENABLED_COLOR));
			Util::Reference<AbstractProperty> c2 = new ColorProperty(PROPERTY_ICON_COLOR,getGUI().getActiveColor(PROPERTY_BUTTON_ENABLED_COLOR));
			getGUI().enableProperty(c);
			getGUI().enableProperty(c2);

			displayChildren(region);

			getGUI().disableProperty(c2);
			getGUI().disableProperty(c);
			
		}else{
			displayChildren(region);		
		}

	}
}


bool Button::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_MIDDLE) {
		return false;
	}
	if(buttonEvent.pressed && !isLocked()){
		select();
		if(buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			activate();
		}
	}
	return true;
}

bool Button::onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if(!isLocked() && keyEvent.key == Util::UI::KEY_RETURN){
		select();
		if(keyEvent.pressed){
			activate();
		}else{
			if(isActive()){
				deactivate();
				action();
			}
		}
		return true;
	}

//    std::cout << (char)unicode<<":"<<mod<<","<<sym<<"\n";
	return false;
}

void Button::setText(const std::string & text){
	textLabel->setText(text);
}

std::string Button::getText()const{
	return textLabel->getText();
}

//! ---o
void Button::action(){
	//  try own action listener
	if(actionListener && actionListener(this, actionName)) {
		return;
	}

	// then use the global listener
	getGUI().componentActionPerformed(this,actionName);
}

//! ---|> MouseMotionListener
listenerResult_t Button::onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent){
	const Geometry::Vec2 absPos(motionEvent.x, motionEvent.y);
	if(component==this && !hover && getAbsRect().contains(absPos) && !isLocked()){
		hover=true;
		invalidateRegion();
		getGUI().addMouseMotionListener(this);

		if(getFlag(HOVER_BUTTON)){
			select();
			action();
		}

		// markForRepaint()
		return LISTENER_EVENT_CONSUMED;
	}else if(component==nullptr && hover && (!coversAbsPosition(absPos))){
		hover=false;
		invalidateRegion();
		// markForRepaint()
		return LISTENER_EVENT_CONSUMED_AND_REMOVE_LISTENER;
	}
	return LISTENER_EVENT_NOT_CONSUMED;
}

void Button::setColor(const Util::Color4ub & color){ // deprecated!!!!!!
	addProperty(new ColorProperty(PROPERTY_TEXT_COLOR,color));
}

}
