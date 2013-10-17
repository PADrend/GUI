/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_DRAW_H
#define GUI_DRAW_H

#include <Geometry/Vec2.h>
#include <Geometry/Rect.h>
#include <Util/Graphics/Color.h>

namespace Util {
class PixelFormat;
}
namespace GUI {

class AbstractFont;

class Draw {
	public:
		// general
		static void beginDrawing(const Geometry::Vec2i & screenSize);
		static void endDrawing();
		static void moveCursor(const Geometry::Vec2i & pos);
		static Geometry::Rect_i queryViewport();
		static void setScissor(const Geometry::Rect_i & rect);
		static void resetScissor();
		static void clearScreen(const Util::Color4ub & color);

		// text
		static const unsigned int TEXT_ALIGN_LEFT=1<<0;
		static const unsigned int TEXT_ALIGN_RIGHT=1<<1;
		static const unsigned int TEXT_ALIGN_CENTER=1<<2;
		static const unsigned int TEXT_ALIGN_MIDDLE=1<<3;

		static void drawText(const std::string & text, const Geometry::Vec2 pos,
								AbstractFont * font, const Util::Color4ub & c);
										
		static void drawText(const std::string & text, const Geometry::Rect & r, AbstractFont * font,
										const Util::Color4ub & c,unsigned int style = TEXT_ALIGN_LEFT | TEXT_ALIGN_MIDDLE);
										
		static float getTextWidth(const std::string & text, AbstractFont * font);
		static Geometry::Vec2 getTextSize(const std::string & text, AbstractFont * font);


		// draw
		static void drawCross(const Geometry::Rect & r, const Util::Color4ub & c, float lineWidth = 4.0);

		static void draw3DRect(const Geometry::Rect & r, bool down,	const Util::Color4ub & bgColor1, const Util::Color4ub & bgColor2);

		static void drawFilledRect(const Geometry::Rect & r, const Util::Color4ub & bgColor, bool blend = true);
		static void drawFilledRect(const Geometry::Rect & r, const Util::Color4ub & bgColorTL, const Util::Color4ub & bgColorBL,
									const Util::Color4ub & bgColorBR, const Util::Color4ub & bgColorTR, bool blend = true);
		static void drawLineRect(const Geometry::Rect & r, const Util::Color4ub & lineColor, bool blend = true);

		static void drawTab(const Geometry::Rect & r, const Util::Color4ub & lineColor,const Util::Color4ub & bgColor1, const Util::Color4ub & bgColor2);
		static void dropShadow(const Geometry::Rect & r);

		static void drawTexturedRect(const Geometry::Rect_i & screenRect, const Geometry::Rect & uvRect, const Util::Color4ub & c, bool blend = true);

		//! @p posAndUV:  { x0,y0,u0,v0, x1,y1,u1,v1, x2,y2,u2,v2, ... }
		static void drawTexturedTriangles(const std::vector<float> & posAndUV, const Util::Color4ub & c, bool blend = true);

		//! @p vertices:  { x0,y0, x1,y1, x2,y2, ... } @p color {c0, c1, c2, ...}
		static void drawLine(const std::vector<float> & vertices,const std::vector<uint32_t> & colors, const float lineWidth = 1.0,bool lineSmooth=false);

		//! @p vertices:  { x0a,y0a, x0b,y0b, x1a,y1a, x1a,x1b, ... } @p color {c0a, c0b, c1a, c1b, c2a, c2b ...}
		static void drawLines(const std::vector<float> & vertices,const std::vector<uint32_t> & colors, const float lineWidth = 1.0);

		//! @p vertices:  { x0,y0, x1,y1, x2,y2, ... } @p color {c0, c1, c2, ...}
		static void drawTriangleFan(const std::vector<float> & vertices,const std::vector<uint32_t> & colors);

		// textures
		static uint32_t generateTextureId();
		static void enableTexture(uint32_t textureId);
		static void disableTexture();
		static void destroyTexture(uint32_t textureId);
		static void uploadTexture(uint32_t textureId,uint32_t width,uint32_t height,const Util::PixelFormat & format, const uint8_t * data);
};

}
#endif // GUI_DRAW_H
