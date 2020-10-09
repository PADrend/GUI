/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_FLOW_LAYOUT_H
#define GUI_FLOW_LAYOUT_H

#include "AbstractLayouter.h"

namespace GUI {

class FlowLayouter : public AbstractLayouter{
	PROVIDES_TYPE_NAME(FlowLayouter)

		int padding;
		int margin;
		
		typedef uint8_t flag_t;
		static const flag_t FLAG_MAXIMIZE = 1<<0;
		static const flag_t FLAG_AUTO_BREAK = 1<<1;
		
		flag_t flags;
	public:
		FlowLayouter() : AbstractLayouter(),padding(0),margin(0),flags(0){}
		FlowLayouter(const FlowLayouter & ) = default;
		virtual ~FlowLayouter(){}
	
		bool getAutoBreak()const	{	return flags&FLAG_AUTO_BREAK;	}
		bool getMaximize()const		{	return flags&FLAG_MAXIMIZE;	}
		int getMargin()const		{	return margin;	}
		int getPadding()const		{	return padding;	}
		
		void setAutoBreak(bool b)	{	b ? flags|=FLAG_AUTO_BREAK : flags &= ~FLAG_AUTO_BREAK;	}

		/*! If true, the size of the layouted panel is set to the maximum of the content's size and the parent's size. This is 
			a special case used e.g. in Panels. Try to avoid this!	*/
		void setMaximize(bool b)	{	b ? flags|=FLAG_MAXIMIZE : flags &= ~FLAG_MAXIMIZE;	}
		void setMargin(int i)		{	margin = i;		}
		void setPadding(int i)		{	padding = i;	}
			
		//! ---|> AbstractLayouter
		GUIAPI virtual void layout(Util::WeakPointer<Component> component) override;

};


}
#endif // GUI_FLOW_LAYOUT_H
