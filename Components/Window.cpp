/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Window.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "../Base/AnimationHandler.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "../Base/Properties.h"
#include "ComponentPropertyIds.h"
#include "Button.h"
#include <Geometry/Interpolation.h>
#include <Util/UI/Event.h>
#include <iostream>

namespace GUI{

const Util::StringIdentifier Window::ACTION_onWindowClosed("onWindowClosed");

static const Util::StringIdentifier actionId_minimize("minimize");
static const Util::StringIdentifier actionId_close("close");
static const Util::StringIdentifier actionId_hide("hide");


//! TitlePanel ---|> Container, MouseMotionListener, MouseButtonListener
struct TitlePanel:public Container,public MouseMotionListener,public MouseButtonListener{
	Window & window;
	Geometry::Vec2 dragOffset;
	TitlePanel(Window & win):Container(win.getGUI()),window(win){
		addMouseButtonListener(this);
	}
	virtual ~TitlePanel(){
		getGUI().removeMouseMotionListener(this);
	}
	// ---|> MouseButtonListener
	listenerResult_t onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) override{
		if(!buttonEvent.pressed)
			return LISTENER_EVENT_NOT_CONSUMED;
		activate();
		getGUI().selectFirst(&window);
		if(buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
			getGUI().addMouseMotionListener(this);
			dragOffset = window.getPosition() - Geometry::Vec2(buttonEvent.x, buttonEvent.y);
		}
		return LISTENER_EVENT_CONSUMED;
	}
	 // ---|> MouseMotionListener
	listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override{
		if (!isActive() || !(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT)) {
			return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
		}
		const Geometry::Vec2 currentMousePos(motionEvent.x, motionEvent.y);
		window.setPosition(currentMousePos + dragOffset);
		return LISTENER_EVENT_CONSUMED;
	}
};

//! ResizePanel ---|> Component, MouseMotionListener, MouseButtonListener
struct ResizePanel:public Component,public MouseMotionListener,public MouseButtonListener{
	Window & window;
	int changeX, changeY;
	ResizePanel(Window & win):Component(win.getGUI()),window(win){
		addMouseButtonListener(this);
	}
	virtual ~ResizePanel(){
		getGUI().removeMouseMotionListener(this);
	}

	//! ---|> Component
	void doDisplay(const Geometry::Rect & /*region*/) override {
		getGUI().displayShape(PROPERTY_WINDOW_RESIZER_SHAPE,getLocalRect());

		if (isSelected()){
			getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,getLocalRect());
		}
	}
	// ---|> MouseMovementListner
	listenerResult_t onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) override{
		if(!buttonEvent.pressed)
			return LISTENER_EVENT_NOT_CONSUMED;
		startResizing(1,1);
		return LISTENER_EVENT_CONSUMED;
	}
	void startResizing(int x,int y){
		changeX = x;
		changeY = y;
		activate();
		select();
		getGUI().addMouseMotionListener(this);
	}
	
	// ---|> MouseMotionListener
	listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override{
		if (!isActive() || !(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT)) {
			return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
		}
		Geometry::Rect r = window.getRect();
		if(changeX > 0) {
			r.setWidth(motionEvent.x - r.getX());
		} else if(changeX < 0) {
			r.setWidth(r.getWidth() + (r.getX() - motionEvent.x));
			r.setX(motionEvent.x);
		}
		if(changeY > 0) {
			r.setHeight(motionEvent.y - r.getY());
		} else if(changeY < 0) {
			r.setHeight(r.getHeight() + (r.getY() - motionEvent.y));
			r.setY(motionEvent.y);
		}
		
		r.setWidth(std::max(r.getWidth(),10.0f));
		r.setHeight(std::max(r.getHeight(), getGUI().getGlobalValue(PROPERTY_WINDOW_TITLEBAR_HEIGHT)));
		window.setRect(r);

		return LISTENER_EVENT_CONSUMED;
	}
};

//! AutoMinimizer ---|> MouseMotionListener
struct AutoMinimizer:public MouseMotionListener{
	Window & win;
	AutoMinimizer(Window & _win):MouseMotionListener(),win(_win)	{	}
	virtual ~AutoMinimizer() 										{	}
	// ---|> MouseMotionListener
	listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override{
		if(motionEvent.buttonMask == Util::UI::MASK_NO_BUTTON) {
			bool inside=win.getAbsRect().changeSizeCentered(40,40).contains(motionEvent.x, motionEvent.y);
			if(inside && win.isMinimized()){
				win.restoreRect();
			} else if(!inside && !win.isMinimized()){
				win.minimize();
			}
		}
		return LISTENER_EVENT_NOT_CONSUMED;
	}
};

