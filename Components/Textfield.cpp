/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Textfield.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>
#include <Util/Timer.h>

namespace GUI {

//! (ctor)
Textfield::Textfield(GUI_Manager & _gui,const std::string & _text,std::string _dataName,flag_t _flags/*=0*/):
		Component(_gui,_flags),
		textRef(nullptr),
		selectionStart(0),selectionEnd(0),backupText(""),cursorPos(0),scrollPos(0),dataName(std::move(_dataName)),
		keyListenerHandle(_gui.addKeyListener(this, std::bind(&Textfield::onKeyEvent, 
															  this, 
															  std::placeholders::_1))),
		mouseButtonListenerHandle(_gui.addMouseButtonListener(this, std::bind(&Textfield::onMouseButton, 
																			  this, 
																			  std::placeholders::_1,
																			  std::placeholders::_2))),
		mouseMotionListenerHandle(_gui.addGlobalMouseMotionListener(std::bind(&Textfield::onMouseMove, 
																			  this, 
																			  std::placeholders::_1,
																			  std::placeholders::_2))),
		listenOnMouseMove(false),
		currentOptionIndex(-1) {
	fontReference = getGUI().getActiveFont(PROPERTY_DEFAULT_FONT);
	setText(_text);

	setFlag(SELECTABLE,true);
	setFlag(USE_SCISSOR,true);
    
	setMouseCursorProperty(PROPERTY_MOUSECURSOR_TEXTFIELD);
}

//! (dtor)
Textfield::~Textfield() {
	getGUI().removeGlobalMouseMotionListener(std::move(mouseMotionListenerHandle));
	getGUI().removeMouseButtonListener(this, std::move(mouseButtonListenerHandle));
	getGUI().removeKeyListener(this, std::move(keyListenerHandle));
}

//! ---|> Component
void Textfield::doDisplay(const Geometry::Rect & /*region*/) {
	getGUI().displayShape(PROPERTY_TEXTFIELD_SHAPE,getLocalRect());

	// update reference to actual font
	fontReference = getGUI().getActiveFont(PROPERTY_DEFAULT_FONT);

	if (isSelected()) {
		// draw text selection
		if (isTextSelected()) {
			Geometry::Vec2 c1=getCursorCoordinate(leftSelect());
			Geometry::Vec2 c2=getCursorCoordinate(rightSelect());

			Geometry::Rect r=getLocalRect();
			r.moveRel(c1.getX() ,2);
			r.setSize(c2.getX()-c1.getX(),getHeight()-4);
			getGUI().displayShape(PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE,r);
		}

		// draw cursor
		Geometry::Vec2 c=getCursorCoordinate(cursorPos);
		getGUI().displayShape(PROPERTY_TEXTFIELD_CURSOR_SHAPE,Geometry::Rect(c.getX(),2,1,getHeight()-4));

		// draw component selection
		Geometry::Rect rect=getLocalRect();
		rect.moveRel(Geometry::Vec2(-2,-2));
		rect.changeSize(4,4);
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,rect);

		if(getText().length() > 0){
			const float buttonSize = getGUI().getGlobalValue(PROPERTY_TEXTFIELD_CLEAR_BUTTON_SIZE);
			const Geometry::Rect eraseRect=Geometry::Rect(getLocalRect().getWidth()-buttonSize,0,buttonSize,buttonSize);
			getGUI().displayShape(PROPERTY_TEXTFIELD_CLEAR_TEXT_SHAPE,eraseRect);
		}

	}
	if (hasOptions()) {
		std::stringstream s;
		s<<"("<<(getCurrentOptionIndex()+1)<<"/"<<countOptions()<<")";
		Draw::drawText(s.str(),getLocalRect()+Geometry::Vec2(-2,0),
						fontReference.get(),
						getGUI().getActiveColor(PROPERTY_TEXTFIELD_OPTIONS_TEXT_COLOR),
						Draw::TEXT_ALIGN_RIGHT|Draw::TEXT_ALIGN_MIDDLE);//,fontType);
	}

	const Geometry::Vec2 textSize = Draw::getTextSize(getText(),fontReference.get());
	const Geometry::Vec2 textPos(getGUI().getGlobalValue(PROPERTY_TEXTFIELD_INDENTATION)+scrollPos,
								(getHeight()-textSize.getHeight())*0.5);

	Draw::drawText(getText(),textPos,
					fontReference.get(),getGUI().getActiveColor(PROPERTY_TEXTFIELD_TEXT_COLOR));

	if(scrollPos<0)
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_LEFT_SHAPE, getLocalRect(), 0);
	if(textSize.x()+scrollPos>getWidth())
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_RIGHT_SHAPE, getLocalRect(), 0);
					
}

void Textfield::setText(const std::string & newText) {
	invalidateRegion();

	if (textRef)
		*textRef=newText;
	else
		text=newText;

	// assure right cursor and scroll-pos
	scrollPos = 0;
	setCursorPos( this->cursorPos > static_cast<int>(newText.length()) ? newText.length() : cursorPos);
}

