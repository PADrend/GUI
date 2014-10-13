/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_STDSHAPES_H
#define GUI_STDSHAPES_H

#include "../Base/AbstractShape.h"
#include <Util/Graphics/Color.h>

namespace GUI {


/***
 **	RectShape ---|> AbstractShape
 **/
class RectShape : public AbstractShape{
	public:
		RectShape(Util::Color4ub _bgColor, Util::Color4ub _lineColor, bool _blend) :
				bgColor(std::move(_bgColor)),lineColor(std::move(_lineColor)),blend(_blend)	{}
		virtual ~RectShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new RectShape(*this);	}

		Util::Color4ub bgColor;
		Util::Color4ub lineColor;
		bool blend;
};

/***
 **	Rect3dShape ---|> AbstractShape
 **/
class Rect3dShape : public AbstractShape{
	public:
		Rect3dShape(Util::Color4ub _bgColor1, Util::Color4ub _bgColor2,bool _blend,bool _invert=false) :
				bgColor1(std::move(_bgColor1)),bgColor2(std::move(_bgColor2)),blend(_blend),invert(_invert){}
		virtual ~Rect3dShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new Rect3dShape(*this);	}

		Util::Color4ub bgColor1;
		Util::Color4ub bgColor2;
		bool blend;
		bool invert;
};

/***
 **	ShadowedRectShape ---|> AbstractShape
 **/
class ShadowedRectShape : public AbstractShape{
	public:
		ShadowedRectShape(Util::Color4ub _bgColor, Util::Color4ub _lineColor, bool _blend) :
				bgColor(std::move(_bgColor)),lineColor(std::move(_lineColor)),blend(_blend)	{}
		virtual ~ShadowedRectShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new ShadowedRectShape(*this);	}

		Util::Color4ub bgColor;
		Util::Color4ub lineColor;
		bool blend;
};

/***
 **	Rounded3dRectShape ---|> AbstractShape
 **/
class Rounded3dRectShape : public AbstractShape{
	public:
		Rounded3dRectShape(Util::Color4ub _bgColor1, Util::Color4ub _bgColor2,bool _blend,
							float _roundnessTL=2,float _roundnessTR=2,float _roundnessBL=2,float _roundnessBR=2) :
				bgColor1(std::move(_bgColor1)),bgColor2(std::move(_bgColor2)),blend(_blend),
				roundnessTL(_roundnessTL),roundnessTR(_roundnessTR),roundnessBL(_roundnessBL),roundnessBR(_roundnessBR){}
		virtual ~Rounded3dRectShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new Rounded3dRectShape(*this);	}

		Util::Color4ub bgColor1;
		Util::Color4ub bgColor2;
		bool blend;
		float roundnessTL,roundnessTR,roundnessBL,roundnessBR;
};

/***
 **	ResizerShape ---|> AbstractShape
 **/
class ResizerShape : public AbstractShape{
	public:
		ResizerShape(Util::Color4ub _color,bool _blend) :
				color(std::move(_color)),blend(_blend){}
		virtual ~ResizerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new ResizerShape(*this);	}

		Util::Color4ub color;
		bool blend;
};

/***
 **	TriangleAtCornerShape ---|> AbstractShape
 **/
class TriangleAtCornerShape : public AbstractShape{
	public:
		TriangleAtCornerShape(Util::Color4ub _color,float _size) :
				color(std::move(_color)),size(_size)	{}
		virtual ~TriangleAtCornerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new TriangleAtCornerShape(*this);	}

		Util::Color4ub color;
		float size;
};

/***
 **	TriangleSelectorShape ---|> AbstractShape
 **/
class TriangleSelectorShape : public AbstractShape{
	public:
		TriangleSelectorShape(Util::Color4ub _color) :
				color(std::move(_color))	{}
		virtual ~TriangleSelectorShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new TriangleSelectorShape(*this);	}

		Util::Color4ub color;
};

/***
 **	ScrollableMarkerShape ---|> AbstractShape
 **/
