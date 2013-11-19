/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "GUI_Manager.h"

#include "Components/Button.h"
#include "Components/Checkbox.h"
#include "Components/Component.h"
#include "Components/ComponentPropertyIds.h"
#include "Components/Container.h"
#include "Components/Connector.h"
#include "Components/EditorPanel.h"
#include "Components/Icon.h"
#include "Components/LayoutHelper.h"
#include "Components/ListView.h"
#include "Components/Menu.h"
#include "Components/Panel.h"
#include "Components/Slider.h"
#include "Components/Splitter.h"
#include "Components/Tab.h"
#include "Components/Textarea.h"
#include "Components/Textfield.h"
#include "Components/Image.h"
#include "Components/TreeView.h"
#include "Components/Window.h"

#include "Base/Draw.h"
#include "Base/ImageData.h"
#include "Base/StyleManager.h"
#include "Base/AnimationHandler.h"
#include "Style/Style.h"
#include "Style/Colors.h" // \todo remove this!

#include <Util/Graphics/Bitmap.h>
#include <Util/Graphics/BitmapUtils.h>
#include <Util/Serialization/Serialization.h>
#include <Util/UI/Event.h>
#include <Util/UI/EventContext.h>
#include <Util/UI/UI.h>
#include <Util/UI/Window.h>
#include <Util/Timer.h>
#include <algorithm>
#include <functional>
#include <iostream>

// ---------------------------------------------
namespace GUI{
using Geometry::Vec2;
using Geometry::Rect;

class GlobalContainer : public Container{
	public:
		GlobalContainer(GUI_Manager & _gui,const Geometry::Rect & _r):Container(_gui,_r){
		}
		virtual ~GlobalContainer(){};

		// ---|> Container
		void bringChildToFront(Component * w) override{
			if(w==nullptr) return;
			if(w->getFlag(ALWAYS_ON_TOP)){
				_insertAfter(w,getLastChild());
			}else{
				Component * lastNotOnTop=getLastChild();
				while( lastNotOnTop && lastNotOnTop->getFlag(ALWAYS_ON_TOP)){
					lastNotOnTop=lastNotOnTop->getPrev();
				}
				if(lastNotOnTop)
					_insertAfter(w,lastNotOnTop);
			}

			w->select();
		}
};

// ---------------------------------------------

void GUI_Manager::copyStringToClipboard(const std::string & s) {
	window->setClipboardText(s);
}

std::string GUI_Manager::getStringFromClipboard() const {
	return window->getClipboardText();
}

// ----------
// ---- MouseCursor
/**
 * MouseCursorHandler ---|> MouseMotionListener,FrameListener
 */
class MouseCursorHandler : public MouseMotionListener, public MouseButtonListener {
	private:
		Util::Reference<GUI_Manager> gui;
		bool cursorLockedByButton;
	public:
		
		MouseCursorHandler(GUI_Manager & _gui) : MouseMotionListener(), MouseButtonListener(), gui(&_gui),cursorLockedByButton(false){
			activateCursor(nullptr);
		}
		
		std::shared_ptr<Util::UI::Cursor> queryHoverComponentMouseCursor(const Vec2 & absPos)const{
			for(Component * c=gui->getComponentAtPos(absPos);c!=nullptr;c=c->getParent()){
				if(c->hasMouseCursorProperty())
					return std::move(gui->getStyleManager().getMouseCursor(c->getMouseCursorProperty()));
			}
			return nullptr;
		}
		// if no cursor is given, the systems's default cursor is used (internally represented by nullptr)
		void activateCursor(std::shared_ptr<Util::UI::Cursor> cursor){
			if(gui->getWindow()!=nullptr && gui->getWindow()->getCursor() != cursor)
				gui->getWindow()->setCursor(std::move(cursor));
		}
		
		// ---|> MouseButtonListener
		listenerResult_t onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) override {
			activateCursor(std::move(queryHoverComponentMouseCursor(Geometry::Vec2(buttonEvent.x, buttonEvent.y))));
			cursorLockedByButton = buttonEvent.pressed;
			return LISTENER_EVENT_NOT_CONSUMED;
		}
		
