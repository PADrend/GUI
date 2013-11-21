/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "ListView.h"
#include "../GUI_Manager.h"
#include "../Base/AnimationHandler.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "Scrollbar.h"
#include "ComponentPropertyIds.h"
#include <Util/UI/Event.h>
#include <algorithm>
#include <functional>
#include <iostream>

namespace GUI {
	
static const Util::StringIdentifier dataId_scrollPos("scroll");
static const Util::StringIdentifier dataId_marking("ListViewMarking");

const ListView::flag_t ListView::AT_LEAST_ONE_MARKING = 1 << 24;
const ListView::flag_t ListView::AT_MOST_ONE_MARKING = 1 << 25;

// ------------------------------------------------------------------------------
// ClientArea

// ---|> Component
void ListView::ListViewClientArea::doLayout() {

	const size_t numEntries(myListView.getNumVisibleEntries());

	Component * c = myListView.getEntry(myListView.getEntryIndexByPosition(myListView.getScrollPos()));
	for( size_t i = 0 ; i < numEntries && c != nullptr ; ++i, c = c->getNext() ) {
		c->layout();
		c->setWidth(getWidth());
	}
}

// ---|> Component
void ListView::ListViewClientArea::doDisplay(const Geometry::Rect & region) {
	const Geometry::Rect myRegion( region.isValid() ? getAbsRect().clipBy(region) : getAbsRect() );

	const size_t numEntries(myListView.getNumVisibleEntries());
	Geometry::Rect markerRect(0, 0, getWidth(), myListView.getEntryHeight());
	Component * c = myListView.getEntry(myListView.getEntryIndexByPosition(myListView.getScrollPos()));
	const Component * comp_cursor = myListView.getCursorEntry();
	for( size_t i = 0 ; i < numEntries && c != nullptr ; ++i, c = c->getNext() ) {
		if(myListView.isMarked(c)) {
			markerRect.setPosition(c->getPosition());
			getGUI().displayShape(PROPERTY_LISTVIEW_MARKED_ENTRY_SHAPE, markerRect);
		}
		if(c == comp_cursor && myListView.isSelected()) {
			markerRect.setPosition(c->getPosition());
			getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE, markerRect);
		}
		if(c->isEnabled() && myRegion.intersects(c->getAbsRect()))
			c->display(region);
	}
}

// ------------------------------------------------------------------------------

//! (ctor)
ListView::ListView(GUI_Manager & _gui, flag_t _flags/*=0*/) :
	Container(_gui, _flags), DataChangeListener(), MouseButtonListener(), MouseMotionListener(),
	entryHeight(getGUI().getGlobalValue(PROPERTY_LISTVIEW_DEFAULT_ENTRY_HEIGHT)),
	clientArea(new ListViewClientArea(_gui, *this)), cursor(0),
	keyListenerHandle(_gui.addKeyListener(this, std::bind(&ListView::onKeyEvent, 
														  this, 
														  std::placeholders::_1))),
	initialMarkingIndex(0) {

	clientArea->setFlag(IS_CLIENT_AREA, true);
	_addChild(clientArea.get());
	clientArea->addMouseButtonListener(this);
	setFlag(USE_SCISSOR, true);
	setFlag(SELECTABLE, true);
}

void ListView::assertIsChild(Component * c)const {
	if(c == nullptr || clientArea.get() != c->getParent())
		throw std::invalid_argument("Given component is no child of this ListView.");
}

//! (dtor)
ListView::~ListView() {
	getGUI().removeMouseMotionListener(this);
	getGUI().removeKeyListener(this, std::move(keyListenerHandle));
}

//! ---|> Component
void ListView::doLayout() {

	clientArea->setHeight(entryHeight * getContentsCount());

	// \note the scrolling in x direction is currently unused (but is left for later extensions)
	maxScrollPos = Geometry::Vec2( std::max(0.0f,clientArea->getWidth() - getWidth()) , std::max(0.0f, clientArea->getHeight() - getHeight() ));
	

	if(scrollPos.y() < 0 )	scrollPos.y(0);
	if(scrollPos.y() > maxScrollPos.y() )scrollPos.y(maxScrollPos.y());

	if(scrollPos.x() < 0 )	scrollPos.x(0);
	if(scrollPos.x() > maxScrollPos.x() )scrollPos.x(maxScrollPos.x());

	// set scrollPosition
	clientArea->setPosition(-scrollPos);

	if (maxScrollPos.y() <= 0) {
		if(scrollBar.isNotNull()){
			Component::destroy(scrollBar.get());
			scrollBar = nullptr;
		}
		clientArea->setWidth(getWidth());
	} else {
		if(scrollBar.isNull()){
			scrollBar = new Scrollbar(getGUI(), dataId_scrollPos, Scrollbar::VERTICAL);
			scrollBar->addDataChangeListener(this);
			scrollBar->setExtLayout( 	ExtLayouter::POS_X_ABS|ExtLayouter::REFERENCE_X_RIGHT|ExtLayouter::ALIGN_X_RIGHT|
						ExtLayouter::POS_Y_ABS|ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::ALIGN_Y_TOP |
						ExtLayouter::WIDTH_ABS|ExtLayouter::HEIGHT_ABS,
						Geometry::Vec2(1,2),Geometry::Vec2(getGUI().getGlobalValue(PROPERTY_SCROLLBAR_WIDTH),-4));
			_addChild(scrollBar.get());
		}
		clientArea->setWidth(getWidth() - scrollBar->getWidth());
	}

	
	if(scrollBar.isNotNull()){
		scrollBar->setMaxScrollPos(std::max(0,static_cast<int>(maxScrollPos.y()))); 
		scrollBar->setScrollPos( scrollPos.y() );
	}
	

}

