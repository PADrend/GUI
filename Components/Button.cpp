/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2014 Claudius Jähn <claudius@uni-paderborn.de>
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
#include "ComponentHoverPropertyFeature.h"
#include <Util/UI/Event.h>
#include <iostream>

namespace GUI {

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
Button::Button(GUI_Manager & _gui,flag_t _flags/*=0*/) :
		Container(_gui,_flags),
		switchedOn(false),
		actionListener(),
		keyListener(createKeyListener(_gui, this, &Button::onKeyEvent)),
		mouseButtonListener(createMouseButtonListener(_gui, this, &Button::onMouseButton)),
		mouseClickListener(createMouseClickListener(_gui,
													this,
													[this](Component *, unsigned int, const Geometry::Vec2 &) {
														if(!isLocked()) {
															action();
														}
														return true;
													})){
	setFlag(SELECTABLE,true);

	// create Label
	textLabel=new Label(getGUI(),"");
	textLabel->addLayouter(getDefaultLabelLayouter());
	textLabel->setTextStyle(Draw::TEXT_ALIGN_CENTER|Draw::TEXT_ALIGN_MIDDLE);
	addContent(textLabel.get());
	
	addComponentHoverProperty(*this,new UseColorProperty(PROPERTY_TEXT_COLOR,PROPERTY_BUTTON_HOVERED_TEXT_COLOR),1,true);
	addComponentHoverProperty(*this,new UseColorProperty(PROPERTY_ICON_COLOR,PROPERTY_BUTTON_HOVERED_TEXT_COLOR),1,true);

//	PROPERTY_TEXT_COLOR
}

//! (dtor)
Button::~Button() = default;

//! ---|> Component
void Button::doDisplay(const Geometry::Rect & region){

	enableLocalDisplayProperties();
	displayDefaultShapes();
	
	if( (!getFlag(FLAT_BUTTON)) || isSwitchedOn() || isActive()){
		getGUI().displayShape(PROPERTY_BUTTON_SHAPE,getLocalRect(),(isActive()||isSwitchedOn())?AbstractShape::ACTIVE : 0);
	}


	if (isActive()){
		disableLocalDisplayProperties();
		Draw::moveCursor(Geometry::Vec2(1,1));
		displayChildren(region);
		Draw::moveCursor(-Geometry::Vec2(1,1));
	}else{
		if(isSelected())
			getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,getLocalRect());

		if(isSwitchedOn()){
			Util::Reference<DisplayProperty> c = new ColorProperty(PROPERTY_TEXT_COLOR,getGUI().getActiveColor(PROPERTY_BUTTON_ENABLED_COLOR));
			Util::Reference<DisplayProperty> c2 = new ColorProperty(PROPERTY_ICON_COLOR,getGUI().getActiveColor(PROPERTY_BUTTON_ENABLED_COLOR));
			disableLocalDisplayProperties();
			getGUI().enableProperty(c);
			getGUI().enableProperty(c2);

			displayChildren(region);

			getGUI().disableProperty(c2);
			getGUI().disableProperty(c);
			
		}else{
			disableLocalDisplayProperties();
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
	if(!actionListener || !actionListener(this, Util::StringIdentifier())) 
		getGUI().componentActionPerformed(this, Util::StringIdentifier());	// then use the global listener

}


void Button::setColor(const Util::Color4ub & color){ // deprecated!!!!!!
	addProperty(new ColorProperty(PROPERTY_TEXT_COLOR,color));
}

}