		// ---|> MouseMotionListener
		listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override{
			// if mouse button is pressed cursor should not be changed, cause position can be outside component and would end in cursor switch!
			if(!cursorLockedByButton){
				activateCursor(std::move(queryHoverComponentMouseCursor(Geometry::Vec2(motionEvent.x, motionEvent.y))));
			}
			return LISTENER_EVENT_NOT_CONSUMED;
		}
};

// ----------
// ---- Tooltip
/**
 * TooltipHandler ---|> MouseMotionListener,FrameListener
 * \todo move TooltipHandler to Overlay
 */
class TooltipHandler:public Component, public MouseMotionListener,public FrameListener{
		Util::Reference<Component> activeComponent;
		float startingTime;
		Vec2 lastMousePos;
		std::string text;
		enum mode_t{
			SEARCHING,ACTIVE,INACTIVE
		} mode;
	public:

		TooltipHandler(GUI_Manager & _gui):Component(_gui),MouseMotionListener(),FrameListener(),startingTime(0),mode(SEARCHING)   {   }
		virtual ~TooltipHandler(){}

		Component * findTooltitComponent(const Vec2 & pos)const{
			for(Component * c=getGUI().getComponentAtPos(pos);c!=nullptr;c=c->getParent()){
				if(c->hasTooltip())
					return c;
			}
			return nullptr;
		}

		// ---|> MouseMotionListener
		listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override {
			const Geometry::Vec2 absPos(motionEvent.x, motionEvent.y);
			if(lastMousePos == absPos) {
				return LISTENER_EVENT_NOT_CONSUMED;
			}
			lastMousePos = absPos;
			activeComponent = findTooltitComponent(absPos);

			switch(mode){
				case ACTIVE:{
					if(activeComponent.isNull()){
						startingTime=Util::Timer::now();
						mode=SEARCHING;
						invalidateRegion();
					}
					break;
				}
				case SEARCHING:{
					startingTime=Util::Timer::now();
					break;
				}
				case INACTIVE:{
					startingTime=Util::Timer::now();
					mode=SEARCHING;
					invalidateRegion();
					break;
				}
				default:
					WARN("unexpected case in switch statement");
			}
//                std::cout <<  startingTime <<"; ";
			return LISTENER_EVENT_NOT_CONSUMED;
		}
		// ---|> FrameListener
		void onFrame(float timeSecs) override{
			if(mode==INACTIVE){
				return;
			}else if(mode==SEARCHING && timeSecs-startingTime > 0.250){
				mode=activeComponent.isNull() ? INACTIVE : ACTIVE;
				invalidateRegion();
			}
			if(mode!=ACTIVE)
				return;

			// detect if the component is deactivated while its tooltip is shown
			if(!getGUI().isCurrentlyEnabled(activeComponent.get())){
				mode = INACTIVE;
				return;
			}


			this->text=activeComponent->getTooltip();
			const Vec2 ttSize=Draw::getTextSize( text,getGUI().getActiveFont(PROPERTY_TOOLTIP_FONT) )+Vec2(8,8);


			Vec2 pos=activeComponent->getAbsPosition()+
						Vec2(activeComponent->getWidth()*0.9+4,activeComponent->getHeight()-2);
			// outside the screen? -> move to the left side of the component
			if(pos.getX()+ttSize.getWidth()>getGUI().getScreenRect().getWidth()){
				pos.setX( activeComponent->getAbsPosition().getX()+2-ttSize.getWidth() );
			}
			// outside the screen? -> move to the top side of the component
			if(pos.getY()+ttSize.getHeight()>getGUI().getScreenRect().getHeight()){
				pos.setY( activeComponent->getAbsPosition().getY()+activeComponent->getHeight()*0.1-ttSize.getHeight() );
			}

			setRect(Rect(pos.getX(),pos.getY(),ttSize.getWidth(),ttSize.getHeight()));

			// remove this, when tolltip is moved to overlay
			display(Rect());

		}
		//! ---|> Component
		void doDisplay(const Rect & /*region*/) override {
			 if(mode!=ACTIVE)
				return;
			Draw::drawFilledRect(getLocalRect(),Colors::ACTIVE_COLOR_1,false);
			Draw::drawLineRect(getLocalRect(),Colors::BORDER_COLOR,false);

			Draw::drawText(text, getLocalRect().changeSizeCentered(-8,-8),
						getGUI().getActiveFont(PROPERTY_TOOLTIP_FONT),
						getGUI().getActiveColor(PROPERTY_TOOLTIP_TEXT_COLOR));

			Draw::dropShadow(getLocalRect());//,color,textStyle,fontType);
		}

};


