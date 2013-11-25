/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Slider.h"
#include "../GUI_Manager.h"
#include "../Base/AbstractShape.h"
#include "../Base/Draw.h"
#include "../Base/Listener.h"
#include "ComponentPropertyIds.h"
#include "Button.h"
#include <Util/UI/Event.h>

namespace GUI{

/***
 **  Slider ---|> Component
 **/
class SliderMarker:public Component {
	Slider & slider;
	MouseButtonListenerHandle mouseButtonListenerHandle;
	MouseMotionListenerHandle mouseMotionListenerHandle;

	public:
	bool listenOnMouseMove;
	//! (ctor)
	SliderMarker(GUI_Manager & _gui,Slider & _slider) :
			Component(_gui),
			slider(_slider),
			mouseButtonListenerHandle(_gui.addMouseButtonListener(this, std::bind(&SliderMarker::onMouseButton, 
																				  this, 
																				  std::placeholders::_1,
																				  std::placeholders::_2))),
			mouseMotionListenerHandle(_gui.addGlobalMouseMotionListener(std::bind(&SliderMarker::onMouseMove, 
																				  this, 
																				  std::placeholders::_1,
																				  std::placeholders::_2))),
			listenOnMouseMove(false) {
	}
	//! (ctor)
	virtual ~SliderMarker() {
		getGUI().removeGlobalMouseMotionListener(std::move(mouseMotionListenerHandle));
		getGUI().removeMouseButtonListener(this, std::move(mouseButtonListenerHandle));
	}

	bool onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
		if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP || buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
			return false;
		}
		if (buttonEvent.pressed) {
			select();
			if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
				getGUI().setActiveComponent(this);
				listenOnMouseMove = true;
			}
		}
		return true;
	}

	bool onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
		if(!listenOnMouseMove) {
			return false;
		}
		if (!isActive() || motionEvent.buttonMask == Util::UI::MASK_NO_BUTTON) {
			listenOnMouseMove = false;
			return false;
		}
		const Geometry::Vec2 localPos = Geometry::Vec2(motionEvent.x, motionEvent.y) - slider.getAbsPosition();
		slider.updateDataFromPos(localPos);
		return true;
	}

	//! ---|> Component
	void doDisplay(const Geometry::Rect & /*region*/) override {
		getGUI().displayShape(PROPERTY_SLIDER_MARKER_SHAPE,getLocalRect(),isActive() ? AbstractShape::ACTIVE : 0);
	}
};

// ----------------------------------------------------

//! (ctor)
Slider::Slider(GUI_Manager & _gui,const Geometry::Rect & _r,float left,float right,int steps,Util::StringIdentifier _dataName,flag_t _flags/*=0*/):
		Container(_gui,_r,_flags),
		markerSize(6),value(0),floatValueRef(nullptr),dataName(std::move(_dataName)),
		keyListenerHandle(_gui.addKeyListener(this, std::bind(&Slider::onKeyEvent, 
															  this, 
															  std::placeholders::_1))),
		mouseButtonListenerHandle(_gui.addMouseButtonListener(this, std::bind(&Slider::onMouseButton, 
																			  this, 
																			  std::placeholders::_1,
																			  std::placeholders::_2))) {
	setRange(left,right,steps);
	setFlag(SELECTABLE,true);

	sliderMarker=new SliderMarker(getGUI(),*this);
	_addChild(sliderMarker.get());

	if(getFlag(SLIDER_BUTTONS)){
		button1=new Button(getGUI());
		button1->setText("<");
		button1->setActionListener([this](Component *, const Util::StringIdentifier &) {
										updateData(getValue() - stepWidth);
										return true;
									});
		_addChild(button1.get());

		button2=new Button(getGUI());
		button2->setText(">");
		button2->setActionListener([this](Component *, const Util::StringIdentifier &) {
										updateData(getValue() + stepWidth);
										return true;
									});
		_addChild(button2.get());
	}
}

