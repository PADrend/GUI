/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_STYLE_H
#define GUI_STYLE_H

#include <Util/Graphics/Color.h>
#include <Util/References.h>

#include <cstdint>
#include <stack>
#include <vector>

namespace GUI{
class AbstractShape;
class AbstractFont;
class StyleManager;
typedef uint8_t propertyId_t;

class Style{
	public:

	// ----------------------------------------------------------------

		// shapes
		static AbstractShape * createButtonShape(float _roundnessTL=2,float _roundnessTR=2,float _roundnessBL=2,float _roundnessBR=2);

		static AbstractShape * getButtonShape();

		static void initStyleManager(StyleManager & m);
};

}
#endif // GUI_COLOR_H