//! ---|> Component
void ListView::doDisplay(const Geometry::Rect & region) {
	getGUI().displayShape(PROPERTY_LISTVIEW_SHAPE, getLocalRect(), 0);
	displayChildren(region,true);
	if(scrollPos.y()>0)
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE, getLocalRect(), 0);
	if(scrollPos.y()<maxScrollPos.y())
		getGUI().displayShape(PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE, getLocalRect(), 0);
}



// ------------------------------------------------------------------
// Children

//! ---|> Container
void ListView::addContent(const Ref & child)		{
	clientArea->addContent(child);
	entryRegistry.push_back(child.get());
	resetPositions(static_cast<int>(getContentsCount()) - 1);
	if(markingList.empty() && getFlag(AT_LEAST_ONE_MARKING)) {
		addMarking(child.get());
	}
	invalidateLayout();
}
//! ---|> Container
void ListView::clearContents() {
	clearMarkings(true);
	clientArea->clearContents();
	rebuildRegistry();
	invalidateLayout();
}

size_t ListView::getEntryIndexByPosition(const Geometry::Vec2 & p)const {
	if(p.getX() < 0 || p.getY() < 0 || entryRegistry.empty())
		return npos;
	return std::min( static_cast<size_t>(entryRegistry.size() - 1), static_cast<size_t>(p.getY() / getEntryHeight() ));
}

//! ---|> Container
void ListView::removeContent(const Ref & child)	{
	assertIsChild(child.get());
	clientArea->removeContent(child);
	rebuildRegistry();
	resetPositions(0);
	removeMarking(child.get(), true);
	if(getFlag(AT_LEAST_ONE_MARKING) && markingList.empty() && !entryRegistry.empty()) {
		addMarking(entryRegistry.front());
	}
	invalidateLayout();
}
//! ---|> Container
void ListView::insertAfter(const Ref & child, const Ref & after)	{
	clientArea->insertAfter(child, after);
	rebuildRegistry();
	resetPositions(0);
}
//! ---|> Container
void ListView::insertBefore(const Ref & child, const Ref & after) {
	clientArea->insertBefore(child, after);
	rebuildRegistry();
	resetPositions(0);
}
void ListView::resetPositions(size_t beginningIndex) {
	for(size_t i = beginningIndex; i < entryRegistry.size(); ++i)
		entryRegistry[i]->setPosition(Geometry::Vec2(0, i * entryHeight));
}
void ListView::rebuildRegistry() {
	entryRegistry.clear();
	for(Component * c = clientArea->getFirstChild(); c != nullptr; c = c->getNext())
		entryRegistry.push_back(c);
}

// ------------------------------------------------------------------
// Cursor
void ListView::scrollToCursor() {
	Component * c = getCursorEntry();
	if(c == nullptr) {
		return;
	} else if( c->getPosition().getY() - getEntryHeight() < getScrollPos().getY() ) {
		scrollTo(c->getPosition() + Geometry::Vec2(0, -getEntryHeight()), 0.1);

	} else if( c->getPosition().getY() + getEntryHeight() * 1.5 > getScrollPos().getY() + getHeight() ) {
		scrollTo(c->getPosition() + Geometry::Vec2(0, 1.5 * getEntryHeight() - getHeight()), 0.1);
	}
}

void ListView::moveCursor(int delta) {
	if(entryRegistry.empty()) {
		setCursorIndex(0);
	} else if(delta < 0) {
		setCursorIndex( static_cast<size_t>(std::max(static_cast<int>(getCursorIndex()) + delta, 0)) );
	} else {
		setCursorIndex( static_cast<size_t>(std::min(getCursorIndex() + delta, entryRegistry.size() - 1)) );
	}
}

