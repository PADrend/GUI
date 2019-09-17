/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
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
#include "Components/ComponentTooltipFeature.h"
#include "Components/ComponentHoverPropertyFeature.h"

#include "Base/AnimationHandler.h"
#include "Base/Draw.h"
#include "Base/ImageData.h"
#include "Base/ListenerHelper.h"
#include "Base/StyleManager.h"
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
#include <Rendering/RenderingContext/RenderingContext.h>

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
	if(window)
		window->setClipboardText(s);
	else
		alternativeClipboard = s;
}

std::string GUI_Manager::getStringFromClipboard() const {
	return window ? window->getClipboardText() : alternativeClipboard;
}

// ----------
// ---- MouseCursor
class MouseCursorHandler {
	private:
		GUI_Manager & gui;
		bool cursorLockedByButton;
		MouseButtonListener mouseButtonListener;
		MouseMotionListener mouseMotionListener;
	public:
		MouseCursorHandler(GUI_Manager & _gui) :
			gui(_gui),
			cursorLockedByButton(false),
			mouseButtonListener(createGlobalMouseButtonListener(_gui,
				[this](Component *, const Util::UI::ButtonEvent & buttonEvent) {
					activateCursor(queryHoverComponentMouseCursor(Geometry::Vec2(buttonEvent.x, buttonEvent.y)));
					cursorLockedByButton = buttonEvent.pressed;
					return false;
				})),
			mouseMotionListener(createMouseMotionListener(_gui,
				[this](Component *, const Util::UI::MotionEvent & motionEvent) {
					// If mouse button is pressed, cursor should not be changed,
					// because the position can be outside component and would
					// end in cursor switch!
					if(!cursorLockedByButton) {
						activateCursor(queryHoverComponentMouseCursor(Geometry::Vec2(motionEvent.x, motionEvent.y)));
					}
					return false;
				})) {
			activateCursor(nullptr);
		}
		std::shared_ptr<Util::UI::Cursor> queryHoverComponentMouseCursor(const Vec2 & absPos)const{
			for(Component * c=gui.getComponentAtPos(absPos);c!=nullptr;c=c->getParent()){
				if(c->hasMouseCursorProperty())
					return gui.getStyleManager().getMouseCursor(c->getMouseCursorProperty());
			}
			return nullptr;
		}
		// if no cursor is given, the systems's default cursor is used (internally represented by nullptr)
		void activateCursor(std::shared_ptr<Util::UI::Cursor> cursor){
			if(gui.getWindow()!=nullptr && gui.getWindow()->getCursor() != cursor)
				gui.getWindow()->setCursor(std::move(cursor));
		}
};

// ----------
// ---- Tooltip
/**
 * \todo move TooltipHandler to Overlay
 */
class TooltipHandler : public Component {
		Util::Reference<Component> activeComponent;
		float startingTime;
		Vec2 lastMousePos;
		std::string text;
		enum mode_t{
			SEARCHING,ACTIVE,INACTIVE
		} mode;
		FrameListenerHandle frameListenerHandle;
		MouseMotionListener mouseMotionListener;
	public:

		TooltipHandler(GUI_Manager & _gui) : 
			Component(_gui),
			startingTime(0),
			mode(SEARCHING),
			frameListenerHandle(_gui.addFrameListener(std::bind(&TooltipHandler::onFrame,
																this,
																std::placeholders::_1))),
			mouseMotionListener(createMouseMotionListener(_gui, this, &TooltipHandler::onMouseMove)) {
		}
		virtual ~TooltipHandler() {
			getGUI().removeFrameListener(std::move(frameListenerHandle));
		}

		Component * findTooltitComponent(const Vec2 & pos)const{
			for(Component * c=getGUI().getComponentAtPos(pos);c!=nullptr;c=c->getParent()){
				if(hasComponentTooltip(*c))
					return c;
			}
			return nullptr;
		}