// ------------------------------------------------------------------------

//! WindowRectAnimation ---|> AnimationHandler
class WindowRectAnimation:public AnimationHandler{
	public:
		Geometry::Rect sourceRect;
		Geometry::Rect targetRect;
		bool enableClientRect;
		WindowRectAnimation(Window * w,Geometry::Rect _targetRect, float _duration,bool _enableClientRect) :
			AnimationHandler(w,_duration),sourceRect(w->getRect()),targetRect(std::move(_targetRect)),enableClientRect(_enableClientRect){
		}
		virtual ~WindowRectAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float currentTime) override {
			if(currentTime > getEndTime()) {
				finish();
				return false;
			}
			const float t = (currentTime - getStartTime()) / getDuration();
			const auto newPos = Geometry::Interpolation::linear(sourceRect.getPosition(), targetRect.getPosition(), t);
			const auto newSize = Geometry::Interpolation::linear(sourceRect.getSize(), targetRect.getSize(), t);
			getComponent()->setRect(newPos, newSize);
			return true;
		}

		// ---|> AnimationHandler
		void finish() override {
			getComponent()->setRect(targetRect);
			if(enableClientRect) {
				dynamic_cast<Window *>(getComponent())->clientArea()->enable();
			}
		}
};
//! WindowCloseAnimation ---|> AnimationHandler
class WindowCloseAnimation:public AnimationHandler{
	public:
		float originalHeight;
		WindowCloseAnimation(Window * w) :
			AnimationHandler(w,0.3),originalHeight(w->getHeight()){
		}
		virtual ~WindowCloseAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float currentTime) override {
			if(currentTime > getEndTime()) {
				finish();
				return false;
			}

			const float t = (currentTime - getStartTime()) / getDuration();
			getComponent()->setHeight(Geometry::Interpolation::quadraticBezier(originalHeight, 0.25f * originalHeight, 25.0f, t));
			return true;
		}
		// ---|> AnimationHandler
		void finish() override{
			Window * w = dynamic_cast<Window *>(getComponent());
			w->setHeight(originalHeight);
			w->_onClosed();
		}
};
//! WindowSpielereiAnimation ---|> AnimationHandler
class WindowSpielereiAnimation:public AnimationHandler{
	public:
		Geometry::Vec2f pos;
		Geometry::Vec2f lastPos;
		float prevTime;

		WindowSpielereiAnimation(Window * w) :
			AnimationHandler(w,0.3),pos(w->getPosition()),lastPos(w->getPosition()),prevTime(-1){
		}
		virtual ~WindowSpielereiAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float t) override{
			Window * w = dynamic_cast<Window *>(getComponent());
			if(prevTime<0)
				prevTime=getLastTime();

			float lastDuration=getLastTime() - prevTime;
			Geometry::Vec2f movement= lastDuration==0 ? Geometry::Vec2f(0,0) :
					(w->getPosition()-lastPos+ Geometry::Vec2f(0, 0.098))/lastDuration * (t-getLastTime()) ;
			prevTime=getLastTime();

			if( (w->getPosition().y() + movement.y() + w->getHeight()) > 1024 && movement.y() >0){
				float d=movement.y()*2.0f;
				if(d>8 && w->getHeight()>50)
					w->setHeight( w->getHeight() - std::min(d,10.0f));

				movement.x( movement.x()*0.5);
				movement.y( movement.y()*-0.5);
			}
			if( (w->getPosition().x() + movement.x() + w->getWidth()) > 1024 && movement.x() >0){
				movement.x( movement.x()*-0.5);
				movement.y( movement.y()*0.8);
			}
			if( w->getPosition().y() < 0 && movement.y() <0){
				movement.x( movement.x()*0.5);
				movement.y( movement.y()*-0.5);
			}
			if( w->getPosition().x() < 0 && movement.x() <0){
				movement.x( movement.x()*-0.5);
				movement.y( movement.y()*0.8);
			}
			lastPos=pos;
			pos+=movement;
			w->setPosition( pos );
			return true;
		}

};

// ------------------------------------------------------------------------