const std::string & Textfield::getText()const {
	if (textRef)
		return *textRef;
	else
		return text;
}

Geometry::Vec2 Textfield::getCursorCoordinate(int _cursorPos) {
	const std::string s=(_cursorPos>=static_cast<int>(getText().length()))?getText():getText().substr(0,_cursorPos);
	Geometry::Vec2 c=Draw::getTextSize(s,fontReference.get());
	c.setX(c.getX()+getGUI().getGlobalValue(PROPERTY_TEXTFIELD_INDENTATION)+scrollPos);
	return c;
}

int Textfield::getCursorPositionFromCoordinate(const Geometry::Vec2 & pos) {
	const float cPos = pos.getX()-getGUI().getGlobalValue(PROPERTY_TEXTFIELD_INDENTATION)-scrollPos;
	if(cPos<=0)
		return 0;

	for(size_t cursor=1;cursor<getText().length();++cursor) {
		const float textWidth = Draw::getTextSize(getText().substr(0,cursor),fontReference.get()).x();
		if(textWidth >= cPos-2) 
			return cursor;
	}
	return getText().length();
}

void Textfield::setCursorPos(int _cursorPos,bool _shift) {
	invalidateRegion();

	if (_cursorPos<0)_cursorPos=0;
	if (_cursorPos>static_cast<int>(getText().length()))_cursorPos=getText().size();
//    shift=isShiftKeyPressed();
	if (_shift) {
		if (selectionStart==-1) {
			selectionStart=cursorPos;
		}
		selectionEnd=_cursorPos;
	} else {
		selectionStart=selectionEnd=-1;
	}
	cursorPos=_cursorPos;

	Geometry::Vec2 cPos=getCursorCoordinate(cursorPos);
	if(cPos.x()>getWidth()-15)
		scrollPos -= cPos.x()-getWidth()+20;
	else if(cPos.x()<5)
		scrollPos -= cPos.x()-10;
	if(cursorPos==0 || scrollPos> 0)
		scrollPos=0;
//	scrollPos(0.0f),
}

bool Textfield::onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if(!keyEvent.pressed)
		return true;

	if(isLocked())
		return false;

	getGUI().enableKeyRepetition(keyEvent);

	const bool _shift = getGUI().isShiftPressed();
	if (keyEvent.key == Util::UI::KEY_A && getGUI().isCtrlPressed()) {
		setCursorPos(0);
		setCursorPos(getText().length(),true);
		return true;
	}
	else if (keyEvent.key == Util::UI::KEY_TAB) {
		return false;

	} else if (keyEvent.key == Util::UI::KEY_RIGHT ) {
		if (cursorPos<static_cast<int>(getText().length()))
			setCursorPos(cursorPos+1,_shift);
	} else if (keyEvent.key == Util::UI::KEY_LEFT) {
		if (cursorPos>0)
			setCursorPos(cursorPos-1,_shift);
	} else if (keyEvent.key == Util::UI::KEY_END) {
		setCursorPos(getText().length(),_shift);
	} else if (keyEvent.key == Util::UI::KEY_HOME) {
		setCursorPos(0,_shift);
	} else if (keyEvent.key == Util::UI::KEY_DELETE) {
		if (isTextSelected()) {
			if(selectionStart<cursorPos){
				const int newPos=cursorPos-selectionLength();
				eraseText(leftSelect(),selectionLength());
				setCursorPos(newPos);
			}else{
				eraseText(leftSelect(),selectionLength());
			}
		} else {
			eraseText(cursorPos,1);
		}
	} else if (keyEvent.key == Util::UI::KEY_BACKSPACE) {
		if (isTextSelected()) {
			if(selectionStart<cursorPos){
				const int newPos=cursorPos-selectionLength();
				eraseText(leftSelect(),selectionLength());
				setCursorPos(newPos);
			}else{
				eraseText(leftSelect(),selectionLength());
			}

		} else if (cursorPos>0) {
			cursorPos--;
			eraseText(cursorPos,1);
		}
	} else if (keyEvent.key == Util::UI::KEY_ESCAPE) {
		text=backupText;
		unselect();
	} else if (keyEvent.key == Util::UI::KEY_RETURN) { // return
		if(getGUI().isCtrlPressed()){
			addOption(getText());
			currentOptionIndex=countOptions()-1;
		}else{
			getGUI().selectNext(this);

			if(isSelected()) // this may happen, if this textfield is the last component so that no next Component can be selected.
				unselect();
			// \todo generate some kind of event?
//            unselect();
		}
	} else if (keyEvent.key == Util::UI::KEY_C && getGUI().isCtrlPressed()) {
		if (isTextSelected()) {
			getGUI().copyStringToClipboard(getText().substr(leftSelect(),selectionLength()));
		}
	} else if (keyEvent.key == Util::UI::KEY_X && getGUI().isCtrlPressed()) {
		if (isTextSelected()) {
			getGUI().copyStringToClipboard(getText().substr(leftSelect(),selectionLength()));
			eraseText(leftSelect(),selectionLength());
			setCursorPos(leftSelect());
		}
	} else if (keyEvent.key == Util::UI::KEY_V && getGUI().isCtrlPressed()) {

		if (isTextSelected()) {
			int l=leftSelect();
			eraseText(l,selectionLength());
			setCursorPos(l);
		}
		std::string s(getGUI().getStringFromClipboard());
		std::string t(getText());
		t.insert(cursorPos,s);
		setText(t);
		setCursorPos(cursorPos+s.length());
	}
	// does not work... No idea why...
