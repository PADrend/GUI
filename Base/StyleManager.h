/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_PROPERTY_MANAGER_H
#define GUI_PROPERTY_MANAGER_H

#include "Fonts/AbstractFont.h"
#include "AbstractShape.h"
#include <Util/References.h>
#include <Util/Macros.h>

#include <Util/Serialization/Serialization.h>

#include <cstdint>
#include <stack>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Util {
namespace UI {
class Cursor;
}
}
namespace GUI{
typedef uint8_t propertyId_t;
typedef std::string propertyName_t;

class StyleManager{
	public:
		~StyleManager();

	// ----------------------------------------------------------------

	//!	@name Color
	// @{
	private:
		typedef std::vector<Util::Color4ub> colorStack_t; // actvive property colors; index 0 always contains the default property color
		typedef std::vector<colorStack_t> colorRegistry_t; // propertyTypeId -> colorStack_t
		colorRegistry_t colorRegistry;

		//! (internal) Increase the size of the registry and set the default entries.
		GUIAPI void initColors(size_t newSize);
	public:
		GUIAPI Util::Color4ub getColor(propertyId_t type)const;
		GUIAPI void pushColor(propertyId_t type,const Util::Color4ub & c);
		GUIAPI void popColor(propertyId_t type);
		GUIAPI void setDefaultColor(propertyId_t type,const Util::Color4ub & c);
	//	@}
	// ----------------------------------------------------------------

	//!	@name Font
	// @{
	private:
		typedef std::vector<Util::Reference<AbstractFont> > fontStack_t;
		typedef std::vector<fontStack_t> fontRegistry_t;
		fontRegistry_t fontRegistry;

		//! (internal) Increase the size of the registry and set the default entries.
		GUIAPI void initFonts(size_t newSize);
	public:
		GUIAPI AbstractFont * getDefaultFont(propertyId_t type)const;
		GUIAPI AbstractFont * getFont(propertyId_t type)const;
		GUIAPI void pushFont(propertyId_t type,AbstractFont * f);
		GUIAPI void popFont(propertyId_t type);
		GUIAPI void setDefaultFont(propertyId_t type,AbstractFont * f);
	//	@}

	// ----------------------------------------------------------------

	//! @name Mousecursor
	//  @{
	private:
		std::unordered_map<propertyName_t, std::shared_ptr<Util::UI::Cursor>> cursorRegistry;
		std::shared_ptr<Util::UI::Cursor> defaultMouseCursor;

	public:
		GUIAPI void setMouseCursor(propertyName_t name, std::shared_ptr<Util::UI::Cursor> cursor);
		GUIAPI void setDefaultMouseCursor(propertyName_t name);
		GUIAPI std::shared_ptr<Util::UI::Cursor> getMouseCursor(propertyName_t name) const;
		GUIAPI void removeMouseCursor(propertyName_t name);
	
	//  @}
	
	// ----------------------------------------------------------------
	

	//!	@name Shape
	// @{
	private:
		typedef std::vector<Util::Reference<AbstractShape> > shapeStack_t;
		typedef std::vector<shapeStack_t> shapeRegistry_t;
		shapeRegistry_t shapeRegistry;

		//! (internal) Increase the size of the registry and set the default entries.
		GUIAPI void initShapes(size_t newSize);
	public:
		//! \note always returns a valid Shape object. If no specific Shape is found a NullShape is returned.
		GUIAPI AbstractShape * getShape(propertyId_t type)const;
		GUIAPI void pushShape(propertyId_t type,AbstractShape * s);
		GUIAPI void popShape(propertyId_t type);
		GUIAPI void setDefaultShape(propertyId_t type,AbstractShape * f);
	//	@}

	// ----------------------------------------------------------------

	//!	@name (global) Value
	// @{
	private:
		typedef std::vector<float> valueRegistry_t;
		valueRegistry_t valueRegistry;
	public:
		GUIAPI float getGlobalValue(propertyId_t type)const; // if the value is not defined, 0 is returned.
		GUIAPI void setGlobalValue(propertyId_t type,float v);
	//	@}


};

}
#endif // GUI_PROPERTY_MANAGER_H