//! (dtor)
Slider::~Slider() {
	getGUI().removeMouseButtonListener(this, std::move(mouseButtonListenerHandle));
	getGUI().removeKeyListener(this, std::move(keyListenerHandle));
}

//! ---|> Component
void Slider::doLayout(){

	if(getFlag(SLIDER_BUTTONS)){
		const float buttonSize = getGUI().getGlobalValue( PROPERTY_SLIDER_BUTTON_SIZE );
		button1->setRect(Geometry::Rect(0,0,buttonSize,buttonSize));
		button2->setRect(Geometry::Rect(getWidth()-buttonSize,0,buttonSize,getHeight()));
	}

	const float pos=getPosFromValue(getValue());
	sliderMarker->setPosition(Geometry::Vec2(pos,0));
	sliderMarker->setSize(getMarkerSize(),getHeight());
}

//! ---|> Component
void Slider::doDisplay(const Geometry::Rect & region) {
	if(stepWidth==0 || numSteps==0 ){
		return;
	}
	const float buttonSize = getGUI().getGlobalValue( PROPERTY_SLIDER_BUTTON_SIZE );


	// show marker
	const float pos=getPosFromValue(getValue());

	// show bar
	// left side
	Geometry::Rect rect=getLocalRect();
	rect.moveRel(getFlag(SLIDER_BUTTONS)?buttonSize:0 ,	getHeight()*0.3);
	rect.setSize( pos , getHeight()*0.3);
	getGUI().displayShape(PROPERTY_SLIDER_BAR_SHAPE,rect);

	if (isSelected()) {
		rect.moveRel(0,-3);
		rect.changeSize(0,6);
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,rect);
	}

	// right side
	rect=getLocalRect();
	rect.moveRel( (getFlag(SLIDER_BUTTONS)?buttonSize:0) + pos + getMarkerSize(),getHeight()*0.3);

	rect.setSize( getWidth() - (getFlag(SLIDER_BUTTONS)?-(2.0*buttonSize):0) - getMarkerSize() - pos , getHeight()*0.3);
	getGUI().displayShape(PROPERTY_SLIDER_BAR_SHAPE,rect);

	if (isSelected()) {
		rect.moveRel(0,-3);
		rect.changeSize(0,6);
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,rect);
	}

	// show null-bar
	rect=getLocalRect();
	rect.moveRel(0,getHeight()*0.3);
	float nullPos=getPosFromValue(0.0f);
	if(getFlag(SLIDER_BUTTONS)){
		if(nullPos<buttonSize)
			nullPos=buttonSize;
		if(nullPos>=getWidth()-buttonSize)
			nullPos=getWidth()-buttonSize;
	}else{
		if(nullPos<0)
			nullPos=0;
		if(nullPos>=getWidth())
			nullPos=getWidth();
	}
	if(nullPos<pos){
		getGUI().displayShape(PROPERTY_SLIDER_ZERO_BAR_SHAPE,Geometry::Rect(nullPos,rect.getY()+2,pos-nullPos,1));
	}else{
		getGUI().displayShape(PROPERTY_SLIDER_ZERO_BAR_SHAPE,Geometry::Rect(pos,rect.getY()+2,nullPos-pos,1));
	}


	// show Value
	if(getFlag(SHOW_VALUE)){
		std::ostringstream s;
		s<<getValue();
		Draw::drawText(s.str(),getLocalRect(),
					getGUI().getActiveFont(PROPERTY_DEFAULT_FONT),getGUI().getActiveColor(PROPERTY_TEXT_COLOR),Draw::TEXT_ALIGN_CENTER);
	}

	displayChildren(region);
}

//! ---|> MouseButtonListener
bool Slider::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if (buttonEvent.pressed && !isLocked()) {
		select();
		if (buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
//            std::cout <<"\n SLIDER: x:" <<pos.getX()<<", "<<getValueFromPos(pos.getX())<<", "<<getPosFromValue(getValueFromPos(pos.getX()))<<"\n";
			const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
			updateDataFromPos(localPos);
			sliderMarker->activate();
			SliderMarker * s=dynamic_cast<SliderMarker *>(sliderMarker.get());
			s->listenOnMouseMove = true;
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP) {
			updateData(getValue()+stepWidth);
		}else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
			updateData(getValue()-stepWidth);
		}
	}
	return true;

}

