/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Tab.h"
#include "../GUI_Manager.h"
#include "../Base/AbstractShape.h"
#include "../Base/Draw.h"
#include "../Base/Listener.h"
#include "../Base/Properties.h"
#include "ComponentPropertyIds.h"
#include "Window.h"
#include <Util/UI/Event.h>
#include <iostream>

namespace GUI {

/***
 **  TabTitlePanel ---|> Container ---|> Component
 **/
struct TabTitlePanel : public Container, public MouseMotionListener, public MouseButtonListener {
	TabbedPanel::Tab & myTab;
	GUI_Manager::KeyListenerHandle keyListenerHandle;
	TabTitlePanel(GUI_Manager & _gui,TabbedPanel::Tab & tab) :
			Container(_gui),MouseMotionListener(),MouseButtonListener(),myTab(tab),
			keyListenerHandle(_gui.addKeyListener(this, std::bind(&TabTitlePanel::onKeyEvent, 
																  this, 
																  std::placeholders::_1))) {
		setFlag(SELECTABLE,true);
		addMouseButtonListener(this);
	}
	virtual ~TabTitlePanel() {
		getGUI().removeMouseMotionListener(this);
		getGUI().removeKeyListener(this, std::move(keyListenerHandle));
	}
	TabbedPanel::Tab * getTab()const {
		return &myTab;
	}

	bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
		if(!keyEvent.pressed)
			return false;
		if(keyEvent.key == Util::UI::KEY_TAB) {
			TabbedPanel::Tab * t=getTab();
			if(t && t->clientArea()->getContentsCount()>0){
				if(getGUI().isShiftPressed())
					getGUI().selectLast(t->clientArea());
				else
					getGUI().selectFirst(t->clientArea());;
			}
			return true;
		}else if(keyEvent.key==Util::UI::KEY_LEFT) {
			TabbedPanel::Tab * t=getTab();
			if (t->getPrev())
				t=dynamic_cast<TabbedPanel::Tab *>(t->getPrev());
			else
				t=dynamic_cast<TabbedPanel::Tab *>(getTab()->getTabbedPanel()->getLastChild());
			if(t){
//			    t->makeActiveTab();
				t->getTitlePanel()->select();
			}
			return true;
		}else if(keyEvent.key==Util::UI::KEY_RIGHT) {
			TabbedPanel::Tab * t=getTab();
			if (t->getNext())
				t=dynamic_cast<TabbedPanel::Tab *>(t->getNext());
			else
				t=dynamic_cast<TabbedPanel::Tab *>(getTab()->getTabbedPanel()->getFirstChild());
			if(t){
//			    t->makeActiveTab();
				t->getTitlePanel()->select();
			}
			return true;
		}else if(keyEvent.key == Util::UI::KEY_RETURN || keyEvent.key == Util::UI::KEY_SPACE) {
			getTab()->makeActiveTab();
			return true;
		}
		return false;
	}

