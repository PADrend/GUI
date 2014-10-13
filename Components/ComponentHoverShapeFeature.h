/*
	This file is part of the GUI library.
	Copyright (C) 2014 Claudius Jähn <claudius@uni-paderborn.de>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_COMPONENT_HOVER_SHAPE_H
#define GUI_COMPONENT_HOVER_SHAPE_H

#include <string>

namespace GUI {
class Component;
class DisplayProperty;
class GUI_Manager;
class HoverPropertyHandler;

bool hasComponentHoverProperty(const Component& c);
DisplayProperty* getComponentHoverProperty(const Component& c);
void setComponentHoverProperty(Component& c, DisplayProperty* s);
void removeComponentHoverProperty(Component& c);

void initHoverPropertyHandler(GUI_Manager& gui,Component& objHolder);

}
#endif // GUI_COMPONENT_HOVER_SHAPE_H
