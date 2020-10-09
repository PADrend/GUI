/*
	This file is part of the GUI library.
	Copyright (C) 2014 Claudius Jähn <claudius@uni-paderborn.de>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_COMPONENT_TOOLTIP_H
#define GUI_COMPONENT_TOOLTIP_H

#include <string>

namespace GUI {
class Component;

GUIAPI bool hasComponentTooltip(const Component& c);
GUIAPI std::string getComponentTooltip(const Component& c);
GUIAPI void setComponentTooltip(Component& c, std::string s);
GUIAPI void removeComponentTooltip(Component& c);

}
#endif // GUI_COMPONENT_TOOLTIP_H