// ---------------------------------------------

//! (ctor)
GUI_Manager::GUI_Manager(Util::UI::EventContext & context) : 
	eventContext(context), window(nullptr), debugMode(0),
	lazyRendering(false), style(new StyleManager) {
	globalContainer=new GlobalContainer(*this,Rect(0,0,1280,1024));
	globalContainer->setMouseCursorProperty(PROPERTY_MOUSECURSOR_DEFAULT);
	
	auto th=new TooltipHandler(*this);
	addMouseMotionListener(th);
	addFrameListener(th);

	Style::initStyleManager(getStyleManager());
    
    auto mh = new MouseCursorHandler(*this);
    addMouseMotionListener(mh);
    addMouseButtonListener(mh);
}


//! (dtor)
GUI_Manager:: ~GUI_Manager(){
	cleanup();
	setActiveComponent(nullptr);
	globalContainer=nullptr;
}

Rect GUI_Manager::getScreenRect()const{
	return globalContainer->getLocalRect();
}

// ------------------------------------------------------------------------
// Event handling & Listener
bool GUI_Manager::isCtrlPressed() const {
	return eventContext.isCtrlPressed();
}

bool GUI_Manager::isShiftPressed() const {
	return eventContext.isShiftPressed();
}

//! (internal)
bool GUI_Manager::handleMouseMovement(const Util::UI::MotionEvent & motionEvent){
	// Global Listener
	for(auto it=mouseMoveListener.begin();
			it!=mouseMoveListener.end();){
		listenerResult_t result = (*it)->onMouseMove(nullptr, motionEvent);
		if(result & LISTENER_REMOVE_LISTENER){
			it = mouseMoveListener.erase(it);
		}else{
			++it;
		}
		if(result & LISTENER_EVENT_CONSUMED)
			return true;
	}
	// Local Listener
	const Geometry::Vec2 absPos(motionEvent.x, motionEvent.y);
	for(Component * c=getComponentAtPos(absPos);c!=nullptr;c=c->getParent()){
		mouseListenerList * l=MouseMotionListener::getListenerRegistry().getListeners(c);// c->getMouseMotionListener();
		if(l==nullptr)
			continue;
		for(auto it=l->begin();it!=l->end();){
			listenerResult_t result = (*it)->onMouseMove(c, motionEvent);
			if(result & LISTENER_REMOVE_LISTENER){
				it = l->erase(it);
			}else{
				++it;
			}
			if(result & LISTENER_EVENT_CONSUMED)
				return true;
		}
	}
	return false;
}

//! (internal)
bool GUI_Manager::handleMouseButton(const Util::UI::ButtonEvent & buttonEvent) {
	
	// handle global listeners
	for(auto it = mouseButtonListeners.begin(); it!=mouseButtonListeners.end(); ){
		const auto result = (*it)->onMouseButton(nullptr, buttonEvent);
		if( result & LISTENER_REMOVE_LISTENER ){
			it = mouseButtonListeners.erase(it);
		}else{
			++it;
		}
		if( result & LISTENER_EVENT_CONSUMED )
			return true;
	}
	
	
	Component * lastActive=getActiveComponent();
	setActiveComponent(nullptr);

	const Geometry::Vec2 absPos(buttonEvent.x, buttonEvent.y);
	for(Component::Ref c=getComponentAtPos(absPos);
			c.isNotNull() && c->isEnabled() && c->coversAbsPosition(absPos);
			c=c->getParent() ){

		std::list<MouseButtonListener*> * l=MouseButtonListener::getListenerRegistry().getListeners(c.get());//c->getMouseButtonListener();
		if(l==nullptr)
			continue;
		for(auto butIt = l->rbegin(); butIt != l->rend();) {
			listenerResult_t result = (*butIt)->onMouseButton(c.get(), buttonEvent);
			if(result & LISTENER_REMOVE_LISTENER){
				// \note This strange construct is used, because std::list does not take reverse_iterators
				//		as parameter for erase
				// @see Item 28 in the book "Effective STL"
				auto tempIter = l->erase((++butIt).base());
				butIt = std::list<MouseButtonListener*>::reverse_iterator(tempIter);

//				butIt= l->erase(butIt);
			}else{
				++butIt;
			}
			if( ! (result & LISTENER_EVENT_CONSUMED))
				continue;

			c->bringToFront();
			if (buttonEvent.pressed || c!=lastActive)
				return true;

			std::list<MouseClickListener*> * click=MouseClickListener::getListenerRegistry().getListeners(c.get());//->getMouseClickListener();
			if(click!=nullptr){
				const Geometry::Vec2 localPos = absPos - c->getAbsPosition();
				for(const auto & clickListener : *click) {
					if(clickListener->onMouseClick(c.get(), buttonEvent.button, localPos)) {
						break;
					}
				}
			}
			return true;
		}
	}
	if (buttonEvent.pressed){
		unselectAll();
	}
	return false;
}