//! (ctor)
Window::Window(GUI_Manager & _gui,const Geometry::Rect & _r,const std::string & _title,flag_t _flags/*=0*/):
		Container(_gui,_r,_flags),ActionListener(),MouseButtonListener(),KeyListener(),
		minimized(false),opacity(1.0),autoMinimizer(){

	clientAreaPanel=new Container(_gui);
	_addChild(clientAreaPanel.get());
	init();

	setTitle(_title);

	//ctor
}

//! (dtor)
Window::~Window(){
	//dtor
}

void Window::close(){
	restore();
	getGUI().finishAnimations(this);
	getGUI().addAnimationHandler(new WindowCloseAnimation(this));
}


void Window::init(){
	setMouseCursorProperty(PROPERTY_MOUSECURSOR_COMPONENTS);
	    
	const int borderSize=getGUI().getGlobalValue(PROPERTY_WINDOW_BORDER_SIZE);
	const int titleHeight=getGUI().getGlobalValue(PROPERTY_WINDOW_TITLEBAR_HEIGHT);

	titlePanel=new TitlePanel(*this);
	titlePanel->setExtLayout( ExtLayouter::REFERENCE_X_LEFT|ExtLayouter::POS_X_ABS|ExtLayouter::ALIGN_X_LEFT|
									ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::POS_Y_ABS|ExtLayouter::ALIGN_Y_TOP|
									ExtLayouter::WIDTH_ABS | ExtLayouter::HEIGHT_ABS,
									Geometry::Vec2(borderSize,borderSize),
									Geometry::Vec2(-2.0f * std::max(borderSize,1),titleHeight-borderSize) );

	_addChild(titlePanel.get());
	resizePanel=new ResizePanel(*this);
	resizePanel->setSize(12,12);
	resizePanel->setExtLayout( ExtLayouter::REFERENCE_X_RIGHT|ExtLayouter::POS_X_ABS|ExtLayouter::ALIGN_X_RIGHT|
									ExtLayouter::REFERENCE_Y_BOTTOM|ExtLayouter::POS_Y_ABS|ExtLayouter::ALIGN_Y_BOTTOM ,
									Geometry::Vec2(1.0,1.0));
    resizePanel->setMouseCursorProperty(PROPERTY_MOUSECURSOR_RESIZEDIAGONAL);
	_addChild(resizePanel.get());

	AbstractProperty * p = new UseShapeProperty(PROPERTY_BUTTON_SHAPE,PROPERTY_WINDOW_BUTTON_SHAPE);

	hiddenButton=new Button(getGUI());
	hiddenButton->setText("h");
	hiddenButton->addProperty(p);
	hiddenButton->setTooltip("Hide window components when not selected");
	hiddenButton->setActionName(actionId_hide);
	hiddenButton->setActionListener(this);
	_addChild(hiddenButton.get());

	minimizeButton=new Button(getGUI());
	minimizeButton->setText(".");
	minimizeButton->addProperty(p);
	minimizeButton->setTooltip("Toggle auto-minimization");
	minimizeButton->setActionName(actionId_minimize);
	minimizeButton->setActionListener(this);
	_addChild(minimizeButton.get());

	disableButton=new Button(getGUI());
	disableButton->setText("X");
	disableButton->addProperty(p);
	disableButton->setTooltip("Close window");
	disableButton->setActionName(actionId_close);
	disableButton->setActionListener(this);
	_addChild(disableButton.get());


	titleTextLabel=new Label(getGUI());
	titleTextLabel->addProperty( new UseColorProperty(PROPERTY_TEXT_COLOR, PROPERTY_WINDOW_TITLE_COLOR) );
	titleTextLabel->addProperty( new UseFontProperty(PROPERTY_DEFAULT_FONT, PROPERTY_WINDOW_TITLE_FONT) );

	titleTextLabel->setTextStyle(Draw::TEXT_ALIGN_CENTER|Draw::TEXT_ALIGN_MIDDLE);
	titleTextLabel->setExtLayout( ExtLayouter::REFERENCE_X_LEFT|ExtLayouter::POS_X_ABS|ExtLayouter::ALIGN_X_LEFT|
										ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::POS_Y_ABS|ExtLayouter::ALIGN_Y_TOP|
										ExtLayouter::WIDTH_REL | ExtLayouter::HEIGHT_REL,
										Geometry::Vec2(0,0),
										Geometry::Vec2(1.0f,1.0f) );
	titlePanel->addContent(titleTextLabel.get());


	clientAreaPanel->setExtLayout( ExtLayouter::REFERENCE_X_LEFT|ExtLayouter::POS_X_ABS|ExtLayouter::ALIGN_X_LEFT|
										ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::POS_Y_ABS|ExtLayouter::ALIGN_Y_TOP|
										ExtLayouter::WIDTH_ABS | ExtLayouter::HEIGHT_ABS,
										Geometry::Vec2(borderSize,titleHeight),
										Geometry::Vec2(-2.0f * borderSize, -static_cast<float>(titleHeight)));


	addMouseButtonListener(this);
	addKeyListener(this);
}

