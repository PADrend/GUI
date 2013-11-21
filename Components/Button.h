/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_Button_H
#define GUI_Button_H

#include "Container.h"
#include "Label.h"
#include "../Base/Listener.h"
#include "../GUI_Manager.h"

namespace GUI {
/***
 **     Button ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class Button : public Container, public MouseMotionListener, public MouseButtonListener, public MouseClickListener {
		PROVIDES_TYPE_NAME(Button)
	public:
		static const flag_t FLAT_BUTTON=1<<24;
		static const flag_t HOVER_BUTTON=1<<25;
		static const Util::StringIdentifier ACTION_Button_click;
				
		Button(GUI_Manager & gui,flag_t flags=0);
		virtual ~Button();

		void setText(const std::string & text);
		std::string getText()const;

		Util::StringIdentifier getActionName()const 	{	return actionName;	}
		bool isSwitchedOn()const 						{	return switchedOn;	}
		void setActionName(const Util::StringIdentifier & n) 	{	actionName = n;	}
		void setSwitch(bool b) 							{	switchedOn=b;	}
		void setFont(AbstractFont * newFont)			{   textLabel->setFont(newFont);	}
		void setActionListener(HandleActionFun fun) {
			actionListener = std::move(fun);
		}
		void setTextStyle(unsigned int style)			{   textLabel->setTextStyle(style);	}
		void setColor(const Util::Color4ub & newColor);//	{   textColor=newColor;	}

		// ---o
		virtual void action();

		// ---|> MouseMotionListener
		virtual listenerResult_t onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent) override;
		// ---|> MouseButtonListener
		virtual listenerResult_t onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent) override;
		// ---|> MouseClickListener
		virtual bool onMouseClick(Component * component, unsigned int button,const Geometry::Vec2 &pos) override;

	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);

	protected:
		Util::WeakPointer<Label> textLabel;
		Util::StringIdentifier actionName;
		bool switchedOn;
		bool hover;
		HandleActionFun actionListener;
		GUI_Manager::KeyListenerHandle keyListenerHandle;
};
}
#endif // GUI_Button_H