//! (internal)
bool GUI_Manager::handleKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if(!keyEvent.pressed && keyRepeatInfo.get()!=nullptr){
		keyRepeatInfo.reset(nullptr);
	}
	for(Component::Ref c=globalContainer->findSelectedComponent();c!=nullptr && c->isEnabled(); c=c->getParent() ){
		std::list<KeyListener*> * l=KeyListener::getListenerRegistry().getListeners(c.get());//c->getKeyListener();
		if(l!=nullptr){
			for(const auto & keyListener : *l) {
				const bool consumed = keyListener->onKeyEvent(c.get(), keyEvent);
				if(consumed) {
					return true;
				}
			}
		}
	}
	if(!keyEvent.pressed)
		return false;
	else if(keyEvent.key == Util::UI::KEY_TAB){
		Component * selectedComponent=globalContainer->findSelectedComponent();
		if(selectedComponent && selectedComponent->hasParent()){
			selectedComponent->getParent()->unselectSubtree();
		}
		if(isShiftPressed()){
			selectPrev(selectedComponent);
		}else{
			selectNext(selectedComponent);
		}
		return true;
	}
	return false;
// global key listener
}

bool GUI_Manager::handleEvent(const Util::UI::Event & e) {
	switch(e.type) {
		case Util::UI::EVENT_MOUSE_BUTTON:
			return handleMouseButton(e.button);
		case Util::UI::EVENT_MOUSE_MOTION:
			return handleMouseMovement(e.motion);
		case Util::UI::EVENT_KEYBOARD:
			return handleKeyEvent(e.keyboard);
		case Util::UI::EVENT_JOY_AXIS:
		case Util::UI::EVENT_JOY_BUTTON:
		case Util::UI::EVENT_JOY_HAT:
		case Util::UI::EVENT_QUIT:
		case Util::UI::EVENT_RESIZE:
		default:
			break;
	}
	return false;
}

void GUI_Manager::enableKeyRepetition(const Util::UI::KeyboardEvent & keyEvent){
	if(keyRepeatInfo.get()==nullptr || keyRepeatInfo->second.key != keyEvent.key){
		keyRepeatInfo.reset( new std::pair<float,Util::UI::KeyboardEvent>(
				Util::Timer::now()+ getGlobalValue(PROPERTY_KEY_REPEAT_DELAY_1),keyEvent) );
	}
}
void GUI_Manager::disableKeyRepetition(){
	keyRepeatInfo.reset(nullptr);
}
		
// ------------------------------------------------------------------------

void GUI_Manager::registerWindow(Component * w){
	if (w)
		globalContainer->addContent(w);
}

void GUI_Manager::unregisterWindow(Component *w){
	globalContainer->removeContent(w);
}

void GUI_Manager::invalidateRegion(const Rect & region){
	invalidRegion.include(region);
}

