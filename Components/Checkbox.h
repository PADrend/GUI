/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
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
#include "../Base/Listener.h"

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

		void setDataName(const Util::StringIdentifier & s) 	{	dataName = s;	}
		Util::StringIdentifier getDataName()const 			{	return dataName;	}

		void setFont(AbstractFont * newFont) 				{	textLabel->setFont(newFont);	}
		void setTextStyle(unsigned int style) 				{	textLabel->setTextStyle(style);	}

		// ---o
		virtual void action();

		// ---|> Component
		virtual void doLayout() override;

	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

		KeyListenerHandle keyListenerHandle;
		MouseButtonListenerHandle mouseButtonListenerHandle;
		MouseClickListenerHandle mouseClickListenerHandle;

	protected:
		Util::WeakPointer<Label> textLabel;
		bool * boolValueRef;
		unsigned int * intValueRef;
		unsigned int intBitMask;
		bool value;
		Util::StringIdentifier dataName;
};
}
#endif // GUI_Checkbox_H
