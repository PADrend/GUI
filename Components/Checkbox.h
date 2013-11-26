/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_Checkbox_H
#define GUI_Checkbox_H

#include "Container.h"
#include "Label.h"
#include "../Base/ListenerHelper.h"

namespace GUI {
/***
 **     Checkbox ---|> Component
 **
 **/
class Checkbox : public Container {
		PROVIDES_TYPE_NAME(Checkbox)
	public:
		Checkbox(GUI_Manager & gui,bool checked=false,const std::string & text="",flag_t flags=0);
		virtual ~Checkbox();

		void setChecked(bool b);
		bool isChecked()const;

		void setValueRef(bool * boolRef);
		void setValueRef(unsigned int * intValueRef,unsigned int intBitMask);

		void setText(const std::string & text);
		std::string getText()const;

		void setFont(AbstractFont * newFont) 				{	textLabel->setFont(newFont);	}
		void setTextStyle(unsigned int style) 				{	textLabel->setTextStyle(style);	}

		// ---o
		virtual void action();

		// ---|> Component
		virtual void doLayout() override;

	private:
		Util::WeakPointer<Label> textLabel;
		bool * boolValueRef;
		unsigned int * intValueRef;
		unsigned int intBitMask;
		bool value;

		KeyListener keyListener;
		MouseButtonListener mouseButtonListener;
		MouseClickListener mouseClickListener;

		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
};
}
#endif // GUI_Checkbox_H