void GUI_Manager::display(){
	
	{ // init draw process
		// update size
		Geometry::Rect_i viewport = Draw::queryViewport();
		globalContainer->setSize(viewport.getWidth(), viewport.getHeight());
		Draw::beginDrawing(Geometry::Vec2i(viewport.getWidth(),viewport.getHeight()));
	}

			
	cleanup();
	executeAnimations();

	
	{ // update layout
		int lastLayoutCount = 0;
		for(int i=0;;++i){
			const int layoutCount = globalContainer->layout();
			if(layoutCount==0) break;
			if(i>3 && layoutCount>=lastLayoutCount){
				if(getDebugMode()>0){
					// if this message occurs repeatedly, the layout does not converge
					std::cout << "(!pending layouts: "<< layoutCount <<" )";
				}
				// no progress!
				break;
			}
			lastLayoutCount = layoutCount;
		}
	}

	if(isLazyRenderingEnabled()){
		Draw::drawLineRect(invalidRegion,Util::Color4ub(255,0,0,128));
		pushScissor(Geometry::Rect_i(invalidRegion));
		Draw::clearScreen(Util::Color4ub(0,0,0,0));
		
		globalContainer->display(invalidRegion);
		invalidRegion.invalidate();
		popScissor();
	}else{
		Rect r;
		r.invalidate();
		globalContainer->display(r);
	}

	
	{ // execute frameListeners
		// make a copy to allow insertions and deletions.
		std::vector<FrameListener *> listenerListCopy(frameListener.begin(),frameListener.end()); 
		
		const float t=Util::Timer::now();
		for(auto & l : listenerListCopy)
			l->onFrame(t);
		
		//! key repetition \note !!! This is not the proper place to do this, as this may introduce side effects !!!!
		if(keyRepeatInfo.get()!=nullptr && keyRepeatInfo->first<t){
			keyRepeatInfo->first = t+getGlobalValue(PROPERTY_KEY_REPEAT_DELAY_2);
			handleKeyEvent(keyRepeatInfo->second);
		}
	}
	Draw::endDrawing();
}

void GUI_Manager::setActiveComponent(Component * c){
	activeComponent=c;
}

void GUI_Manager::unselectAll(){
	setActiveComponent(nullptr);
	globalContainer->unselectSubtree();
}

void GUI_Manager::closeAllMenus(){
	for(Component * c = globalContainer->getFirstChild();c!=nullptr;c=c->getNext()){
		if(Menu * m = dynamic_cast<Menu*>(c))
			m->close();
	}
}

void GUI_Manager::selectNext(Component * _c){
	Component::Ref c=_c;
	while(c.isNotNull()){
		if(c->getNext()!=nullptr){
			c=c->getNext();
			if(selectFirst(c.get()))
				break;
		}else{
			c=c->getParent();
		}
	}
}

void GUI_Manager::selectPrev(Component * _c){
	Component::Ref c=_c;
	while(c.isNotNull()){
		if(c->getPrev()!=nullptr){
			c=c->getPrev();
			if(selectLast(c.get()))
				break;
		}else{
			c=c->getParent();
		}
	}
}

//! Select first selectable component in subtree using dfs
bool GUI_Manager::selectFirst(Component * c){
	if(c==nullptr)
		return false;

	if(c->isSelectable()){
		c->select();
		return true;
	}
	Container * con=dynamic_cast<Container*>(c);
	if(con==nullptr){
		return false;
	}

	for(c=con->getFirstChild();c!=nullptr;c=c->getNext()){
		if(selectFirst(c)){
			return true;

		}
	}
	return false;
}

//! Select last selectable component in subtree using dfs
bool GUI_Manager::selectLast(Component * c){
	if(c==nullptr)
		return false;
	if(c->isSelectable()){
		c->select();
		return true;
	}
	Container * con=dynamic_cast<Container*>(c);
	if(con==nullptr)
		return false;
	for(c=con->getLastChild();c!=nullptr;c=c->getPrev()){
		if(selectLast(c))
			return true;
	}
	return false;
}

void GUI_Manager::componentActionPerformed(Component *c,const Util::StringIdentifier & actionName){
	for(auto it=actionListener.begin();it!=actionListener.end();){
		listenerResult_t result=(*it)->handleAction(c,actionName);
		if(result & LISTENER_REMOVE_LISTENER){
			it=actionListener.erase(it);
		}else{
			++it;
		}
		if(result&LISTENER_EVENT_CONSUMED)
			return;
	}
//    std::cout << "Info: Unhandled Action:"<<actionName<<" \n";
}
///**
// *
// */
//void GUI_Manager::componentMouseClicked(Component *c,unsigned int button,const Geometry::Vec2 &localPos,bool pressed){
//	std::cout << "\nTODO!!!!!! "<< c<<":"<<button<<":"<<localPos.toString()<<"\n";
//
//}

