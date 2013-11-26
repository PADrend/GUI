/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "EditorPanel.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "../Base/ListenerHelper.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "../Style/Colors.h" // \todo Remove this!
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>

namespace GUI{
using namespace Geometry;

//! Component encloses its children and is at least (almost) as big as its parent.
class FillLayouter : public AbstractLayouter{
	PROVIDES_TYPE_NAME(FillLayouter)

	public:
		FillLayouter() : AbstractLayouter(){}
		FillLayouter(const FillLayouter & ) = default;
		virtual ~FillLayouter(){}
	
		
		//! ---|> AbstractLayouter
		void layout(Util::WeakPointer<Component> component) override{
			Container * container = dynamic_cast<Container*>(component.get());
			if(!container)
				throw std::invalid_argument("FillLayouter can only be applied to Containers.");
			float width = 0;
			float height = 0;
			
			for(Component * child=container->getFirstChild();child!=nullptr;child=child->getNext()){
				width = std::max(width,child->getPosition().x()+child->getWidth() );
				height = std::max(height,child->getPosition().y()+child->getHeight() );
			}
			if(container->hasParent()){
				width = std::max(width,container->getParent()->getWidth()-1 );
				height = std::max(height,container->getParent()->getHeight()-1 );
			}
			container->setSize(width,height);
		}

};

static FillLayouter * getDefaultLayouter(){
	static Util::Reference<FillLayouter> defaultLayouter = new FillLayouter();
	return defaultLayouter.get();
}

// --------------------------------------------------------------------------------------------------

//! (ctor)
EditorPanel::EditorPanel(GUI_Manager & _gui,flag_t _flags/*=0*/) :
		Container(_gui,_flags), state(CLICK_SELECTING),
		mouseButtonListener(createMouseButtonListener(_gui, this, &EditorPanel::onMouseButton)),
		optionalMouseMotionListener(createOptionalMouseMotionListener(_gui, this, &EditorPanel::onMouseMove)) {
	setFlag(USE_SCISSOR,true);
	setFlag(SELECTABLE,true);
	setFlag(BORDER,true);
	addLayouter(getDefaultLayouter());
}


//! (dtor)
EditorPanel::~EditorPanel() = default;

//! ---|> Component
void EditorPanel::doDisplay(const Geometry::Rect & region) {
	for(const auto & markedChild : getMarkedChildren()) {
		if(isSelected()) {
			Draw::drawLineRect(markedChild->getRect().changeSizeCentered(4, 4), Colors::ACTIVE_COLOR_3, false);
		} else {
			Draw::drawLineRect(markedChild->getRect().changeSizeCentered(4, 4), Colors::PASSIVE_COLOR_2, false);
		}
	}

	displayChildren(region);
	if(state==DRAG_SELECTING){
		Rect r=Rect(dragStartPos.x(),dragStartPos.y(),0,0);
		r.include(dragPos.x(),dragPos.y());
		Draw::drawLineRect( r,Colors::ACTIVE_COLOR_3,false);
	}
}

//! ---|> MouseMotionListener
bool EditorPanel::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent){
	switch(state){
		case CLICK_SELECTING:{
			optionalMouseMotionListener.disable();
			return false;
		}
		case DRAG_SELECTING:{
			if(!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT)) {
				state=CLICK_SELECTING;
				// rectSelect_break(); // cant remove mouseListener inside onMouseMove
				return false;
			}
			dragPos = Geometry::Vec2(motionEvent.x, motionEvent.y) - getAbsPosition();
			break;
		}
		case MOVING:{
			if(!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_LEFT)) {
				state=CLICK_SELECTING;
			}else {
				move_execute(Geometry::Vec2(motionEvent.deltaX, motionEvent.deltaY));
			}
			break;
		}
		default:
			WARN("unexpected case in switch statement");

	}
	return false;
}