		bool onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
			const Geometry::Vec2 absPos(motionEvent.x, motionEvent.y);
			if(lastMousePos == absPos) {
				return false;
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
			return false;
		}
		void onFrame(double timeSecs) {
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


			this->text = getComponentTooltip(*activeComponent.get());
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
GUI_Manager::GUI_Manager(Util::UI::EventContext * context) : 
		eventContext(context), window(nullptr), debugMode(0),
		lazyRendering(false), style(new StyleManager),
		mouseCursorHandler(new MouseCursorHandler(*this)),
		tooltipHandler(new TooltipHandler(*this)){
	globalContainer=new GlobalContainer(*this,Rect(0,0,1280,1024));
	globalContainer->setMouseCursorProperty(PROPERTY_MOUSECURSOR_DEFAULT);
	initHoverPropertyHandler(*this,*globalContainer.get());
	
	Style::initStyleManager(getStyleManager());
}

//! (dtor)
GUI_Manager::~GUI_Manager() {
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
	return eventContext ? eventContext->isCtrlPressed() : false;
}

bool GUI_Manager::isShiftPressed() const {
	return eventContext ? eventContext->isShiftPressed() : false;
}

//! (internal)
bool GUI_Manager::handleMouseMovement(const Util::UI::MotionEvent & motionEvent){
	// Use a copy to allow insertions and deletions.
	for(const auto & handleMouseMoveFun : globalMouseMotionListener.getElementsCopy()) {
		if(handleMouseMoveFun(nullptr, motionEvent)) {
			return true;
		}
	}
	return false;
}

//! (internal)
bool GUI_Manager::handleMouseButton(const Util::UI::ButtonEvent & buttonEvent) {
	// Handle global listeners
	// Use nullptr as component to access global registry.
	const auto globalIt = mouseButtonListener.find(nullptr);
	if(globalIt != mouseButtonListener.cend()) {
		// Use a copy to allow insertions and deletions.
		for(const auto & handleMouseButtonFun : globalIt->second.getElementsCopy()) {
			if(handleMouseButtonFun(nullptr, buttonEvent)) {
				return true;
			}
		}
	}

	Component * lastActive=getActiveComponent();
	setActiveComponent(nullptr);

	const Geometry::Vec2 absPos(buttonEvent.x, buttonEvent.y);
	for(Component::Ref c=getComponentAtPos(absPos);
			c.isNotNull() && c->isEnabled() && c->coversAbsPosition(absPos);
			c=c->getParent() ){

		const auto componentIt = mouseButtonListener.find(c.get());
		if(componentIt == mouseButtonListener.cend()) {
			continue;
		}
		for(const auto & handleMouseButtonFun : componentIt->second.getElementsCopy()) {
			if(!(handleMouseButtonFun(c.get(), buttonEvent))) {
				continue;
			}

			c->bringToFront();
			if (buttonEvent.pressed || c!=lastActive)
				return true;

			const auto clickIt = mouseClickListener.find(c.get());
			if(clickIt != mouseClickListener.cend()) {
				const Geometry::Vec2 localPos = absPos - c->getAbsPosition();
				// Use a copy to allow insertions and deletions.
				for(const auto & clickListener : clickIt->second.getElementsCopy()) {
					if(clickListener(c.get(), buttonEvent.button, localPos)) {
						break;
					}
				}
			}
			return true;
		}
	}
	if(buttonEvent.pressed) {
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
		const auto it = keyListener.find(c.get());
		if(it != keyListener.cend()) {
			// Use a copy to allow insertions and deletions.
			for(const auto & fun : it->second.getElementsCopy()) {
				const bool consumed = fun(keyEvent);
				if(consumed) {
					return true;
				}
			}
		}
	}
	if(!keyEvent.pressed)
		return false;
	else if(keyEvent.key == Util::UI::KEY_TAB){
		Component::Ref selectedComponent = globalContainer->findSelectedComponent();	// hold reference as unselecting may trigger destruction
		if(selectedComponent && selectedComponent->hasParent()){
			selectedComponent->getParent()->unselectSubtree();
		}
		if(isShiftPressed()){
			selectPrev(selectedComponent.get());
		}else{
			selectNext(selectedComponent.get());
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

void GUI_Manager::display(Rendering::RenderingContext& rc){
	
	{ // init draw process
		// update size
		Geometry::Rect_i viewport = rc.getViewport();
		globalContainer->setSize(viewport.getWidth(), viewport.getHeight());
		Draw::beginDrawing(rc, Geometry::Vec2i(viewport.getWidth(),viewport.getHeight()));
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
		const double time = Util::Timer::now();
		// Use a copy to allow insertions and deletions.
		for(const auto & fun : frameListener.getElementsCopy()) {
			fun(time);
		}

		//! key repetition \note !!! This is not the proper place to do this, as this may introduce side effects !!!!
		if(keyRepeatInfo.get()!=nullptr && keyRepeatInfo->first < time){
			keyRepeatInfo->first = time + getGlobalValue(PROPERTY_KEY_REPEAT_DELAY_2);
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
	while( c ){
		if( c->getNext() ){
			c = c->getNext();
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

void GUI_Manager::componentActionPerformed(Component * c, const Util::StringIdentifier & actionName) {
	// Use a copy to allow insertions and deletions.
	for(const auto & handleAction : actionListener.getElementsCopy()) {
		if(handleAction(c, actionName)) {
			return;
		}
	}
}

void GUI_Manager::componentDataChanged(Component * component) {
	// Inform component's data change listener
	const auto componentIt = dataChangeListener.find(component);
	if(componentIt != dataChangeListener.cend()) {
		// Use a copy to allow insertions and deletions.
		for(const auto & changeListener : componentIt->second.getElementsCopy()) {
			changeListener(component);
		}
	}
	// Inform global data change listeners
	// Use nullptr as component to access global registry.
	const auto globalIt = dataChangeListener.find(nullptr);
	if(globalIt != dataChangeListener.cend()) {
		// Use a copy to allow insertions and deletions.
		for(const auto & changeListener : globalIt->second.getElementsCopy()) {
			changeListener(component);
		}
	}
}

void GUI_Manager::componentDestruction(const Component * component) {
	// Inform functions listening for a component's destruction
	const auto componentIt = componentDestructionListener.find(component);
	if(componentIt != componentDestructionListener.cend()) {
		// Use a copy to allow insertions and deletions.
		for(const auto & onComponentDestruction : componentIt->second.getElementsCopy()) {
			onComponentDestruction();
		}
		componentDestructionListener.erase(componentIt);
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
	animationHandlerList.emplace_back(h);
	h->setStartTime( Util::Timer::now() );
}

//! (internal)
void GUI_Manager::executeAnimations(){
	const float t = Util::Timer::now();
	animationHandlerList_t animationHandlerList2;
	animationHandlerList2.reserve( animationHandlerList.size() );
	std::swap(animationHandlerList2,animationHandlerList);

	while( !animationHandlerList2.empty() ){
		std::unique_ptr<AnimationHandler> handler = std::move(animationHandlerList2.back());
		animationHandlerList2.pop_back();
		if( handler->animate(t) ){
			handler->updateLastTime(t);
			animationHandlerList.emplace_back( std::move(handler) );
		}
	}
}

//! (internal)
void GUI_Manager::finishAnimations(Component * c){
	animationHandlerList_t animationHandlerList2;
	animationHandlerList2.reserve( animationHandlerList.size() );
	std::swap(animationHandlerList2,animationHandlerList);	
	
	while( !animationHandlerList2.empty() ){
		std::unique_ptr<AnimationHandler> handler = std::move(animationHandlerList2.back());
		animationHandlerList2.pop_back();
		if( handler->getComponent()==c ){
			handler->finish();
		}else{
			animationHandlerList.emplace_back( std::move(handler) );
		}
	}
}
//! (internal)
void GUI_Manager::stopAnimations(Component * c){
	animationHandlerList_t animationHandlerList2;
	animationHandlerList2.reserve( animationHandlerList.size() );
	std::swap(animationHandlerList2,animationHandlerList);	
	
	while( !animationHandlerList2.empty() ){
		std::unique_ptr<AnimationHandler> handler = std::move(animationHandlerList2.back());
		animationHandlerList2.pop_back();
		if( handler->getComponent()!=c )
			animationHandlerList.emplace_back( std::move(handler) );
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
void GUI_Manager::disableProperty(const Util::Reference<DisplayProperty> & p)const{
	p->disable(getStyleManager());
}
void GUI_Manager::enableProperty(const Util::Reference<DisplayProperty> & p)const{
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

//! [factory] Checkbox
Checkbox * GUI_Manager::createCheckbox(const std::string & text/*=""*/,bool checked/*=false*/,flag_t flags/*=0*/){
	return new Checkbox(*this,checked,text,flags);
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
Panel * GUI_Manager::createPanel(flag_t flags/*=0*/){
	return new Panel(*this,flags);
}

//! [factory] Slider
Slider * GUI_Manager::createSlider(const Geometry::Rect & r,float left/*=0*/,float right/*=1*/,int steps/*=10*/,flag_t flags/*=0*/){
	return new Slider(*this,r,left,right,steps,flags);
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

//! [factory] Textarea
Textarea * GUI_Manager::createTextarea(const std::string &text/*=""*/,flag_t flags/*=0*/){
	auto c = new Textarea(*this,flags);
	c->setText(text);
	return c;
}
//! [factory] TextField
Textfield * GUI_Manager::createTextfield(const std::string &text/*=""*/,flag_t flags/*=0*/){
	return new Textfield(*this,text,flags);
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
