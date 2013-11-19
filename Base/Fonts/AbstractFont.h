/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ABSTRACT_FONT_H
#define GUI_ABSTRACT_FONT_H

#include <string>
#include <Geometry/Vec2.h>
#include <Util/ReferenceCounter.h>
#include <Util/TypeNameMacro.h>
#include <Util/Graphics/Color.h>

namespace GUI {

/***
 **	AbstractFont
 **/
class AbstractFont : public Util::ReferenceCounter<AbstractFont> {
		PROVIDES_TYPE_NAME(AbstractFont)

	public:
		AbstractFont(uint32_t _lineHeight=1) : Util::ReferenceCounter<AbstractFont>(),lineHeight(_lineHeight) {}
		virtual ~AbstractFont() {}

		// ---o
		virtual void enable()	{	}
		virtual void disable()	{	}
		virtual void renderText( const Geometry::Vec2 & pos, const std::string & text, const Util::Color4ub & color )=0;
		virtual Geometry::Vec2 getRenderedTextSize( const std::string & text )=0;

		uint32_t getLineHeight()const				{	return lineHeight;	}

	private:
		uint32_t lineHeight;
	protected:
		void setLineHeight(uint32_t h)				{	lineHeight = h;	}
};
}
#endif // GUI_ABSTRACT_FONT_H