void GUI_Manager::componentDataChanged(Component *c,const Util::StringIdentifier & actionName){
	// inform component's dataChange-Listener
	dataChangeListenerList * l=DataChangeListener::getListenerRegistry().getListeners(c);//c->getDataChangeListener();
	if(l!=nullptr){
		for(const auto & changeListener : *l) {
			if(changeListener) {
				changeListener->handleDataChange(c, actionName);
			}
		}
	}
	// inform global dataChange-Listener
	for(const auto & changeListener : dataChangeListener) {
		if(changeListener) {
			changeListener->handleDataChange(c, actionName);
		}
	}
}

Component * GUI_Manager::getComponentAtPos(const Geometry::Vec2 & pos){
	return globalContainer->getComponentAtPos(pos);
}

bool GUI_Manager::isCurrentlyEnabled(Component * c)const{
	for(Component * current = c ; current!=nullptr ; current=current->getParent()){
		if(!current->isEnabled())
			return false;
		else if(current == globalContainer.get())
			return true;
	}
	return false;
}

void GUI_Manager::pushScissor(const Geometry::Rect_i & r) {
	Geometry::Rect_i scissorRect(r.getX() - 1, r.getY()-1, r.getWidth() + 2, r.getHeight() + 2);

	if (!scissors.empty()) {
		scissorRect.clipBy(scissors.top());
		if (scissorRect.isInvalid()) {
			scissorRect = Geometry::Rect_i(0, 0, 0, 0);
		}
	}
	Draw::setScissor(scissorRect);
	scissors.push(scissorRect);
}

void GUI_Manager::popScissor() {
	if (scissors.empty()){
		WARN(" ");
		return;
	}
	scissors.pop();
	if (scissors.empty()) {
		Draw::resetScissor();
	} else {
		Draw::setScissor(scissors.top());
	}
}
// -----------
// ---- Cleanup
void GUI_Manager::markForRemoval(Component *c){
	if(c!=nullptr)
		removalList.push_back(c);
}

void GUI_Manager::cleanup(){
	if(!removalList.empty()){
		std::list<Util::Reference<Component>> l;
		l.swap(removalList);
		for(const auto & componentRef : l) {
			Component::destroy(componentRef.get());
		}
	}
}


// ----------
// ---- animation handling

void GUI_Manager::addAnimationHandler(AnimationHandler * h){
	animationHandlerList.push_back(h);
	h->setStartTime( Util::Timer::now() );
}

//! (internal)
void GUI_Manager::executeAnimations(){
	float t=Util::Timer::now();
	for(auto it=animationHandlerList.begin();
			it!=animationHandlerList.end();) {
		if( (*it)->animate(t) ){
			(*it)->updateLastTime(t);
			++it;
		}else{
			delete *it;
			it = animationHandlerList.erase(it);
		}
	}
}

//! (internal)
void GUI_Manager::finishAnimations(Component * c){
	for(auto it=animationHandlerList.begin();
			it!=animationHandlerList.end();) {
		if( (*it)->getComponent()==c ){
			(*it)->finish();
			delete *it;
			it = animationHandlerList.erase(it);
		}else{
			++it;
		}
	}
}
//! (internal)
void GUI_Manager::stopAnimations(Component * c){
	for(auto it=animationHandlerList.begin();
			it!=animationHandlerList.end();) {
		if( (*it)->getComponent()==c ){
			delete *it;
			it = animationHandlerList.erase(it);
		}else{
			++it;
		}
	}
}

// ----------
// ---- Properties

void GUI_Manager::displayShape(const propertyId_t id,const Rect & rect,AbstractShape::flag_t flags){
	getStyleManager().getShape(id)->display(rect,flags);
}
void GUI_Manager::displayLineShape(const propertyId_t id,const std::vector<Geometry::Vec2> & points,AbstractShape::flag_t flags){
	auto shape = dynamic_cast<AbstractLineShape*>( getStyleManager().getShape(id) );
	if(shape!=nullptr)
		shape->displayLine(points,flags);
}

