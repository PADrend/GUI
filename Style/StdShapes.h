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
		RectShape(const Util::Color4ub & _bgColor, const Util::Color4ub & _lineColor, bool _blend) :
				bgColor(_bgColor),lineColor(_lineColor),blend(_blend)	{}
		virtual ~RectShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new RectShape(*this);	}

		Util::Color4ub bgColor;
		Util::Color4ub lineColor;
		bool blend;
};

/***
 **	Rect3dShape ---|> AbstractShape
 **/
class Rect3dShape : public AbstractShape{
	public:
		Rect3dShape(const Util::Color4ub & _bgColor1, const Util::Color4ub & _bgColor2,bool _blend,bool _invert=false) :
				bgColor1(_bgColor1),bgColor2(_bgColor2),blend(_blend),invert(_invert){}
		virtual ~Rect3dShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new Rect3dShape(*this);	}

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
		ShadowedRectShape(const Util::Color4ub & _bgColor, const Util::Color4ub & _lineColor, bool _blend) :
				bgColor(_bgColor),lineColor(_lineColor),blend(_blend)	{}
		virtual ~ShadowedRectShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new ShadowedRectShape(*this);	}

		Util::Color4ub bgColor;
		Util::Color4ub lineColor;
		bool blend;
};

/***
 **	Rounded3dRectShape ---|> AbstractShape
 **/
class Rounded3dRectShape : public AbstractShape{
	public:
		Rounded3dRectShape(const Util::Color4ub & _bgColor1, const Util::Color4ub & _bgColor2,bool _blend,
							float _roundnessTL=2,float _roundnessTR=2,float _roundnessBL=2,float _roundnessBR=2) :
				bgColor1(_bgColor1),bgColor2(_bgColor2),blend(_blend),
				roundnessTL(_roundnessTL),roundnessTR(_roundnessTR),roundnessBL(_roundnessBL),roundnessBR(_roundnessBR){}
		virtual ~Rounded3dRectShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new Rounded3dRectShape(*this);	}

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
		ResizerShape(const Util::Color4ub & _color,bool _blend) :
				color(_color),blend(_blend){}
		virtual ~ResizerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new ResizerShape(*this);	}

		Util::Color4ub color;
		bool blend;
};

/***
 **	TriangleSelectorShape ---|> AbstractShape
 **/
class TriangleSelectorShape : public AbstractShape{
	public:
		TriangleSelectorShape(const Util::Color4ub & _color) :
				color(_color)	{}
		virtual ~TriangleSelectorShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new TriangleSelectorShape(*this);	}

		Util::Color4ub color;
};

/***
 **	ScrollableMarkerShape ---|> AbstractShape
 **/
class ScrollableMarkerShape : public AbstractShape{
	public:
		ScrollableMarkerShape(const Util::Color4ub & _colorTop,const Util::Color4ub & _colorRight,
								const Util::Color4ub & _colorBottom,const Util::Color4ub & _colorLeft,float _width) :
				colorTop(_colorTop),colorRight(_colorRight),colorBottom(_colorBottom),colorLeft(_colorLeft),width(_width)	{}
		virtual ~ScrollableMarkerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new ScrollableMarkerShape(*this);	}

		Util::Color4ub colorTop,colorRight,colorBottom,colorLeft;
		float width;
};

/***
 **	SliderMarkerShape ---|> AbstractShape
 **/
class SliderMarkerShape : public AbstractShape{
	public:
		SliderMarkerShape(const Util::Color4ub & c1, const Util::Color4ub & c2) :
				color1(c1),color2(c2){}
		virtual ~SliderMarkerShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new SliderMarkerShape(*this);	}

		Util::Color4ub color1;
		Util::Color4ub color2;
};

/***
 **	TabHeaderShape ---|> AbstractShape
 **/
class TabHeaderShape : public AbstractShape{
	public:
		TabHeaderShape(const Util::Color4ub & c1, const Util::Color4ub & c2, const Util::Color4ub & c3)
				: color1(c1),color2(c2),color3(c3){}
		virtual ~TabHeaderShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new TabHeaderShape(*this);	}

		Util::Color4ub color1;
		Util::Color4ub color2;
		Util::Color4ub color3;
};

/***
 **	GridShape ---|> AbstractShape
 **/
class GridShape : public AbstractShape{
	public:
		GridShape(const Util::Color4ub & _majorColor,const Util::Color4ub & _minorColor,float _hDistance,float _vDistance,int _hNumMinors=0,int _vNumMinors=0) :
				majorColor(_majorColor),minorColor(_minorColor),hDistance(_hDistance),vDistance(_vDistance),hNumMinors(_hNumMinors),vNumMinors(_vNumMinors)	{}
		virtual ~GridShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new GridShape(*this);	}

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
		CrossShape(const Util::Color4ub & _color1,const Util::Color4ub & _color2,float _lineWidth) :
				color1(_color1),color2(_color2),lineWidth(_lineWidth)	{}
		virtual ~CrossShape()	{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect & rect,flag_t flag);
		virtual AbstractShape * clone()		{	return new CrossShape(*this);	}

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
		StraightLineShape(const Util::Color4ub & _color,float _lineWidth) : 
				color(_color),lineWidth(_lineWidth){}
		virtual ~StraightLineShape()	{}

		// ---|> AbstractShape
		virtual StraightLineShape * clone()		{	return new StraightLineShape(color,lineWidth);	}
		
		// ---|> AbstractLineShape
		virtual void displayLine(const std::vector<Geometry::Vec2> & points,flag_t flag);
		
		Util::Color4ub color;
		float lineWidth;
};


/***
 **	SmoothConnectorShape ---|> AbstractLineShape
 **/
class SmoothConnectorShape : public AbstractLineShape{
	public:
		SmoothConnectorShape(const Util::Color4ub & _color,float _lineWidth) : 
				color(_color),lineWidth(_lineWidth){}
		virtual ~SmoothConnectorShape()	{}

		// ---|> AbstractShape
		virtual SmoothConnectorShape * clone()		{	return new SmoothConnectorShape(color,lineWidth);	}
		
		// ---|> AbstractLineShape
		virtual void displayLine(const std::vector<Geometry::Vec2> & points,flag_t flag);
		
		Util::Color4ub color;
		float lineWidth;
};

}

#endif // GUI_STDSHAPES_H
