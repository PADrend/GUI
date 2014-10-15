/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "StdShapes.h"
#include "../Base/Draw.h"
#include "../Base/BasicColors.h"
#include "Colors.h"
#include <Geometry/Interpolation.h>
#include <Util/References.h>
#include <iostream>

using namespace Geometry;

namespace GUI {

//! RectShape ---|> AbstractShape
void RectShape::display(const Rect & rect,flag_t /*flag*/){
	Draw::drawFilledRect(rect,bgColor,blend);
	Draw::drawLineRect(rect,lineColor,blend);
}

//! Rect3dShape ---|> AbstractShape
void Rect3dShape::display(const Rect & rect,flag_t flags){
	Draw::draw3DRect(rect, (flags&ACTIVE)^invert , bgColor1, bgColor2);
}

//! ScrollableMarkerShape ---|> AbstractShape
void ScrollableMarkerShape::display(const Rect & rect,flag_t /*flag*/){
	if(!colorTop.isTransparent()){
		Draw::drawFilledRect(Rect(rect.getMinX(),rect.getMinY(),rect.getWidth(),width),
						colorTop,Colors::NO_COLOR,Colors::NO_COLOR,colorTop);
	}
	else if(!colorRight.isTransparent()){
		Draw::drawFilledRect(Rect(rect.getMaxX()-width,rect.getMinY(),width,rect.getHeight()),
						Colors::NO_COLOR,Colors::NO_COLOR,colorRight,colorRight);
	}
	else if(!colorBottom.isTransparent()){
		Draw::drawFilledRect(Rect(rect.getMinX(),rect.getMaxY()-width,rect.getWidth(),width),
						Colors::NO_COLOR,colorBottom,colorBottom,Colors::NO_COLOR);
	}
	else if(!colorLeft.isTransparent()){
		Draw::drawFilledRect(Rect(rect.getMinX(),rect.getMinY(),width,rect.getHeight()),
						colorLeft,colorLeft,Colors::NO_COLOR,Colors::NO_COLOR);
	}

}
//! ShadowedRectShape ---|> AbstractShape
void ShadowedRectShape::display(const Rect & rect,flag_t /*flag*/){
	Draw::drawFilledRect(rect,bgColor,blend);
	Draw::drawLineRect(rect,lineColor,blend);
	Draw::dropShadow(rect);
}

//! SliderMarkerShape ---|> AbstractShape
void SliderMarkerShape::display(const Rect & rect,flag_t flags){
	if (flags&ACTIVE)
		Draw::draw3DRect(rect,true,color1,color2);
	else
		Draw::draw3DRect(rect,false,color1,color2);
	Vec2 c=rect.getCenter();
	Draw::draw3DRect(Rect(c.x()-1 , c.y()-1 ,2,2),false,color1,color2);
}

//! TabHeaderShape ---|> AbstractShape
void TabHeaderShape::display(const Rect & rect,flag_t flags){
	if (flags&ACTIVE)
		Draw::drawTab(rect,color1,color2,color3);
	else
		Draw::drawTab(rect,Colors::NO_COLOR,color2,color1);
}

//! Rounded3dRectShape ---|> AbstractShape
void Rounded3dRectShape::display(const Rect & rect,flag_t flags){
	const bool down=flags&ACTIVE;
	if (bgColor1 != Colors::NO_COLOR){
		std::vector<float> vertices;
		std::vector<uint32_t> colors;
		vertices.reserve(8*2);
		colors.reserve(8);

		colors.insert(colors.end(),4, (down ? bgColor2:bgColor1).getAsUInt());
		vertices.push_back(rect.getMaxX());				vertices.push_back(rect.getMinY()+roundnessTR);
		vertices.push_back(rect.getMaxX()-roundnessTR);	vertices.push_back(rect.getMinY());
		vertices.push_back(rect.getMinX()+roundnessTL);	vertices.push_back(rect.getMinY());
		vertices.push_back(rect.getMinX());				vertices.push_back(rect.getMinY()+roundnessTL);

		colors.insert(colors.end(),4,(down ? bgColor1:bgColor2).getAsUInt());
		vertices.push_back(rect.getMinX());				vertices.push_back(rect.getMaxY()-roundnessBL);
		vertices.push_back(rect.getMinX()+roundnessBL);	vertices.push_back(rect.getMaxY());
		vertices.push_back(rect.getMaxX()-roundnessBR);	vertices.push_back(rect.getMaxY());
		vertices.push_back(rect.getMaxX());				vertices.push_back(rect.getMaxY()-roundnessBR);

		Draw::drawTriangleFan(vertices,colors);
	}

	{	// draw border
		const Util::Color4ub c1 = down ? Colors::BRIGHT_COLOR : Colors::DARK_COLOR;
		const Util::Color4ub c2 = down ? Colors::DARK_COLOR   : Colors::BRIGHT_COLOR;

		std::vector<float> vertices;
		std::vector<uint32_t> colors;
		vertices.reserve(10*2);
		colors.reserve(10);

		const Geometry::Rect_i r2(rect);
		Geometry::Rect r3(r2);
		r3.moveRel(0.5f,0.5f);
	
		colors.insert(colors.end(),5,c1.getAsUInt());
		vertices.push_back(r3.getMinX());				vertices.push_back(r3.getMaxY()-roundnessBL);
		vertices.push_back(r3.getMinX()+roundnessBL);	vertices.push_back(r3.getMaxY());
		vertices.push_back(r3.getMaxX()-roundnessBR);	vertices.push_back(r3.getMaxY());
		vertices.push_back(r3.getMaxX());				vertices.push_back(r3.getMaxY()-roundnessBR);
		vertices.push_back(r3.getMaxX());				vertices.push_back(r3.getMinY()+roundnessTR);

		colors.insert(colors.end(),5,c2.getAsUInt());
		vertices.push_back(r3.getMaxX());				vertices.push_back(r3.getMinY()+roundnessTR);
		vertices.push_back(r3.getMaxX()-roundnessTR);	vertices.push_back(r3.getMinY());
		vertices.push_back(r3.getMinX()+roundnessTL);	vertices.push_back(r3.getMinY());
		vertices.push_back(r3.getMinX());				vertices.push_back(r3.getMinY()+roundnessTL);
		vertices.push_back(r3.getMinX());				vertices.push_back(r3.getMaxY()-roundnessBL);
		Draw::drawLine(vertices,colors,1.0);
	}
}
//
//! ResizerShape ---|> AbstractShape
void ResizerShape::display(const Rect & rect,flag_t /*flag*/){
	const float w=floorf(rect.getWidth()/4.0f);
	const float h=floorf(rect.getWidth()/4.0f);
	const float x=floorf(rect.getX());
	const float y=floorf(rect.getY());
	Draw::drawFilledRect(Rect(x + floorf(w*2.75f), y+ floorf(h*0.25f), w,h) ,color,blend);

	Draw::drawFilledRect(Rect(x + floorf(w*1.5f) , y+ floorf(h*1.5f), w,h) ,color,blend);
	Draw::drawFilledRect(Rect(x + floorf(w*2.75f), y+ floorf(h*1.5f), w,h) ,color,blend);

	Draw::drawFilledRect(Rect(x + floorf(w*0.25f), y+ floorf(h*2.75f), w,h) ,color,blend);
	Draw::drawFilledRect(Rect(x + floorf(w*1.5f) , y+ floorf(h*2.75f), w,h) ,color,blend);
	Draw::drawFilledRect(Rect(x + floorf(w*2.75f), y+ floorf(h*2.75f), w,h) ,color,blend);
}//

//! TriangleAtCornerShape ---|> AbstractShape
void TriangleAtCornerShape::display(const Rect & rect,flag_t/* flags*/){
	std::vector<float> vertices;
	std::vector<uint32_t> colors;
	vertices.reserve(3*2);
	colors.reserve(3);

	colors.insert(colors.end(),3, color.getAsUInt());

	const Geometry::Vec2 corner = rect.getCorner(CORNER_XY);
	vertices.push_back(corner.x());				vertices.push_back(corner.y());
	vertices.push_back(corner.x());				vertices.push_back(corner.y()-size);
	vertices.push_back(corner.x()-size);		vertices.push_back(corner.y());

	Draw::drawTriangleFan(vertices,colors);
}

//! TriangleSelectorShape ---|> AbstractShape
void TriangleSelectorShape::display(const Rect & rect,flag_t flags){
	std::vector<float> vertices;
	std::vector<uint32_t> colors;
	vertices.reserve(3*2);
	colors.reserve(3);

	colors.insert(colors.end(),3, color.getAsUInt());

	const float sideLength = std::min(rect.getWidth(),rect.getHeight());
	const float halfSideLength = sideLength*0.5;
	const float h = std::sqrt( sideLength*sideLength - halfSideLength*halfSideLength );
	const float halfH = h*0.5;
	const Geometry::Vec2 center = rect.getCenter();
	if(flags&ACTIVE){
		vertices.push_back(center.x()-halfSideLength);		vertices.push_back(center.y()-halfH);
		vertices.push_back(center.x());						vertices.push_back(center.y()+halfH);
		vertices.push_back(center.x()+halfSideLength);		vertices.push_back(center.y()-halfH);
	}else{
		vertices.push_back(center.x()-halfH);				vertices.push_back(center.y()-halfSideLength);
		vertices.push_back(center.x()-halfH);				vertices.push_back(center.y()+halfSideLength);
		vertices.push_back(center.x()+halfH);				vertices.push_back(center.y());
	}
	Draw::drawTriangleFan(vertices,colors);
}

//! GridShape ---|> AbstractShape
void GridShape::display(const Rect & rect,flag_t /*flag*/){
	std::vector<float> vertices;
	std::vector<uint32_t> colors;

	// horizontal lines
	if(hDistance>0){
		int count=0;
		for(float pos=0;pos<=rect.getHeight();pos+=hDistance){
			colors.insert(colors.end(),2,(((count++ % (hNumMinors+1)) ==0) ? majorColor : minorColor).getAsUInt());
			vertices.push_back(0);						vertices.push_back(pos);
			vertices.push_back(rect.getWidth());		vertices.push_back(pos);
		}
	}
	// vertical lines
	if(vDistance>0){
		int count=0;
		for(float pos=0;pos<=rect.getWidth();pos+=vDistance){
			colors.insert(colors.end(),2,(((count++ % (vNumMinors+1)) ==0) ? majorColor : minorColor).getAsUInt());
			vertices.push_back(pos);					vertices.push_back(0);
			vertices.push_back(pos);					vertices.push_back(rect.getHeight());
		}
	}
	Draw::drawLines(vertices,colors);
}

//! CrossShape ---|> AbstractShape
void CrossShape::display(const Rect & rect,flag_t flags){
	Draw::drawCross(rect,flags&ACTIVE ? color2 : color1,lineWidth);
}


//! StraightLineShape ---|> LineShape
void StraightLineShape::displayLine(const std::vector<Geometry::Vec2> & points,flag_t){
	std::vector<float> vertices;
	vertices.reserve(points.size()*2);
	for(auto & point : points){
		vertices.push_back(static_cast<int>(point.x())+0.5f);
		vertices.push_back(static_cast<int>(point.y())+0.5f);
	}

	std::vector<uint32_t> colors;
	colors.insert(colors.end(),vertices.size(),color.getAsUInt());
	Draw::drawLine(vertices,colors,lineWidth,true);
}

//! SmoothConnectorShape ---|> LineShape
void SmoothConnectorShape::displayLine(const std::vector<Geometry::Vec2> & points,flag_t){
	std::vector<float> vertices;
	for(size_t i=1;i<points.size();++i){
		const Geometry::Vec2 p0(points[i-1]); // startPoint
		const Geometry::Vec2 p3(points[i]); // endPoint
		const float d = std::min(p0.distance(p3)*0.4,100.0);
		const Geometry::Vec2 p1(p0+Geometry::Vec2(d,0));
		const Geometry::Vec2 p2(p3-Geometry::Vec2(d,0));

		vertices.push_back(p0.x());
		vertices.push_back(p0.y());
		for(float t=0.0;t<=1.00;t += t<0.5 ? ((t*t)+0.1)*0.1 : (( (1.0-t)*(1.0-t))+0.1)*0.1 ){
			const Geometry::Vec2 p_t = Interpolation::cubicBezier(p0, p1, p2, p3, t);
			vertices.push_back(p_t.x());
			vertices.push_back(p_t.y());
		}
	}
	std::vector<uint32_t> colors;
	colors.insert(colors.end(),vertices.size(),color.getAsUInt());
	Draw::drawLine(vertices,colors,lineWidth,true);
}


}
