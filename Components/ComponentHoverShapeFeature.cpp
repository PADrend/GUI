/*
	This file is part of the GUI library.
	Copyright (C) 2014 Claudius Jähn <claudius@uni-paderborn.de>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "ComponentHoverShapeFeature.h"
#include "Component.h"
#include "Container.h"
#include "../Base/AbstractProperty.h"
#include "../GUI_Manager.h"
#include "../Base/ListenerHelper.h"
#include <Util/GenericAttribute.h>
#include <Util/UI/Event.h>
#include <iostream>

using namespace GUI;
typedef Util::ReferenceAttribute<GUI::DisplayProperty>  Attr_t;
static const Util::StringIdentifier attrName_hoverProperty("_HOVER_SHAPE");

bool GUI::hasComponentHoverProperty(const GUI::Component& c){
	return getComponentHoverProperty(c);
}
GUI::DisplayProperty* GUI::getComponentHoverProperty(const GUI::Component& c){
	const Attr_t* attr = dynamic_cast<const Attr_t*>(c.getAttribute(attrName_hoverProperty));
	return attr?attr->get() : nullptr;
}
void GUI::setComponentHoverProperty(GUI::Component& c, GUI::DisplayProperty* s){
	if(s)
		c.setAttribute( attrName_hoverProperty, new Attr_t(s) );
	else
		GUI::removeComponentHoverProperty(c);
}
void GUI::removeComponentHoverProperty(Component& c){
	c.unsetAttribute( attrName_hoverProperty );
}

class GUI::HoverPropertyHandler {
		GUI_Manager & gui;
		MouseMotionListener mouseMotionListener;
		Util::Reference<Component> activeComponent;
		
	public:
		HoverPropertyHandler(GUI_Manager & _gui) :
			gui(_gui),
			mouseMotionListener(createMouseMotionListener(_gui,
				[this](Component *, const Util::UI::MotionEvent & motionEvent) {
					Component *c;
					for(c=gui.getComponentAtPos(Geometry::Vec2(motionEvent.x, motionEvent.y)); c; c=c->getParent()){
						if( hasComponentHoverProperty(*c)){
							break;
						}
					}
					setActiveComponent( c );
//					std::cout << c<< " "<<motionEvent.x<<"\n";
					return false;
				})) {
		}
		void setActiveComponent(Component* c){
			if(activeComponent!=c){
				if(activeComponent)
					activeComponent->removeProperty(  getComponentHoverProperty(*activeComponent.get()) );
				activeComponent = c;
				if(activeComponent){
					auto * p = getComponentHoverProperty(*activeComponent.get());
					if(p)
						activeComponent->addProperty( p );
				}
			}
		}
};

void GUI::initHoverPropertyHandler(GUI_Manager& gui,Component& objHolder){
	// create Handler and store it as attribute of the objHolder
	objHolder.setAttribute(Util::StringIdentifier("_HoverPropertyHandler"),
							new Util::WrapperAttribute<std::unique_ptr<HoverPropertyHandler>>(new HoverPropertyHandler(gui)) );
}