//! ---|> Component
void Window::doLayout(){
	const unsigned int titleHeight=getGUI().getGlobalValue(PROPERTY_WINDOW_TITLEBAR_HEIGHT);
	const bool hideComponents = getFlag(HIDDEN_WINDOW) && !isSelected();

	if (getWidth()<10 || getHeight()<titleHeight){
		Geometry::Rect r=getLocalRect();
		r.include(10,titleHeight);
		setRect(r);
	}


	if(hideComponents){
		titlePanel->disable();
	}else{
		titlePanel->enable();
	}

	float titleButtonPos=getWidth()-titleHeight-2;

	if(getFlag(NO_CLOSE_BUTTON)||isMinimized()||hideComponents ){
		disableButton->disable();
	}else{
		disableButton->setSize(titleHeight-2,titleHeight-2);
		disableButton->setPosition(Geometry::Vec2(titleButtonPos,1));
		disableButton->enable();
		titleButtonPos-=titleHeight;
	}

	if(getFlag(NO_MINIMIZE_BUTTON)||isMinimized()||hideComponents){
		hiddenButton->disable();
		minimizeButton->disable();
	}else{
		hiddenButton->setSize(titleHeight-2,titleHeight-2);
		hiddenButton->setPosition(Geometry::Vec2(titleButtonPos,1));
		hiddenButton->enable();
		hiddenButton->setSwitch(getFlag(HIDDEN_WINDOW));
		titleButtonPos-=titleHeight;

		minimizeButton->setSize(titleHeight-2,titleHeight-2);
		minimizeButton->setPosition(Geometry::Vec2(titleButtonPos,1));
		minimizeButton->enable();
		titleButtonPos-=titleHeight;
	}

	if(getFlag(NO_RESIZE_PANEL)||isMinimized()||hideComponents){
		resizePanel->disable();
	}else{
		resizePanel->enable();
	}
}

//! ---|> Component
void Window::doDisplay(const Geometry::Rect & region){
	getGUI().pushScissor(Geometry::Rect_i(getAbsRect()));

	if(!isMinimized() && !getFlag(HIDDEN_WINDOW) ){
		getGUI().displayShape(PROPERTY_WINDOW_ACTIVE_SHAPE,getLocalRect());
		if(isSelected())
			getGUI().displayShape(PROPERTY_WINDOW_ACTIVE_SHAPE,getLocalRect());
		else
			getGUI().displayShape(PROPERTY_WINDOW_PASSIVE_SHAPE,getLocalRect());
	}

	displayChildren(region);
	getGUI().popScissor();

	if(!getFlag(HIDDEN_WINDOW) ){//|| isSelected()){
		Draw::dropShadow(getLocalRect());
	}
}

//! ---|> Component
void Window::invalidateRegion(){
	if(getGUI().isLazyRenderingEnabled()){
		for(Component * c = this;c!=nullptr;c=c->getParent())
			if(!c->isEnabled())
				return;
		getGUI().invalidateRegion(getAbsRect().changeSize(10,10));
	}
}