bool EditorPanel::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if(buttonEvent.pressed && getGUI().isCtrlPressed()) {
		if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP) {
			float scale=0.8;
			for(Component * child=getFirstChild();child!=nullptr;child=child->getNext()){
				child->setPosition( child->getPosition()*scale);
				child->setWidth(child->getWidth()*scale);
				child->setHeight(child->getHeight()*scale);
			}
			return true;
		} else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {

			float scale=1.0/0.8;
			for(Component * child=getFirstChild();child!=nullptr;child=child->getNext()){
				child->setPosition( child->getPosition()*scale);
				child->setWidth(child->getWidth()*scale);
				child->setHeight(child->getHeight()*scale);
			}
			return true;
		}
	}
	const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
	switch(state){
		case CLICK_SELECTING:{
			if(buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
				select();
				for(const auto & markedChild : getMarkedChildren()) {
					if(markedChild->coversLocalPos(localPos - markedChild->getPosition())) {
						move_start(localPos);
						return true;
					}
				}
				for(Component * child=getFirstChild();child!=nullptr;child=child->getNext()){
					if(!child->coversLocalPos(localPos-child->getPosition()))
						continue;
					if(!getGUI().isShiftPressed())
						unmarkAll();
					markChild(child);
					getGUI().componentDataChanged(this);
					move_start(localPos);
					return true;
				}
				rectSelect_start(localPos);
//				getGUI().componentMouseClicked(this,button,pos,pressed); // ???
				return true;

			}else if(buttonEvent.button == Util::UI::MOUSE_BUTTON_RIGHT) {
				if(!getMarkedChildren().empty()){
					unmarkAll();
					getGUI().componentDataChanged(this);
				}
				return true;
			}
			break;
		}
		case DRAG_SELECTING:{
			if(!buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
				rectSelect_finish(localPos);
				return true;
			} else if(buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_BUTTON_RIGHT) {
				rectSelect_break();
				return true;
			}

			break;
		}
		case MOVING:{
			if(!buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
				move_finish();
				return true;
			} else if(buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_BUTTON_RIGHT) {
				move_break(localPos);
				return true;
			}
			break;
		}
		default:
			WARN("unexpected case in switch statement");
	}
	return false;
}

void EditorPanel::rectSelect_start(const Vec2 & pos){
	state=DRAG_SELECTING;
	dragStartPos=pos;
	dragPos=pos;
	optionalMouseMotionListener.enable();
}

void EditorPanel::rectSelect_break(){
	state=CLICK_SELECTING;
	optionalMouseMotionListener.disable();
}

void EditorPanel::rectSelect_finish(const Vec2 & pos){
	Rect r=Rect(dragStartPos.x(),dragStartPos.y(),0,0);
	r.include(pos.x(),pos.y());

	bool _markingChanged=false;
	// Clear selection
	if(!getGUI().isShiftPressed()) {
		bool markSomething=false;
		for(Component * child=getFirstChild();child!=nullptr;child=child->getNext()){
			if(r.contains(child->getRect())){
				markSomething=true;
				break;
			}
		}
		if(markSomething){
			unmarkAll();
			_markingChanged=true;
		}
	}

	// Select components
	for(Component * child=getFirstChild();child!=nullptr;child=child->getNext()){
		if(r.contains(child->getRect())){
			markChild(child);
			_markingChanged=true;
		}
	}

	state=CLICK_SELECTING;
	optionalMouseMotionListener.disable();
	if(_markingChanged)
		getGUI().componentDataChanged(this);
}

void EditorPanel::move_start(const Vec2 & pos){
	state=MOVING;
	dragStartPos=pos;
	dragPos=pos;
	optionalMouseMotionListener.enable();
}

void EditorPanel::move_execute(const Vec2 & delta){
	for(const auto & markedChild : getMarkedChildren()) {
		markedChild->moveRel(delta);
	}
	invalidateSubtreeLayout();
}

void EditorPanel::move_break(const Vec2 & pos){
	state=CLICK_SELECTING;
	optionalMouseMotionListener.disable();
	for(const auto & markedChild : getMarkedChildren()) {
		markedChild->moveRel(dragStartPos - pos);
	}
	invalidateSubtreeLayout();
}


void EditorPanel::move_finish(){
	state=CLICK_SELECTING;
	optionalMouseMotionListener.disable();
}


// ------------------------------------------------------------------------------

// ---- Marking

bool EditorPanel::markChild(Component * c){
	if(c!=nullptr && c->getParent() == this){
		return markedChildren.insert(c).second;
	}
	return false;
}


bool EditorPanel::unmarkChild(Component * c){
	return markedChildren.erase(c)>0;
}

void EditorPanel::unmarkAll(){
	markedChildren.clear();
}

void EditorPanel::markingChanged(){
	getGUI().componentDataChanged(this);
}

}
