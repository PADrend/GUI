/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ABSTRACT_SHAPE_H
#define GUI_ABSTRACT_SHAPE_H

#include <Geometry/Rect.h>
#include <Util/ReferenceCounter.h>
#include <Util/References.h>
#include <Util/TypeNameMacro.h>
#include <cstdint>
#include <vector>


namespace GUI {

/***
 **	AbstractShape
 **/
class AbstractShape : public Util::ReferenceCounter<AbstractShape> {
		PROVIDES_TYPE_NAME(AbstractShape)

	public:
		typedef uint16_t flag_t;

		static const flag_t ACTIVE = 1<<0;

		AbstractShape() : Util::ReferenceCounter<AbstractShape>() {}
		virtual ~AbstractShape() {}

		// ---o
		virtual void display(const Geometry::Rect & rect,flag_t flag=0)=0;
		virtual AbstractShape * clone()=0;
};

/***
 **	AbstractLineShape
 **/
class AbstractLineShape : public AbstractShape {
		PROVIDES_TYPE_NAME(AbstractLineShape)
	public:

		AbstractLineShape() : AbstractShape() {}
		virtual ~AbstractLineShape() {}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect &, flag_t) override{}
		// ---o
		virtual void displayLine(const std::vector<Geometry::Vec2> & points,flag_t flag=0)=0;
};


/***
 **	NullShape ---|> AbstractShape
 ** Invisible singleton dummy Shape
 **/
class NullShape : public AbstractShape{
		NullShape() 								{}
	public:
		static NullShape * instance(){
			static Util::Reference<NullShape> s(new NullShape);
			return s.get();
		}

		virtual ~NullShape()						{}

		// ---|> AbstractShape
		virtual void display(const Geometry::Rect &,flag_t) override	{}
		virtual AbstractShape * clone() override				{	return this;	}
};

}
#endif // GUI_ABSTRACT_SHAPE_H
