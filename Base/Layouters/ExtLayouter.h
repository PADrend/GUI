/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_EXTERNAL_LAYOUTER_H
#define GUI_EXTERNAL_LAYOUTER_H

#include "AbstractLayouter.h"
#include <Geometry/Vec2.h>

namespace GUI {
	
class ExtLayouter : public AbstractLayouter{
	PROVIDES_TYPE_NAME(ExtLayouter)


	/*!	@name External Layout	(EXPERIMENTAL)
		Layout functions for sophisticated automatic positioning
		and resizing of a Components according to the size of its parent.
		Examples:
			// component always 2px away from lower right corner:
			component->setExtLayout(
					POS_X_ABS|REFERENCE_X_RIGHT|ALIGN_X_RIGHT|
					POS_Y_ABS|REFERENCE_Y_BOTTOM|ALIGN_Y_BOTTOM,
					Vec2(2.0,2.0) );
			// component centered, width is 20px smaller than parent's and height is 50%:
			component->setExtLayout(
					POS_X_ABS|REFERENCE_X_CENTER|ALIGN_X_CENTER|
					POS_Y_ABS|REFERENCE_Y_CENTER|ALIGN_Y_CENTER|
					WIDTH_ABS|HEIGHT_REL,
					Vec2(0.0,0.0), Vec(-20,0.5) );
	*/
	public:
		
		ExtLayouter() : AbstractLayouter(),flags(0){}
		ExtLayouter(const ExtLayouter & ) = default;
		virtual ~ExtLayouter(){}
		
		typedef uint32_t extLayoutFlags_t;

		//! Refernce point of this component
		static const extLayoutFlags_t REFERENCE_X_LEFT 		= 1<<0;
		static const extLayoutFlags_t REFERENCE_X_CENTER 	= 1<<1;
		static const extLayoutFlags_t REFERENCE_X_RIGHT 	= 1<<2;
		static const extLayoutFlags_t REFERENCE_Y_TOP 		= 1<<3;
		static const extLayoutFlags_t REFERENCE_Y_CENTER 	= 1<<4;
		static const extLayoutFlags_t REFERENCE_Y_BOTTOM 	= 1<<5;

		//! Alignment inside of the parent
		static const extLayoutFlags_t ALIGN_X_RIGHT 		= 1<<6;
		static const extLayoutFlags_t ALIGN_X_CENTER 		= 1<<7;
		static const extLayoutFlags_t ALIGN_X_LEFT 			= 1<<8;
		static const extLayoutFlags_t ALIGN_Y_TOP 			= 1<<9;
		static const extLayoutFlags_t ALIGN_Y_CENTER		= 1<<10;
		static const extLayoutFlags_t ALIGN_Y_BOTTOM 		= 1<<11;

		/*! Type of position-value
			 - ABS : pixel, REL : relative (1.0 is 100%)
			 - If not set for an axis, the original value is not changed.	*/
		static const extLayoutFlags_t POS_X_ABS 			= 1<<12;
		static const extLayoutFlags_t POS_X_REL 			= 1<<13;
		static const extLayoutFlags_t POS_Y_ABS 			= 1<<14;
		static const extLayoutFlags_t POS_Y_REL 			= 1<<15;

		/*! Type of size-value
			 - If not set for an axis, the original value is not changed.
			 - ABS : pixel, REL : relative (1.0 is 100%)
			 - CHILDREN referes to the children's size; FILL fills up to the next component (or the end)
			 - For WIDTH_ABS and HEIHGT_ABS: If size-value < 0, the value is subtracted from
				the corrseponding size of the parent.	*/
		static const extLayoutFlags_t WIDTH_REL 			= 1<<16;
		static const extLayoutFlags_t HEIGHT_REL 			= 1<<17;
		static const extLayoutFlags_t WIDTH_ABS 			= 1<<18;
		static const extLayoutFlags_t HEIGHT_ABS 			= 1<<19;
		static const extLayoutFlags_t WIDTH_CHILDREN_REL 	= 1<<20;
		static const extLayoutFlags_t HEIGHT_CHILDREN_REL 	= 1<<21;
		static const extLayoutFlags_t WIDTH_CHILDREN_ABS	= 1<<22;
		static const extLayoutFlags_t HEIGHT_CHILDREN_ABS	= 1<<23;
		static const extLayoutFlags_t WIDTH_FILL_REL 		= 1<<24;
		static const extLayoutFlags_t HEIGHT_FILL_REL 		= 1<<25;
		static const extLayoutFlags_t WIDTH_FILL_ABS 		= 1<<26;
		static const extLayoutFlags_t HEIGHT_FILL_ABS 		= 1<<27;

		const Geometry::Vec2 & getPosition()const	{	return extPos;	}
		const Geometry::Vec2 & getSize()const		{	return extSize;	}
		extLayoutFlags_t getFlags()const			{	return flags;	}

		void setPosition(const Geometry::Vec2 & v)	{	extPos	= v;	}
		void setSize(const Geometry::Vec2 & v)		{	extSize	= v;	}
		void setFlags(extLayoutFlags_t f)			{	flags = f;	}

		//! ---|> AbstractLayouter
		GUIAPI virtual void layout(Util::WeakPointer<Component> component) override;
	private:
		Geometry::Vec2 extPos;
		Geometry::Vec2 extSize;
		extLayoutFlags_t flags;
};

}
#endif // GUI_EXTERNAL_LAYOUTER_H