Util::Color4ub GUI_Manager::getActiveColor(const propertyId_t id)const{
	return getStyleManager().getColor(id);
}
AbstractFont * GUI_Manager::getActiveFont(const propertyId_t id)const{
	return getStyleManager().getFont(id);
}
AbstractFont * GUI_Manager::getDefaultFont(const propertyId_t id)const{
	return getStyleManager().getDefaultFont(id);
}

float GUI_Manager::getGlobalValue(const propertyId_t id)const{
	return getStyleManager().getGlobalValue(id);
}
void GUI_Manager::disableProperty(const Util::Reference<AbstractProperty> & p)const{
	p->disable(getStyleManager());
}
void GUI_Manager::enableProperty(const Util::Reference<AbstractProperty> & p)const{
	p->enable(getStyleManager());
}

void GUI_Manager::setDefaultColor(const propertyId_t id,const Util::Color4ub & c){
	getStyleManager().setDefaultColor(id,c);
}
void GUI_Manager::setDefaultFont(const propertyId_t id,AbstractFont * f){
	getStyleManager().setDefaultFont(id,f);
}
void GUI_Manager::setDefaultShape(const propertyId_t id,AbstractShape * s){
	getStyleManager().setDefaultShape(id,s);
}
void GUI_Manager::setGlobalValue(const propertyId_t id,float v){
	getStyleManager().setGlobalValue(id,v);
}

void GUI_Manager::registerMouseCursor(const propertyName_t & id, const Util::Reference<Util::Bitmap> & bitmap, const uint32_t clickX, const uint32_t clickY){
	getStyleManager().setMouseCursor(id, Util::UI::createCursor(bitmap, clickX, clickY));
}

void GUI_Manager::removeMouseCursor(const propertyName_t & id){
	getStyleManager().removeMouseCursor(id);
}



// --------------------
// ---- Factories

//! [factory] Button
Button * GUI_Manager::createButton(const std::string & text/*=""*/,flag_t flags/*=0*/){
	auto button = new Button(*this,flags);
	button->setText(text);
	return button;
}

//! [factory] Button
Button * GUI_Manager::createButton(const std::string & text/*=""*/,const Util::StringIdentifier & actionName/*=""*/,flag_t flags/*=0*/){
	auto button = new Button(*this,flags);
	button->setActionName(actionName);
	button->setText(text);
	return button;
}

//! [factory] Button
Button * GUI_Manager::createButton(const Geometry::Rect & r,const std::string & text/*=""*/,const Util::StringIdentifier & actionName/*=""*/,flag_t flags/*=0*/){
	auto button = new Button(*this,flags);
	button->setActionName(actionName);
	button->setText(text);
	button->setRect(r);
	return button;
}

//! [factory] Checkbox
Checkbox * GUI_Manager::createCheckbox(const std::string & text/*=""*/,bool checked/*=false*/,flag_t flags/*=0*/){
	return new Checkbox(*this,checked,text,flags);
}

//! [factory] Checkbox
Checkbox * GUI_Manager::createCheckbox(const Geometry::Rect & r,const std::string & text/*=""*/,bool checked/*=false*/,flag_t flags/*=0*/){
	return new Checkbox(*this,r,checked,text,flags);
}

//! [factory] Container
Container * GUI_Manager::createContainer(const Geometry::Rect & r,flag_t flags/*=0*/){
	return new Container(*this,r,flags);
}

//! [factory] Connector
Connector * GUI_Manager::createConnector(flag_t flags/*=0*/){
	return new Connector(*this,flags);
}

//! [factory] Icon
Icon *GUI_Manager::createIcon(const Geometry::Vec2 & pos, Util::WeakPointer<ImageData> imageData,const Geometry::Rect & imageRect,flag_t flags/*=0*/){
	return new Icon(*this,pos,imageData,imageRect,flags);
}

//! [factory] Icon
Icon *GUI_Manager::createIcon(const Geometry::Rect & r,flag_t flags/*=0*/){
	return new Icon(*this,r,flags);
}

//! [factory] Image
Image *GUI_Manager::createImage(const Geometry::Rect & r,flag_t flags/*=0*/){
	return new Image(*this,r,flags);
}

//! [factory] Image
Image *GUI_Manager::createImage(const Util::FileName & fileName, flag_t flags/*=0*/) {
	Util::Reference<Util::Bitmap> bitmap = Util::Serialization::loadBitmap(fileName);
	if (bitmap.isNull()) {
		return nullptr;
	}
	auto image = new Image(*this, *bitmap.get(), flags);
	return image;
}

