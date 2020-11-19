/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_PROPERTIES_H
#define GUI_PROPERTIES_H

#include "AbstractProperty.h"
#include "Fonts/AbstractFont.h"
#include "AbstractShape.h"

#include <Util/Graphics/Color.h>
#include <Util/References.h>

namespace GUI{

//! ColorProperty ---|> DisplayProperty
class ColorProperty : public DisplayProperty {
		PROVIDES_TYPE_NAME(ColorProperty)

		Util::Color4ub color;
	public:
		ColorProperty(propertyId_t _propertyId,Util::Color4ub c) : DisplayProperty(_propertyId),color(std::move(c)) {}
		virtual ~ColorProperty() {}

		const Util::Color4ub & getColor()const		{	return color;	}
		void setColor(const Util::Color4ub& c)		{	color=c;	}

		//! ---|> DisplayProperty
		GUIAPI virtual void doEnable(StyleManager & s) override;
		GUIAPI virtual void doDisable(StyleManager & s) override;
};

//! FontProperty ---|> DisplayProperty
class FontProperty : public DisplayProperty {
		PROVIDES_TYPE_NAME(FontProperty)

		Util::Reference<AbstractFont> font;
	public:
		FontProperty(propertyId_t _propertyId,AbstractFont * f) : DisplayProperty(_propertyId),font(f) {}
		virtual ~FontProperty() {}

		AbstractFont * getFont()const				{	return font.get();	}
		void setFont(AbstractFont * f)				{	font=f;	}

		//! ---|> DisplayProperty
		GUIAPI virtual void doEnable(StyleManager & s) override;
		GUIAPI virtual void doDisable(StyleManager & s) override;
};

/*! UseColorProperty ---|> DisplayProperty
	Apply the Color of another property (prop2). */
class UseColorProperty : public DisplayProperty {
		PROVIDES_TYPE_NAME(UseColorProperty)

		propertyId_t prop2;
	public:
		UseColorProperty(propertyId_t _propertyId, propertyId_t _prop2) : DisplayProperty(_propertyId),prop2(_prop2) {}
		virtual ~UseColorProperty() {}

		//! ---|> DisplayProperty
		GUIAPI virtual void doEnable(StyleManager & s) override;
		GUIAPI virtual void doDisable(StyleManager & s) override;
};

/*! UseFontProperty ---|> DisplayProperty
	Apply the FOnt of another property (prop2). */
class UseFontProperty : public DisplayProperty {
		PROVIDES_TYPE_NAME(UseFontProperty)

		propertyId_t prop2;
	public:
		UseFontProperty(propertyId_t _propertyId, propertyId_t _prop2) : DisplayProperty(_propertyId),prop2(_prop2) {}
		virtual ~UseFontProperty() {}

		//! ---|> DisplayProperty
		GUIAPI virtual void doEnable(StyleManager & s) override;
		GUIAPI virtual void doDisable(StyleManager & s) override;
};

/*! UseShapeProperty ---|> DisplayProperty
	Apply the Shape of another property (prop2). */
class UseShapeProperty : public DisplayProperty {
		PROVIDES_TYPE_NAME(UseShapeProperty)

		propertyId_t prop2;
	public:
		UseShapeProperty(propertyId_t _propertyId, propertyId_t _prop2) : DisplayProperty(_propertyId),prop2(_prop2) {}
		virtual ~UseShapeProperty() {}

		//! ---|> DisplayProperty
		GUIAPI virtual void doEnable(StyleManager & s) override;
		GUIAPI virtual void doDisable(StyleManager & s) override;
};

//! ShapeProperty ---|> DisplayProperty
class ShapeProperty : public DisplayProperty {
		PROVIDES_TYPE_NAME(ShapeProperty)

		Util::Reference<AbstractShape> shape;
	public:
		ShapeProperty(propertyId_t _propertyId,AbstractShape * s) : DisplayProperty(_propertyId),shape(s) {}
		virtual ~ShapeProperty() {}

		AbstractShape * getShape()const				{	return shape.get();	}
		void setShape(AbstractShape * s)			{	shape=s;	}

		//! ---|> DisplayProperty
		GUIAPI virtual void doEnable(StyleManager & s) override;
		GUIAPI virtual void doDisable(StyleManager & s) override;
};

}

#endif // GUI_PROPERTIES_H