	//! ---|> MouseButtonListener
	listenerResult_t onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) override {
		if (buttonEvent.pressed) {
			if(buttonEvent.button == Util::UI::MOUSE_BUTTON_RIGHT && getGUI().isCtrlPressed()){
				TabbedPanel * oldTabbedPanel=getTab()->getTabbedPanel();
				Window * window=getGUI().createWindow(Geometry::Rect(buttonEvent.x, buttonEvent.y, getTab()->getWidth(), getTab()->getHeight()), "...");
				auto tp=new TabbedPanel(getGUI(),AUTO_MAXIMIZE);
				window->addContent(tp);
				tp->addContent(getTab());
				getTab()->invalidateRegion();
				getTab()->invalidateLayout();
				oldTabbedPanel->recalculateTabTitlePositions();
				return LISTENER_EVENT_CONSUMED;
			}
			getGUI().setActiveComponent(this);
			select();
			myTab.makeActiveTab();
			getGUI().addMouseMotionListener(this);
			return LISTENER_EVENT_CONSUMED;
		}
		return LISTENER_EVENT_NOT_CONSUMED;
	}
	//! ---|> MouseMotionListener
	listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) override{
		if (!isActive() || motionEvent.buttonMask == Util::UI::MASK_NO_BUTTON) {
			return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
		}
		const Geometry::Vec2 absPos = Geometry::Vec2(motionEvent.x, motionEvent.y);
		TabTitlePanel * ttp=dynamic_cast<TabTitlePanel *>(getGUI().getComponentAtPos(absPos));
		if (ttp) {// replace another tab
			if ( ttp==this)
				return LISTENER_EVENT_CONSUMED;
			TabbedPanel * tp=ttp->myTab.getTabbedPanel();
			if (tp==nullptr)   return LISTENER_EVENT_CONSUMED;
			if ( tp!=myTab.getTabbedPanel() ) {
				TabbedPanel * oldTabbedPanel=myTab.getTabbedPanel();
				tp->addContent(&myTab);
				tp->recalculateTabTitlePositions();
				oldTabbedPanel->recalculateTabTitlePositions();
				myTab.makeActiveTab();
				return LISTENER_EVENT_CONSUMED;
			}

			if (myTab.getPrev() != ttp->getTab()) {
				tp->insertAfter(&myTab,ttp->getTab());
			} else {
				// Flackert (wegem schnellen hi-und hertauschen)
//				tp->insertChildBefore(&myTab,ttp->getTab());
			}
			tp->recalculateTabTitlePositions();
			myTab.makeActiveTab();
			return LISTENER_EVENT_CONSUMED;
		}

		TabbedPanel * tp=dynamic_cast<TabbedPanel *>(getGUI().getComponentAtPos(absPos));
		if (tp==nullptr)
			return LISTENER_EVENT_CONSUMED;
//                    tp->insertTab(&myTab,0);
		TabbedPanel * oldTabbedPanel=myTab.getTabbedPanel();
		if (oldTabbedPanel==tp)
			return LISTENER_EVENT_CONSUMED;

		tp->addContent(&myTab);
		tp->recalculateTabTitlePositions();
		oldTabbedPanel->recalculateTabTitlePositions();
		myTab.makeActiveTab();

		return LISTENER_EVENT_CONSUMED;
	}

	//! ---|> Component
	void doDisplay(const Geometry::Rect & region) override {
		Util::Reference<AbstractProperty> c;

		if (myTab.isActiveTab()) {
			Geometry::Rect rect=getLocalRect();
			rect.changeSize(0,2);
			getGUI().displayShape(PROPERTY_TAB_HEADER_SHAPE,rect, AbstractShape::ACTIVE);

			c = new ColorProperty(PROPERTY_TEXT_COLOR,getGUI().getActiveColor(PROPERTY_TAB_HEADER_ACTIVE_TEXT_COLOR));
		}else{
			c = new ColorProperty(PROPERTY_TEXT_COLOR,getGUI().getActiveColor(PROPERTY_TAB_HEADER_PASSIVE_TEXT_COLOR));
			getGUI().displayShape(PROPERTY_TAB_HEADER_SHAPE,getLocalRect(), 0);
		}
		if(isSelected()) {
			Geometry::Rect rect = getLocalRect();
			rect.changeSizeCentered(-2, -2);
			getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,rect);
		}

		getGUI().enableProperty(c);
		displayChildren(region);
		getGUI().disableProperty(c);
	}
};

// ----------------------------------------------------------------------------------

//! (Tab] [ctor)
TabbedPanel::Tab::Tab(GUI_Manager & _gui,const std::string & _title,Container * _clientArea/*=0*/)
		:Container(_gui),clientAreaPanel(_clientArea),titlePanel(nullptr),titleTextLabel(nullptr) {
	setFlag(TRANSPARENT_COMPONENT,true);

	setFlag(AUTO_MAXIMIZE,true);

	titlePanel=new TabTitlePanel(getGUI(),*this);
	_addChild(titlePanel);

	titleTextLabel=new Label(getGUI());
	titlePanel->addContent(titleTextLabel);
	titleTextLabel->setTextStyle(Draw::TEXT_ALIGN_CENTER);


	if(clientAreaPanel==nullptr)
		clientAreaPanel=new Container(getGUI());
	_addChild(clientAreaPanel);
	clientAreaPanel->setFlag(USE_SCISSOR,true);
	
	clientAreaPanel->disable();

	setTitle(_title);
}

//! (Tab] [dtor)
TabbedPanel::Tab::~Tab() = default;

//! [Tab] ---|> Component
void TabbedPanel::Tab::doLayout() {
	const int titleHeight=static_cast<int>(getGUI().getGlobalValue(PROPERTY_TAB_HEADER_HEIGHT));

	int shift = 0;
	if (isActiveTab()) {
		shift = -2;
	}
	clientAreaPanel->setPosition(Geometry::Vec2(0,titleHeight+3));
	clientAreaPanel->setSize(getWidth(),getHeight()-titleHeight-3);

	titlePanel->setPosition(Geometry::Vec2(getTabTitlePos(),2+shift));
	titlePanel->setSize(titlePanel->getWidth(),titleHeight-shift);

	titleTextLabel->setPosition(Geometry::Vec2(2,2));
	titleTextLabel->setSize(titlePanel->getLocalRect().getWidth()-4,titlePanel->getLocalRect().getHeight());

}

