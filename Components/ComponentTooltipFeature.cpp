/*
	This file is part of the GUI library.
	Copyright (C) 2014 Claudius Jähn <claudius@uni-paderborn.de>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "ComponentTooltipFeature.h"
#include "Component.h"
#include <Util/GenericAttribute.h>

static const Util::StringIdentifier attrName_tooltip("_TOOLTIP");

bool GUI::hasComponentTooltip(const Component& c){
	return c.getAttribute(attrName_tooltip);
}
std::string GUI::getComponentTooltip(const Component& c){
	auto attr = c.getAttribute(attrName_tooltip);
	return attr ? attr->toString() : "";
}
void GUI::setComponentTooltip(Component& c, std::string s){
	c.setAttribute( attrName_tooltip,Util::GenericAttribute::createString(std::move(s)) );
}
void GUI::removeComponentTooltip(Component& c){
	c.unsetAttribute( attrName_tooltip );
}

