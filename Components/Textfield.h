/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_TEXTFIELD_H
#define GUI_TEXTFIELD_H

#include "Component.h"
#include "../Base/Listener.h"
#include "../Base/Fonts/AbstractFont.h"
#include "../GUI_Manager.h"
#include <array>

namespace GUI {

/***
 **  Textfield ---|> Component
 **/
class Textfield: public Component {
		PROVIDES_TYPE_NAME(Textfield)
	public:
		Textfield(GUI_Manager & gui,const std::string &text="",std::string dataName="",flag_t flags=0);
		virtual ~Textfield();

		void setTextRef(std::string * newTextRef) 		{	textRef=newTextRef;	}
		void setText(const std::string & newText);
		const std::string & getText()const;
//		void setFont(AbstractFont * newFont);

		// ---|> Component
		virtual bool onSelect() override;
		virtual bool onUnselect() override;
	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
		bool onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent);

		std::string text;
		std::string * textRef;
		Util::Reference<AbstractFont> fontReference; // this is updated by the actual font property on each call of display

		int selectionStart;
		int selectionEnd;
		std::string backupText;
		int cursorPos;
		int scrollPos;
		std::string dataName;

		GUI_Manager::KeyListenerHandle keyListenerHandle;
		GUI_Manager::MouseButtonListenerHandle mouseButtonListenerHandle;
		GUI_Manager::MouseMotionListenerHandle mouseMotionListenerHandle;
		bool listenOnMouseMove;

		Geometry::Vec2 getCursorCoordinate(int cursorPos);
		int getCursorPositionFromCoordinate(const Geometry::Vec2 & pos);
		void setCursorPos(int _cursorPos,bool shift=false);

		bool isTextSelected()const	{	return selectionStart!=selectionEnd;	}
		int leftSelect()const 		{	return selectionStart<selectionEnd?selectionStart:selectionEnd;	}
		int rightSelect()const 		{	return selectionStart>selectionEnd?selectionStart:selectionEnd;	}
		int selectionLength()const 	{	return rightSelect()-leftSelect();	}
		void eraseText(int from,int length) {
			std::string t=getText();
			t.erase(from,length);
			setText(t);
		}
		// ----------
		// ---- Options
	private:
		std::vector<std::string> options;
		int currentOptionIndex;
	public:
		void addOption(const std::string & option);
		std::string getOption(int index);
		void clearOptions();
		int getCurrentOptionIndex()const 	{	return currentOptionIndex;	}
		void setCurrentOptionIndex(int index);
		bool hasOptions()const 				{	return countOptions()>0;	}
		int countOptions()const 			{	return options.size();	}
};

}

#endif // GUI_TEXTFIELD_H