//! [factory] Image
Image *GUI_Manager::createImage(const Util::Bitmap & bitmap, flag_t flags/*=0*/) {
	return new Image(*this, bitmap, flags);
}

//! [factory] Label
Label *GUI_Manager::createLabel(const Geometry::Rect & r,const std::string & text/*=""*/,flag_t flags/*=0*/){
	return new Label(*this,r,text,flags);
}

//! [factory] Label
Label * GUI_Manager::createLabel(const std::string & text/*=""*/,flag_t flags/*=0*/){
	return new Label(*this,text,flags);
}

//! [factory] Label
ListView * GUI_Manager::createListView(flag_t flags/*=0*/){
	return new ListView(*this,flags);
}

//! [factory] Menu
Menu * GUI_Manager::createMenu(flag_t flags/*=0*/){
	auto m=new Menu(*this,flags);
	registerWindow(m);
	return m;
}



//! [factory] EditorPanel
EditorPanel * GUI_Manager::createEditorPanel(flag_t flags/*=0*/){
	return new EditorPanel(*this,flags);
}

//! [factory] NextColumn
NextColumn * GUI_Manager::createNextColumn(float additionalSpacing){
	return new NextColumn(*this,additionalSpacing);
}

//! [factory] NextRow
NextRow * GUI_Manager::createNextRow(float additionalSpacing){
	return new NextRow(*this,additionalSpacing);
}

//! [factory] Panel
Panel * GUI_Manager::createPanel(const Geometry::Rect & r,flag_t flags/*=0*/){
	return new Panel(*this,r,flags);
}

//! [factory] Panel
Panel * GUI_Manager::createPanel(flag_t flags/*=0*/){
	return new Panel(*this,flags);
}

//! [factory] Slider
Slider * GUI_Manager::createSlider(const Geometry::Rect & r,float left/*=0*/,float right/*=1*/,int steps/*=10*/,const std::string & dataName/*=""*/,flag_t flags/*=0*/){
	return new Slider(*this,r,left,right,steps,dataName,flags);
}

//! [factory] Splitter
Splitter * GUI_Manager::createVSplitter(flag_t flags/*=0*/){
	return new Splitter(*this,Splitter::VERTICAL,flags);
}

//! [factory] Splitter
Splitter * GUI_Manager::createHSplitter(flag_t flags/*=0*/){
	return new Splitter(*this,Splitter::HORIZONTAL,flags);
}

//! [factory] TabbedPanel
TabbedPanel * GUI_Manager::createTabbedPanel(flag_t flags/*=0*/){
	return new TabbedPanel(*this,flags);
}

//! [factory] TabbedPanel
TabbedPanel * GUI_Manager::createTabbedPanel(const Geometry::Rect & r,flag_t flags/*=0*/){
	auto c = new TabbedPanel(*this,flags);
	c->setRect(r);
	return c;
}

//! [factory] Textarea
Textarea * GUI_Manager::createTextarea(const std::string &text/*=""*/,flag_t flags/*=0*/){
	auto c = new Textarea(*this,flags);
	c->setText(text);
	return c;
}
//! [factory] TextField
Textfield * GUI_Manager::createTextfield(const std::string &text/*=""*/,flag_t flags/*=0*/){
	return new Textfield(*this,text,"",flags);
}

//! [factory] TextField
Textfield * GUI_Manager::createTextfield(const Geometry::Rect & r,const std::string &text/*=""*/,flag_t flags/*=0*/){
	return new Textfield(*this,r,text,"",flags);
}

//! [factory] TreeView
TreeView * GUI_Manager::createTreeView(const Geometry::Rect & r,unsigned int flags/*=0*/){
	return new TreeView(*this,r,"",flags);
}

//! [factory] TreeView::TreeViewEntry
Container * GUI_Manager::createTreeViewEntry(Component * c){
	return new TreeView::TreeViewEntry(*this,nullptr,c);
}

//! [factory] Window
Window * GUI_Manager::createWindow(const Geometry::Rect & r,const std::string & title/*=""*/,flag_t flags/*=0*/){
	auto w=new Window(*this,r,title,flags);
	registerWindow(w);
	return w;
}

}