class ScrollableMarkerShape : public AbstractShape{
	public:
		ScrollableMarkerShape(Util::Color4ub _colorTop,Util::Color4ub _colorRight,
								Util::Color4ub _colorBottom,Util::Color4ub _colorLeft,float _width) :
				colorTop(std::move(_colorTop)),colorRight(std::move(_colorRight)),colorBottom(std::move(_colorBottom)),colorLeft(std::move(_colorLeft)),width(_width)	{}
		virtual ~ScrollableMarkerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new ScrollableMarkerShape(*this);	}

		Util::Color4ub colorTop,colorRight,colorBottom,colorLeft;
		float width;
};

/***
 **	SliderMarkerShape ---|> AbstractShape
 **/
class SliderMarkerShape : public AbstractShape{
	public:
		SliderMarkerShape(Util::Color4ub c1, Util::Color4ub c2) :
				color1(std::move(c1)),color2(std::move(c2)){}
		virtual ~SliderMarkerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new SliderMarkerShape(*this);	}

		Util::Color4ub color1;
		Util::Color4ub color2;
};

/***
 **	TabHeaderShape ---|> AbstractShape
 **/
class TabHeaderShape : public AbstractShape{
	public:
		TabHeaderShape(Util::Color4ub c1, Util::Color4ub c2, Util::Color4ub c3)
				: color1(std::move(c1)),color2(std::move(c2)),color3(std::move(c3)){}
		virtual ~TabHeaderShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new TabHeaderShape(*this);	}

		Util::Color4ub color1;
		Util::Color4ub color2;
		Util::Color4ub color3;
};

/***
 **	GridShape ---|> AbstractShape
 **/
class GridShape : public AbstractShape{
	public:
		GridShape(Util::Color4ub _majorColor,Util::Color4ub _minorColor,float _hDistance,float _vDistance,int _hNumMinors=0,int _vNumMinors=0) :
				majorColor(std::move(_majorColor)),minorColor(std::move(_minorColor)),hDistance(_hDistance),vDistance(_vDistance),hNumMinors(_hNumMinors),vNumMinors(_vNumMinors)	{}
		virtual ~GridShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new GridShape(*this);	}

		Util::Color4ub majorColor;
		Util::Color4ub minorColor;
		float hDistance;
		float vDistance;
		int hNumMinors;
		int vNumMinors;
};

/***
 **	CrossShape ---|> AbstractShape
 **/
class CrossShape : public AbstractShape{
	public:
		CrossShape(Util::Color4ub _color1,Util::Color4ub _color2,float _lineWidth) :
				color1(std::move(_color1)),color2(std::move(_color2)),lineWidth(_lineWidth)	{}
		virtual ~CrossShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag) override;
		virtual AbstractShape * clone() override		{	return new CrossShape(*this);	}

		Util::Color4ub color1;
		Util::Color4ub color2;
		float lineWidth;
};

// -----------------------------
// line shapes

/***
 **	StraightLineShape ---|> AbstractLineShape
 **/
class StraightLineShape : public AbstractLineShape{
	public:
		StraightLineShape(Util::Color4ub _color,float _lineWidth) : 
				color(std::move(_color)),lineWidth(_lineWidth){}
		virtual ~StraightLineShape()	{}

		// ---|> AbstractShape
		virtual StraightLineShape * clone() override		{	return new StraightLineShape(color,lineWidth);	}
		
		// ---|> AbstractLineShape
		virtual void displayLine(const std::vector<Geometry::Vec2> & points,flag_t flag) override;
		
		Util::Color4ub color;
		float lineWidth;
};


/***
 **	SmoothConnectorShape ---|> AbstractLineShape
 **/
class SmoothConnectorShape : public AbstractLineShape{
	public:
		SmoothConnectorShape(Util::Color4ub _color,float _lineWidth) : 
				color(std::move(_color)),lineWidth(_lineWidth){}
		virtual ~SmoothConnectorShape()	{}

		// ---|> AbstractShape
		virtual SmoothConnectorShape * clone() override		{	return new SmoothConnectorShape(color,lineWidth);	}
		
		// ---|> AbstractLineShape
		virtual void displayLine(const std::vector<Geometry::Vec2> & points,flag_t flag) override;
		
		Util::Color4ub color;
		float lineWidth;
};

}

#endif // GUI_STDSHAPES_H