bool Slider::onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if(isLocked())
		return false;
	if (keyEvent.key==Util::UI::KEY_LEFT) {
		if (keyEvent.pressed)
			updateData(getValue()-stepWidth);
		return true;
	} else if (keyEvent.key==Util::UI::KEY_RIGHT) {
		if (keyEvent.pressed)
			updateData(getValue()+stepWidth);
		return true;
	} else if (keyEvent.key==Util::UI::KEY_HOME) {
		if (keyEvent.pressed)
			updateData(rangeLeft);
		return true;
	} else if (keyEvent.key==Util::UI::KEY_END) {
		if (keyEvent.pressed)
			updateData(rangeRight);
		return true;
	}
	return false;
}

float Slider::getValue()const {
	if (floatValueRef)
		return *floatValueRef;
	else
		return value;
}

void Slider::setValue(float f) {
	invalidateRegion();
	invalidateLayout();
	if (floatValueRef)
		(*floatValueRef)=f;
	else
		value=f;
}

void Slider::setValueRef(float * _valueRef) {
	floatValueRef=_valueRef;
}

void Slider::setRange(float _left,float _right,int _steps) {
	rangeLeft=_left;
	rangeRight=_right;
	numSteps=_steps;
	stepWidth=numSteps>0?(rangeRight-rangeLeft)/static_cast<float>(numSteps):0;
	invalidateLayout();
}

//! ---o
float Slider::getPosFromValue(float _value)const {
	const float buttonSize = getGUI().getGlobalValue( PROPERTY_SLIDER_BUTTON_SIZE );
	const float w=getWidth() - (getFlag(SLIDER_BUTTONS)? 2*buttonSize : 0) - getMarkerSize();
	if (stepWidth==0 || numSteps==0 ) return 0.0;

	float v=(_value-rangeLeft) / stepWidth;
	float p=w/numSteps * v;
//    std::cout << v;
	if (p<0) p=0;
	if (p>w) p=w;
	return (w/static_cast<float>(numSteps))*v+(getFlag(SLIDER_BUTTONS)? buttonSize:0);
}

//! ---o
float Slider::getValueFromPos(float pos)const {
	const float buttonSize = getGUI().getGlobalValue( PROPERTY_SLIDER_BUTTON_SIZE );
	const float w=getWidth()- (getFlag(SLIDER_BUTTONS)? 2*buttonSize : 0)- getMarkerSize();
	pos-=getFlag(SLIDER_BUTTONS)?buttonSize:0;

	if (pos>w) pos=w;
	if (pos<0) pos=0;
	if (pos==w) return rangeRight;
	else if (pos==0) return rangeLeft;

	float v=rangeLeft+ (rangeRight-rangeLeft)*pos/w;
	if (stepWidth) {
		v=ceilf(v/stepWidth)*stepWidth;
	}
	return v;
}

void Slider::updateDataFromPos(const Geometry::Vec2 & p) {
	setValue(getValueFromPos( p.getX() - getMarkerSize()/2 ));
//    std::cout << " " <<p.getX();
	dataUpdated();
}

void Slider::updateData(float f) {
	if (rangeRight>rangeLeft) {
		if (f<rangeLeft) f=rangeLeft;
		if (f>rangeRight) f=rangeRight;
	}
	else {
		if (f>rangeLeft) f=rangeLeft;
		if (f<rangeRight) f=rangeRight;
	}
	setValue(f);
	dataUpdated();
}

void Slider::setRelMarkerSize(const float relMarkerSize){
	int m = static_cast<int>(getWidth()*relMarkerSize);
	setMarkerSize(m>6?m:6);
}

//! ---o
void Slider::dataUpdated() {
	getGUI().componentDataChanged(this,dataName);
}

}
