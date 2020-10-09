/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include "Component.h"
#include "../Base/Fonts/AbstractFont.h"

namespace GUI{

/***
 **  Label ---|> Component
 **/
class Label: public Component	{
	PROVIDES_TYPE_NAME(Label)
	
		static const flag_t RECALCULATE_SIZE=1<<24;
	
	public:
		GUIAPI Label(GUI_Manager & gui,const std::string &text="",flag_t flags=0);
		GUIAPI Label(GUI_Manager & gui,const Geometry::Rect & r,const std::string &text="",flag_t flags=0);
		GUIAPI virtual ~Label();

		GUIAPI void setText(const std::string & newText);
		const std::string & getText()const				{	return text;	}

		void setTextStyle(unsigned int style)           {   textStyle=style;    }
		void setTextStyle(unsigned int style,bool b)    {   textStyle=b ? (textStyle|style) : (textStyle-(textStyle&style));    }
		GUIAPI void setColor(const Util::Color4ub & newColor);//  			{   color=newColor; } // deprecated!!!!!!!!!!!!!!!!
		GUIAPI void setFont(AbstractFont * newFont);

	private:
		// ---|> Component
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;
		GUIAPI virtual void doLayout() override;

		std::string text;
		unsigned int textStyle;

};

}

#endif // GUI_LABEL_H
