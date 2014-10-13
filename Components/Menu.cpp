/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
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
Menu::Menu(GUI_Manager & _gui, flag_t _flags) :
	Container(_gui, _flags),
	keyListener(createKeyListener(_gui, this, &Menu::onKeyEvent)),
	mouseButtonListener(createMouseButtonListener(_gui, this, &Menu::onMouseButton)) {
	disable();
	setFlag(ALWAYS_ON_TOP,true);
	addProperty(new UseColorProperty(PROPERTY_TEXT_COLOR,PROPERTY_MENU_TEXT_COLOR));
    setMouseCursorProperty(PROPERTY_MOUSECURSOR_COMPONENTS);
}

//! (dtor)
Menu::~Menu() {
	if(optionalFrameListener) {
		getGUI().removeFrameListener(std::move(*optionalFrameListener.get()));
		optionalFrameListener.reset();
	}
}

//! ---|> Component
void Menu::doLayout() {
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
	enableLocalDisplayProperties();
	displayDefaultShapes();		
	getGUI().displayShape(PROPERTY_MENU_SHAPE,getLocalRect());
	disableLocalDisplayProperties();

	displayChildren(region,true);
}

static void onFrame(Menu & menu, double /*timeSecs*/) {
	// check if the menu should be closed
	if(menu.getGUI().getActiveComponent() != &menu && menu.hasParent()) {
		bool menuFound = false;
		for(Component * c = menu.getNext(); c; c = c->getNext()) {
			if(!c->isEnabled() || !c->getAbsRect().intersects(menu.getAbsRect())) {
				continue;
			}
			// something other than a menu in front of this menu? -> close this
			if(dynamic_cast<Menu*>(c) == nullptr) {
				menu.close();
				return;
			}
			menuFound = true;
			break;
		}
		// this is the topmost menu, but it is not selected or active -> close this
		if(!menu.isSelected() && !menuFound) {
			menu.close();
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
	optionalFrameListener.reset(new FrameListenerHandle(getGUI().addFrameListener(std::bind(&onFrame, std::ref(*this), std::placeholders::_1))));
}

void Menu::close(){
	if(optionalFrameListener) {
		getGUI().removeFrameListener(std::move(*optionalFrameListener.get()));
		optionalFrameListener.reset();
	}
	disable();
//    getGUI().removeMouseMotionListener(this);
	if( getFlag(ONE_TIME_MENU)){
		getGUI().markForRemoval(this);
	}
}


//! ---|> MouseButtonListener
bool Menu::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & /*buttonEvent*/){
	return true;
}

bool Menu::onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if (keyEvent.pressed && keyEvent.key == Util::UI::KEY_ESCAPE) {
		unselect();
		return true;
	}
	return false;
}

}
