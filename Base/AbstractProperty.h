/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ABSTRACT_PROPERTY_H
#define GUI_ABSTRACT_PROPERTY_H

#include <Util/ReferenceCounter.h>
#include <Util/TypeNameMacro.h>

#include <string>

namespace GUI{
typedef uint8_t propertyId_t;
typedef std::string propertyName_t;
class StyleManager;


//! DisplayProperty
class DisplayProperty : public Util::ReferenceCounter<DisplayProperty> {
		PROVIDES_TYPE_NAME(DisplayProperty)

		propertyId_t propertyId;
	public:
		DisplayProperty(propertyId_t _propertyId) : propertyId(_propertyId) {}
		virtual ~DisplayProperty() {}

		propertyId_t getPropertyId()const			{	return propertyId;	}

		void enable(StyleManager & s)		{	doEnable(s);	}
		void disable(StyleManager & s)		{	doDisable(s);	}

		//! ---o
		virtual void doEnable(StyleManager & s)=0;
		virtual void doDisable(StyleManager & s)=0;
};

}

#endif // GUI_ABSTRACT_PROPERTY_H
