/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Properties.h"
#include "StyleManager.h"

namespace GUI{

//! ---|> AbstractProperty
void ColorProperty::doEnable(StyleManager & s){
	s.pushColor(getPropertyId(),getColor());
}
//! ---|> AbstractProperty
void ColorProperty::doDisable(StyleManager & s){
	s.popColor(getPropertyId());
}
// ---
// FontProperty

//! ---|> AbstractProperty
void FontProperty::doEnable(StyleManager & s){
	s.pushFont(getPropertyId(),getFont());
}
//! ---|> AbstractProperty
void FontProperty::doDisable(StyleManager & s){
	s.popFont(getPropertyId());
}

// ---
// UseColorProperty

//! ---|> AbstractProperty
void UseColorProperty::doEnable(StyleManager & s){
	s.pushColor(getPropertyId(),s.getColor(prop2));
}
//! ---|> AbstractProperty
void UseColorProperty::doDisable(StyleManager & s){
	s.popColor(getPropertyId());
}

// ---
// UseFontProperty

//! ---|> AbstractProperty
void UseFontProperty::doEnable(StyleManager & s){
	s.pushFont(getPropertyId(),s.getFont(prop2));
}
//! ---|> AbstractProperty
void UseFontProperty::doDisable(StyleManager & s){
	s.popFont(getPropertyId());
}

// ---
// UseShapeProperty

//! ---|> AbstractProperty
void UseShapeProperty::doEnable(StyleManager & s){
	s.pushShape(getPropertyId(),s.getShape(prop2));
}
//! ---|> AbstractProperty
void UseShapeProperty::doDisable(StyleManager & s){
	s.popShape(getPropertyId());
}

// ---
//ShapeProperty

//! ---|> AbstractProperty
void ShapeProperty::doEnable(StyleManager & s){
	s.pushShape(getPropertyId(),getShape());
}
//! ---|> AbstractProperty
void ShapeProperty::doDisable(StyleManager & s){
	s.popShape(getPropertyId());
}

}
