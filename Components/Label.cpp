/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Label.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "../Base/Properties.h"
#include "ComponentPropertyIds.h"
#include <sstream>
#include <iostream>

namespace GUI {

//! (ctor)
Label::Label(GUI_Manager & _gui,const std::string & _text,flag_t _flags/*=0*/):
	Component(_gui,_flags),textStyle(Draw::TEXT_ALIGN_LEFT){
	setText(_text);

	// guess the initial size using the default font
	setSize(Draw::getTextSize(_text,getGUI().getActiveFont(PROPERTY_DEFAULT_FONT)));

	//ctor
}

//! (ctor)
Label::Label(GUI_Manager & _gui,const Geometry::Rect & _r,const std::string & _text,flag_t _flags/*=0*/):
	Component(_gui,_r,_flags),textStyle(Draw::TEXT_ALIGN_LEFT){
	setText(_text);
	//ctor
}

//! (dtor)
Label::~Label(){
	//dtor
}

void Label::doDisplay(const Geometry::Rect & /*region*/){
	enableLocalDisplayProperties();
	displayDefaultShapes();	
	Draw::drawText(getText(),getLocalRect(),getGUI().getActiveFont(PROPERTY_DEFAULT_FONT),getGUI().getActiveColor( PROPERTY_TEXT_COLOR ),textStyle);
	if(getGUI().getDebugMode()>0)
		Draw::drawLineRect(getLocalRect(),Util::Color4ub(255,0,0,20));
	disableLocalDisplayProperties();
}

//! ---|> Component
void Label::doLayout(){
	if(getFlag(RECALCULATE_SIZE)){
		const Geometry::Vec2 newSize = Draw::getTextSize(getText(),getGUI().getActiveFont(PROPERTY_DEFAULT_FONT));
		setSize( textStyle==Draw::TEXT_ALIGN_LEFT ? newSize.x() : getWidth(), newSize.y() );
		setFlag(RECALCULATE_SIZE,false);
	}
}

void Label::setText(const std::string & newText){
	invalidateRegion();

	// if no layouter is set, the size may change
	if(text!=newText && !hasLayouter()){
		invalidateLayout();
		setFlag(RECALCULATE_SIZE,true);
	}
	text = newText;

}

void Label::setColor(const Util::Color4ub & newColor){
	addProperty(new ColorProperty(PROPERTY_TEXT_COLOR,newColor));
}

void Label::setFont(AbstractFont * newFont){
	addProperty(new FontProperty(PROPERTY_DEFAULT_FONT,newFont));
}

}