// ------------------------------------------------------------------
// Events


//! ---|> DataChangeListener
void ListView::handleDataChange(Component *, const Util::StringIdentifier & actionName) {
	if(scrollBar.isNotNull() && actionName==dataId_scrollPos){
		setScrollingPosition( Geometry::Vec2(0.0f,scrollBar->getScrollPos() ));
	}
}

//! ---|> MouseButtonListener
listenerResult_t ListView::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition()+scrollPos;
	if(buttonEvent.pressed)
		select();
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_MIDDLE ) {
		if(maxScrollPos.x() <= 0 && maxScrollPos.y() <= 0)
			return LISTENER_EVENT_NOT_CONSUMED;
		else if(buttonEvent.pressed) {
			getGUI().addMouseMotionListener(this);
		} else {// !pressed
			getGUI().removeMouseMotionListener(this);
		}
		return LISTENER_EVENT_CONSUMED;
	} else if(buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT && buttonEvent.pressed) {
		const size_t index = getEntryIndexByPosition(localPos);
		if(index != npos) {
			performMarkingAction(getEntryIndexByPosition(localPos), getGUI().isCtrlPressed(), getGUI().isShiftPressed());
			setCursorIndex(index);
		}
		return LISTENER_EVENT_CONSUMED;
	} else if(buttonEvent.button == Util::UI::MOUSE_BUTTON_RIGHT && buttonEvent.pressed) {
		clearMarkings();
		return LISTENER_EVENT_CONSUMED;
	} else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN && buttonEvent.pressed) {
		finishScrolling();
		const float amount = std::min(getEntryHeight() * 3.0, getHeight() * 0.33);
		scrollTo(scrollPos + Geometry::Vec2(0, amount), 0.1);
		return LISTENER_EVENT_CONSUMED;
	} else if(buttonEvent.button == Util::UI::MOUSE_WHEEL_UP && buttonEvent.pressed) {
		finishScrolling();
		const float amount = std::min(getEntryHeight() * 3.0, getHeight() * 0.33);
		scrollTo(scrollPos + Geometry::Vec2(0, -amount), 0.1);
		return LISTENER_EVENT_CONSUMED;
	} else {
		return LISTENER_EVENT_NOT_CONSUMED;
	}
}

//! ---|> MouseMotionListener
listenerResult_t ListView::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
	if (!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_MIDDLE)) {
		return LISTENER_EVENT_NOT_CONSUMED_AND_REMOVE_LISTENER;
	}
	const Geometry::Vec2 delta(motionEvent.deltaX, motionEvent.deltaY);
	setScrollingPosition(scrollPos - delta * 2.0);
	return LISTENER_EVENT_CONSUMED;
}

bool ListView::onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if(keyEvent.pressed) {
		select();
		if(keyEvent.key == Util::UI::KEY_UP) {
			moveCursor(-1);
			scrollToCursor();
			return true;
		} else if(keyEvent.key == Util::UI::KEY_DOWN) {
			moveCursor(1);
			scrollToCursor();
			return true;
		} else if(keyEvent.key == Util::UI::KEY_HOME) {
			setCursorIndex(0);
			scrollToCursor();
			return true;
		} else if(keyEvent.key == Util::UI::KEY_END) {
			setCursorIndex(entryRegistry.size() - 1);
			scrollToCursor();
			return true;
		} else if(keyEvent.key == Util::UI::KEY_PAGEUP) {
			moveCursor(getNumVisibleEntries() * -0.75);
			scrollToCursor();
			return true;
		} else if(keyEvent.key == Util::UI::KEY_PAGEDOWN) {
			moveCursor(getNumVisibleEntries() * 0.75);
			scrollToCursor();
			return true;
		} else if(keyEvent.key == Util::UI::KEY_SPACE) {
			performMarkingAction(getCursorIndex(), getGUI().isCtrlPressed(), getGUI().isShiftPressed());
			return true;
		}
	}
	return false;
}

// ------------------------------------------------------------------
// Scrolling

//! ListView_ScrollAnimation ---|> AnimationHandler
class ListView_ScrollAnimation: public AnimationHandler {
	public:
		Geometry::Vec2 targetPos;

		ListView_ScrollAnimation(ListView * p, Geometry::Vec2 _targetPos, float _duration) :
			AnimationHandler(p, _duration), targetPos(std::move(_targetPos)) {
		}
		virtual ~ListView_ScrollAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float t) override {
			if(t > getEndTime()) {
				finish();
				return false;
			}
			ListView * p = dynamic_cast<ListView *>(getComponent());

			for(float f = 0.0; f < t - getStartTime(); f += 0.01) // for every 10 ms
				p->setScrollingPosition( (p->getScrollPos() * 99.0f + targetPos) * 0.01f);
			return true;
		}

		// ---|> AnimationHandler
		void finish() override {
			dynamic_cast<ListView *>(getComponent())->setScrollingPosition(targetPos);
		}
};