//     else if (ke->getChar() == 'z' &&ke->getModifier()&KMOD_CTRL  ) {
//        text=backupText;
//        setCursorPos(0);
//    }
	else if (keyEvent.key == Util::UI::KEY_UP && hasOptions()) {
//        int i=getCurrentOptionIndex();
		if(getCurrentOptionIndex()>0)
			setCurrentOptionIndex(getCurrentOptionIndex()-1);
		else
			setCurrentOptionIndex(countOptions()-1);


	} else if (keyEvent.key == Util::UI::KEY_DOWN && hasOptions()) {
		if(getCurrentOptionIndex()<countOptions()-1)
			setCurrentOptionIndex(getCurrentOptionIndex()+1);
		else
			setCurrentOptionIndex(0);
	}
	else if (keyEvent.str[0] >= 32) {
		const bool autoCompletion = hasOptions() &&
				(!isTextSelected() || (selectionEnd==cursorPos && selectionStart==static_cast<int>(getText().length())));

		if (isTextSelected()) {
			int l = leftSelect();
			eraseText(l,selectionLength());
			setCursorPos(l);
			selectionStart=selectionEnd=-1;
		}
		std::string t(getText());
		t.insert(cursorPos, keyEvent.str, 1);
		setText(t);
		setCursorPos(cursorPos+1);

		// auto completion
		if(autoCompletion) {
			for(const auto & option : options) {
				if(option.compare(0, t.length(), t) == 0 && t.length() != option.length()) {
					setText(option);
					selectionStart = option.length();
					selectionEnd = cursorPos;
				}
			}


		}

	}
	return true;
}

//! ---|> MouseButtonListener
bool Textfield::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent){
	if (buttonEvent.pressed && !isLocked()) {
		const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
		if (isSelected() && buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			const float buttonSize = getGUI().getGlobalValue(PROPERTY_TEXTFIELD_CLEAR_BUTTON_SIZE);
			const Geometry::Rect clearRect(getLocalRect().getWidth()-buttonSize,0,buttonSize,buttonSize);
			if(clearRect.contains(localPos)){
				setText("");
				return true;
			}
		}
		select();
		if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			setCursorPos(getCursorPositionFromCoordinate(localPos), getGUI().isShiftPressed());
			listenOnMouseMove = true;
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP && hasOptions()) {
			setCurrentOptionIndex( (countOptions()+getCurrentOptionIndex()-1) % countOptions());
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN && hasOptions()) {
			setCurrentOptionIndex( (getCurrentOptionIndex()+1) % countOptions());
		}else{
			return false;
		}
		return true;
	}
	return false;

}

//! ---|> MouseMotionListener
listenerResult_t Textfield::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent){
	if(!listenOnMouseMove) {
		return LISTENER_EVENT_NOT_CONSUMED;
	}
	if (!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT) || isLocked()) {
		listenOnMouseMove = false;
		return LISTENER_EVENT_NOT_CONSUMED;
	}
	const Geometry::Vec2 localPos = Geometry::Vec2(motionEvent.x, motionEvent.y) - getAbsPosition();
	setCursorPos(getCursorPositionFromCoordinate(localPos), true);
	return LISTENER_EVENT_CONSUMED;
}


//! ---|> Component
bool Textfield::onSelect() {
	backupText=getText();
	return true;
}

//! ---|> Component
bool Textfield::onUnselect() {
	listenOnMouseMove = false;

	if (backupText!=getText()) { // TODO: use changed flag
		/* if componentDataChanged(...) issues a recursive call to onUnselect, 
			componentDataChanged(...) should be called only once: */
		backupText = getText(); 
		getGUI().componentDataChanged(this,dataName);
	}

	return true;
}


// ----------
// ---- Options


void Textfield::addOption(const std::string & option) {
	invalidateRegion();

	options.push_back(option);
	if (currentOptionIndex<0) {
		currentOptionIndex=0;
		setText(option);
	}
}


void Textfield::clearOptions() {
	options.clear();
	currentOptionIndex=-1;
}


std::string Textfield::getOption(int index) {
	if (index>=0 && index<countOptions()) {
		return options[index];
	}
	else return "";
}


void Textfield::setCurrentOptionIndex(int index){
	if (index<0 || index>=countOptions())
		return;
	std::string oldText=getText();
	int oldIndex=currentOptionIndex;
	currentOptionIndex=index;

	text=getOption(currentOptionIndex);
	if(cursorPos>=static_cast<int>(text.length())){
		setCursorPos(text.length());
	}
	if(oldText!=getText() || oldIndex!=index)
		getGUI().componentDataChanged(this,dataName);

	selectionStart=selectionEnd=-1;
}

}
