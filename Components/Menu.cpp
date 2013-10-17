/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Menu.h"
#include "../GUI_Manager.h"
#include "../Base/Properties.h"
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>

namespace GUI {

//! (ctor)
Menu::Menu(GUI_Manager & _gui,flag_t _flags/*=0*/)
	:Container(_gui,_flags),KeyListener(),MouseButtonListener(){
	init();
	//ctor
}

//! (ctor)
Menu::Menu(GUI_Manager & _gui,const Geometry::Rect & _r,flag_t _flags/*=0*/)
		:Container(_gui,_r,_flags),KeyListener(),MouseButtonListener(){
	init();
	//ctor
}

void Menu::init(){
	disable();
	addKeyListener(this);
	addMouseButtonListener(this);
	setFlag(ALWAYS_ON_TOP,true);
	addProperty(new UseColorProperty(PROPERTY_TEXT_COLOR,PROPERTY_MENU_TEXT_COLOR));
    setMouseCursorProperty(PROPERTY_MOUSECURSOR_COMPONENTS);
}

//! (dtor)
Menu::~Menu() {
	getGUI().removeFrameListener(this);
	//dtor
}

//! ---|> Component
void  Menu::doLayout() {
	float y=3;
	float x=0;
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext()){
		if(!c->isEnabled())
			continue;
		if(c->getWidth()>x)
			x=c->getWidth();
		c->setPosition(Geometry::Vec2(3,y));
		y+=c->getHeight()+2;
	}
	setSize(x+6,y+3);
}

//! ---|> Component
void Menu::doDisplay(const Geometry::Rect & region) {
	getGUI().displayShape(PROPERTY_MENU_SHAPE,getLocalRect());

	displayChildren(region,true);
}

//! ---|> FrameListener
void Menu::onFrame(float /*timeSecs*/){
	
	// check if the menu should be closed
	if( getGUI().getActiveComponent()!=this && hasParent()){
		bool menuFound = false;

		for(Component * c=getNext();c;c=c->getNext()){
			if( ! c->isEnabled() || !c->getAbsRect().intersects(getAbsRect()))
				continue;
			// something other than a menu in front of this menu? -> close this
			if(dynamic_cast<Menu*>(c) == nullptr ){
				close();
				return;
			}
			menuFound=true;
			break;
		}
		// this is the topmost menu, but it is not selected or active -> close this
		if(!isSelected() && !menuFound){
			close();
		}
	}

}
//! ---|> Component
bool Menu::onSelect(){
	getGUI().setActiveComponent(this);
	return true;
}
//
//! ---|> Component
bool Menu::onUnselect(){
	return true;
}

void Menu::open(const Geometry::Vec2 &pos){
	setPosition(pos);
	enable();
	getGUI().setActiveComponent(this);
	bringToFront();

	getGUI().selectFirst(this);
	getGUI().addFrameListener(this);
}




void Menu::close(){
	getGUI().removeFrameListener(this);
	disable();
//    getGUI().removeMouseMotionListener(this);
	if( getFlag(ONE_TIME_MENU)){
		getGUI().markForRemoval(this);
	}
}


//! ---|> MouseButtonListener
listenerResult_t Menu::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & /*buttonEvent*/){
	return LISTENER_EVENT_CONSUMED;
}

//! ---|> KeyListener
bool Menu::onKeyEvent(Component * /*component*/, const Util::UI::KeyboardEvent & keyEvent) {
	if (keyEvent.pressed && keyEvent.key == Util::UI::KEY_ESCAPE) {
		unselect();
		return true;
	}
	return false;
}

}