void ListView::scrollTo(const Geometry::Vec2 & pos, float duration) {
	getGUI().stopAnimations(this);
	getGUI().addAnimationHandler(new ListView_ScrollAnimation(this, pos, duration));
}

void ListView::setScrollingPosition(const Geometry::Vec2 & pos) {
	const Geometry::Vec2 oldScrollPos = scrollPos;
	scrollPos.x( std::max( std::min( pos.x(), maxScrollPos.x() ) , 0.0f ) );
	scrollPos.y( std::max( std::min( pos.y(), maxScrollPos.y() ) , 0.0f ) );
	if(scrollPos!=oldScrollPos){
		clientArea->invalidateLayout();
		invalidateLayout();
	}
}
//! (internal)
void ListView::finishScrolling() {
	getGUI().finishAnimations(this);
}
// -------------------------------------------------------------------
// Markings

void ListView::addMarking(Component * c) {
	doAddMarking(c);
	markingChanged();
}

void ListView::clearMarkings(bool forced) {
	if(doClearMarking(forced)) {
		markingChanged();
	}
}

//! (internal)
void ListView::doAddMarking(Component * c) {
	assertIsChild(c);
	std::pair<markingSet_t::iterator, bool> result = markingSet.insert(c);

	// component was already marked
	if(!result.second) {
		markingList.remove(c);
	}

	markingList.push_back(c);
	if(getFlag(AT_MOST_ONE_MARKING)) {
		while(markingList.size() > 1) {
			doRemoveMarking(markingList.front(), true);
		}
	}
}

//! (internal)
bool ListView::doClearMarking(bool forced) {
	if(markingSet.empty())
		return false;
	if(getFlag(AT_LEAST_ONE_MARKING) && !forced ) {
		Component * c = markingList.back();
		markingSet.clear();
		markingList.clear();
		doAddMarking(c);
	} else {
		markingSet.clear();
		markingList.clear();

	}
	return true;
}

//! (internal)
bool ListView::doRemoveMarking(Component * c, bool forced) {
	assertIsChild(c);
	if(getFlag(AT_LEAST_ONE_MARKING) && markingList.size() == 1 && !forced) {
		return false;
	}
	if(markingSet.erase(c)) {
		markingList.remove(c);
		return true;
	}
	return false;
}


//! ---o
void ListView::markingChanged() {
	getGUI().componentDataChanged(this, dataId_marking); //dataName
}

//! (internal)
void ListView::performMarkingAction(const size_t index, const bool accumulative, const bool grouping) {
	if(index >= entryRegistry.size())
		return;

	// l-button + shift (+ ctrl) -> add or remove marking from initial index to current (based on marking of initial index)
	if( grouping ) {
		initialMarkingIndex = std::min(static_cast<size_t>(entryRegistry.size() - 1), initialMarkingIndex);
		bool doMark = true;

		// ctrl is pressed, do not erase the prior marking but take status of initial index as basis
		if( accumulative ) {
			doMark = isMarked(entryRegistry[initialMarkingIndex]);
		} else {
			doClearMarking(true);
		}

		const size_t start = std::min(index, initialMarkingIndex);
		const size_t end = std::max(index, initialMarkingIndex);
		if(doMark) {
			for(size_t i = start; i <= end; ++i)
				doAddMarking(entryRegistry[i]);
		} else {
			for(size_t i = start; i <= end; ++i)
				doRemoveMarking(entryRegistry[i], false);
		}
		markingChanged();
	} // l-button + ctrl -> toggle (and store initial index)
	else if( accumulative ) {
		initialMarkingIndex = index;
		Component * c = entryRegistry[index];
		if(isMarked(c))
			removeMarking(c);
		else
			addMarking(c);
	}// l-button -> set marking (and store initial index)
	else {
		initialMarkingIndex = index;
		setMarking(entryRegistry[index]);
	}
}


void ListView::removeMarking(Component * c, bool forced) {
	if(doRemoveMarking(c, forced)) {
		markingChanged();
	}
}

void ListView::setMarking(Component * c) {
	assertIsChild(c);
	if(markingSet.size() == 1 && isMarked(c))
		return;
	doClearMarking(true);
	doAddMarking(c);
	markingChanged();
}
void ListView::setMarkings(const markingList_t & newMarkings){
	if(newMarkings == markingList)
		return;
	doClearMarking(true);
	std::for_each(newMarkings.begin(), newMarkings.end(), 
				  std::bind(&ListView::doAddMarking, this, std::placeholders::_1));
	markingChanged();
}
}