//! [Tab] ---|> Component
void TabbedPanel::Tab::doDisplay(const Geometry::Rect & region) {
	if (clientAreaPanel->isEnabled()) {
		Geometry::Rect r = clientAreaPanel->getLocalRect();
		r.moveRel(clientAreaPanel->getPosition());
		getGUI().displayShape(PROPERTY_TAB_BODY_SHAPE,r);
	}
	displayChildren(region);
}

//! (Tab)
void TabbedPanel::Tab::setTitle(const std::string & title) {
	titleTextLabel->setText(title);

	titlePanel->setWidth(Draw::getTextWidth(title,getGUI().getActiveFont(PROPERTY_DEFAULT_FONT))+10);
	TabbedPanel * p=getTabbedPanel();
	if (p) {
		p->recalculateTabTitlePositions();
	}
}

//! (Tab)
std::string TabbedPanel::Tab::getTitle()const {
	return titleTextLabel->getText();
}

//! (Tab)
bool TabbedPanel::Tab::isActiveTab()const {
	TabbedPanel * tp=getTabbedPanel();
	if (tp)
		return tp->getActiveTab()==this;
	else
		return true;
}

//! (Tab)
TabbedPanel * TabbedPanel::Tab::getTabbedPanel()const {
	return dynamic_cast<TabbedPanel * >(getParent());
}

//! (Tab)
void TabbedPanel::Tab::makeActiveTab() {
	TabbedPanel * tp=getTabbedPanel();
	if (tp)
		return tp->setActiveTab(this);
	invalidateLayout();
	invalidateRegion();

}


// -------------------------------------------------------------------------------------------------------------------

//! (ctor)
TabbedPanel::TabbedPanel(GUI_Manager & _gui,flag_t _flags/*=0*/) : 
		Container(_gui,_flags),activeTab(nullptr) {
	//ctor
}


TabbedPanel::Tab * TabbedPanel::createTab(const std::string & name,Container * clientArea/*=0*/) {
	auto tab = new Tab(getGUI(), name, clientArea);
	tab->setRect(Geometry::Rect(0, 0, getWidth(), getHeight()));
	addContent(tab);
	if(!getActiveTab()) {
		setActiveTab(tab);
	}
	return tab;
}

void TabbedPanel::setActiveTab(Tab * tab) {
	if(activeTab && activeTab->getTabbedPanel()==this){
		activeTab->clientArea()->disable();
		activeTab->invalidateLayout();
	}
	activeTab=tab;
	invalidateLayout();
	if(activeTab) {
		activeTab->clientArea()->enable();
		activeTab->invalidateLayout();
	}
	recalculateTabTitlePositions();
}

//! ----|> Component
void TabbedPanel::bringChildToFront(Component * c) {
	Tab * t=dynamic_cast<Tab *>(c);
	if (c)
		setActiveTab(t);
	bringToFront();
}

void TabbedPanel::recalculateTabTitlePositions() {
	float pos=5;
	for (Component * c=getFirstChild();c!=nullptr;c=c->getNext()) {
		Tab * t=dynamic_cast<Tab*>(c);
		if (t!=nullptr) {
			t->setTabTitlePos(pos);
			pos+=t->getTabTitleWidth()+1;
		}
	}
	invalidateRegion();
	invalidateLayout();
}

//! ----|> Component
void TabbedPanel::doLayout() {
	if ( activeTab==nullptr || activeTab->getParent()!=this ) {
		setActiveTab(dynamic_cast<Tab*>(getLastChild()));
	}
	
}

//! ----|> Component
void TabbedPanel::addContent(const Ref & child) {
	_addChild(child);
	recalculateTabTitlePositions();
}

//! ----|> Component
void TabbedPanel::removeContent(const Ref & child) {
	Container::removeContent(child);
	recalculateTabTitlePositions();
}

void TabbedPanel::setActiveTabIndex(int nr){
	int currentIndex=0;
	for (Component * c=getFirstChild();c!=nullptr;c=c->getNext()) {
		Tab * t=dynamic_cast<Tab*>(c);
		if (t!=nullptr) {
			if(currentIndex == nr){
				setActiveTab(t);
				return;
			}
			++currentIndex;
		}
	}
}

int TabbedPanel::getActiveTabIndex()const{
	int currentIndex=0;
	for (Component * c=getFirstChild();c!=nullptr;c=c->getNext()) {
		const Tab * t=dynamic_cast<const Tab*>(c);
		if (t!=nullptr) {
			if( t==activeTab ){
				return currentIndex;
			}
			++currentIndex;
		}
	}
	return -1;
}

}