//! ---|> MouseButtonListener
listenerResult_t Window::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent){
	if(buttonEvent.pressed){
		if(resizePanel.isNotNull() && !isMinimized()){
			static const float resizeBorderWidth = 5.0f;
			const float clickX = buttonEvent.x - getRect().getX();
			const float clickY = buttonEvent.y - getRect().getY();
			if(clickX <=resizeBorderWidth){ // left
				if(clickY >=getHeight()-resizeBorderWidth){ // bottom left
					static_cast<ResizePanel*>(resizePanel.get())->startResizing(-1,1);
				}else if(clickY <=resizeBorderWidth){ // top left
					static_cast<ResizePanel*>(resizePanel.get())->startResizing(-1,-1);
				}else{
					static_cast<ResizePanel*>(resizePanel.get())->startResizing(-1,0);
				}
			}else if(clickY <=resizeBorderWidth){ // top
				if(clickX >=getWidth()-resizeBorderWidth){ // top right
					static_cast<ResizePanel*>(resizePanel.get())->startResizing(1,-1);
				}else{
					static_cast<ResizePanel*>(resizePanel.get())->startResizing(0,-1);
				}
			}else if(clickX >=getWidth()-resizeBorderWidth){ // right
				static_cast<ResizePanel*>(resizePanel.get())->startResizing(1,0);
			}else if(clickY >=getHeight()-resizeBorderWidth){ // bottom
				static_cast<ResizePanel*>(resizePanel.get())->startResizing(0,1);
			}
			
		}
		getGUI().selectFirst(this);
		return LISTENER_EVENT_CONSUMED;
	}
	return LISTENER_EVENT_NOT_CONSUMED;
}

//! (internal)
void Window::_onClosed(){
	disable();
	getGUI().componentActionPerformed(this,ACTION_onWindowClosed);
	if(getFlag(ONE_TIME_WINDOW)){
		getGUI().markForRemoval(this);
	}
}

//! ---|> Component
bool Window::onSelect(){
	if(getFlag(HIDDEN_WINDOW)) // buttons become visible
		invalidateLayout();
	return true;
}

//! ---|> Component
bool Window::onUnselect(){
	if(getFlag(HIDDEN_WINDOW)) // buttons become invisible
		invalidateLayout();
	return true;
}

void Window::setTitle(const std::string & title){
	titleTextLabel->setText(title);
	invalidateRegion();
}


std::string Window::getTitle()const{
	return titleTextLabel->getText();
}

//! ---|> KeyListener
bool Window::onKeyEvent(Component * /*component*/, const Util::UI::KeyboardEvent & keyEvent){
	if (keyEvent.pressed && keyEvent.key == Util::UI::KEY_ESCAPE) {
		unselect();
		return true;
	}
	return false;
}


void Window::minimize(){
	if (minimized)
		return;
	if(!autoMinimizer){
		autoMinimizer.reset(new AutoMinimizer(*this));
		getGUI().addMouseMotionListener(autoMinimizer.get());
		minimizeButton->setText("A");
	}

	minimized=true;
	getGUI().finishAnimations(this);
	storedRect = getRect();
	getGUI().addAnimationHandler(
			new WindowRectAnimation(this,Geometry::Rect(getPosition().getX(),getPosition().getY(),
											150,getGUI().getGlobalValue(PROPERTY_WINDOW_TITLEBAR_HEIGHT)),0.2, false));
	clientAreaPanel->disable();
}


void Window::restoreRect(){
	if (!minimized)
		return;
	getGUI().finishAnimations(this);
	getGUI().addAnimationHandler(
			new WindowRectAnimation(this,storedRect,0.2, true));

	minimized=false;
}

void Window::restore(){
	restoreRect();
	if(autoMinimizer){
		getGUI().removeMouseMotionListener(autoMinimizer.get());
		autoMinimizer.reset();
		minimizeButton->setText(".");
	}
}


void Window::setLogo(Component * newLogo){
	if(newLogo==getLogo())
		return;
	titlePanel->removeContent(getLogo());
	logo=newLogo;
	titlePanel->addContent(getLogo());
	getLogo()->setPosition(Geometry::Vec2(0,0));
}

//! ---|> ActionListener
listenerResult_t Window::handleAction(Component *,const Util::StringIdentifier & actionName){
	if(actionName==actionId_minimize){
		if (autoMinimizer)
			restore();
		else
			minimize();
		return LISTENER_EVENT_CONSUMED;
	}else if(actionName==actionId_hide){
		setFlag(HIDDEN_WINDOW,!getFlag(HIDDEN_WINDOW));
		return LISTENER_EVENT_CONSUMED;
	}else if(actionName==actionId_close){
		if(getGUI().isShiftPressed()) {
			getGUI().finishAnimations(this);
			getGUI().addAnimationHandler(
				new WindowSpielereiAnimation(this));
			return LISTENER_EVENT_CONSUMED;
		}
		close();
		return LISTENER_EVENT_CONSUMED;
	}else return LISTENER_EVENT_NOT_CONSUMED;
}

}
