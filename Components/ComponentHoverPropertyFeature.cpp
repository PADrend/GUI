/*
	This file is part of the GUI library.
	Copyright (C) 2014 Claudius Jähn <claudius@uni-paderborn.de>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "ComponentHoverPropertyFeature.h"
#include "Component.h"
#include "Container.h"
#include "../Base/AbstractProperty.h"
#include "../GUI_Manager.h"
#include "../Base/ListenerHelper.h"
#include <Util/GenericAttribute.h>
#include <Util/UI/Event.h>
#include <iostream>

using namespace GUI;


// [ (property, layer, recursive) ]
typedef Util::WrapperAttribute<std::vector<std::tuple<Util::Reference<GUI::DisplayProperty>,hoverPropertyLayer_t,bool>>>  HoverAttr_t;

static const Util::StringIdentifier attrName_hoverProperty("_HOVER_PROP");

static HoverAttr_t* getContainerAttribute(const GUI::Component& c){
	return dynamic_cast<HoverAttr_t*>(c.getAttribute(attrName_hoverProperty));
}

bool GUI::hasComponentHoverProperties(const GUI::Component& c){
	auto* attr = getContainerAttribute(c);
	return attr && !attr->get().empty();
}

void GUI::addComponentHoverProperty(GUI::Component& c, GUI::DisplayProperty* s,hoverPropertyLayer_t layer,bool recursive){
	HoverAttr_t* attr = getContainerAttribute(c);
	if(!attr){
		attr = new HoverAttr_t;
		c.setAttribute( attrName_hoverProperty, attr);
	}
	attr->ref().emplace_back(Util::Reference<DisplayProperty>(s),layer,recursive);
}

void GUI::clearComponentHoverProperties(Component& c){
	c.unsetAttribute( attrName_hoverProperty );
}

class GUI::HoverPropertyHandler {
		GUI_Manager & gui;
		MouseMotionListener mouseMotionListener;
//		Util::Reference<Component> activeComponent;
		std::vector<std::tuple<Util::Reference<Component>,DisplayProperty*,bool>>  undo; // [ (component,Property,recursive)* ]
		
	public:
		HoverPropertyHandler(GUI_Manager & _gui) :
			gui(_gui),
			mouseMotionListener(createMouseMotionListener(_gui,
				[this](Component *, const Util::UI::MotionEvent & motionEvent) {
					// undo 
					for(auto& entry: undo){
						if(std::get<2>(entry))
							std::get<0>(entry)->removeProperty(std::get<1>(entry));
						else
							std::get<0>(entry)->removeLocalProperty(std::get<1>(entry));
					}
					
					hoverPropertyLayer_t usedLayersMask = 0;
					
					for(Component* c=gui.getComponentAtPos(Geometry::Vec2(motionEvent.x, motionEvent.y)); c; c=c->getParent()){
						auto* attr = getContainerAttribute(*c);
						if(attr){
							const hoverPropertyLayer_t oldUsedLayersMask = usedLayersMask;
							// [ (property, layer, recursive) ]
							for(auto& entry:attr->ref()){
								auto pLayer = std::get<1>(entry);
								if((oldUsedLayersMask&pLayer) == 0){
									usedLayersMask |= pLayer;
									auto* property = std::get<0>(entry).get();
									bool recursive = std::get<2>(entry);
									if(recursive)
										c->addProperty(property);
									else
										c->addLocalProperty(property);
									undo.emplace_back( Util::Reference<Component>(c),property,recursive );
								}
							}
						}
					}
					return false;
				})){}
};

void GUI::initHoverPropertyHandler(GUI_Manager& gui,Component& objHolder){
	// create Handler and store it as attribute of the objHolder
	objHolder.setAttribute(Util::StringIdentifier("_HoverPropertyHandler"),
							new Util::WrapperAttribute<std::unique_ptr<HoverPropertyHandler>>(new HoverPropertyHandler(gui)) );
}
