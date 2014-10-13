/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "TreeView.h"
#include "../GUI_Manager.h"
#include "../Base/AbstractShape.h"
#include "../Base/AnimationHandler.h"
#include "../Base/ListenerHelper.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "../Base/StyleManager.h"
#include "ComponentPropertyIds.h"
#include "Scrollbar.h"
#include <Util/UI/Event.h>
#include <algorithm>
#include <functional>
#include <iostream>

namespace GUI {

//! TV_ScrollAnimation ---|> AnimationHandler
class TV_ScrollAnimation:public AnimationHandler{
	public:
		float targetPos;

		TV_ScrollAnimation(TreeView * tv,float _targetPos, float _duration) :
			AnimationHandler(tv,_duration),targetPos(_targetPos){
		}
		virtual ~TV_ScrollAnimation()	{}

		// ---|> AnimationHandler
		bool animate(float t) override{
			if(t > getEndTime()) {
				finish();
				return false;
			}
			TreeView * tv= dynamic_cast<TreeView *>(getComponent());
			tv->scrollTo( (tv->getScrollPos()*2 + targetPos ) /3.0 );
			return true;
		}

		// ---|> AnimationHandler
		void finish() override{
			dynamic_cast<TreeView *>(getComponent())->scrollTo(targetPos);
		}
};
// ------------------------------------------------------------------------------------------------------------

//! (TreeView::TreeViewEntry] [ctor)
TreeView::TreeViewEntry::TreeViewEntry(GUI_Manager & _gui,TreeView * _treeView,Component * c/*=nullptr*/,flag_t _flags/*=0*/):
		Container(_gui,_flags),
		myTreeView(_treeView),marked(false),
		mouseButtonListener(createMouseButtonListener(_gui, this, &TreeViewEntry::onMouseButton)) {
	if(c) {
		Container::_insertAfter(c,getLastChild());
	}
}

//! (TreeView::TreeViewEntry] [dtor)
TreeView::TreeViewEntry::~TreeViewEntry() {
	myTreeView = nullptr;
}

//! [TreeView::TreeViewEntry] ---|> Component
void TreeView::TreeViewEntry::doDisplay(const Geometry::Rect & region) {
	if(myTreeView==nullptr ||
			getAbsPosition().getY()+getHeight()<myTreeView->getAbsPosition().getY() ||
			getAbsPosition().getY()>myTreeView->getAbsPosition().getY()+myTreeView->getHeight() )
		return;

	enableLocalDisplayProperties();
	displayDefaultShapes();			
	
	if(isSelected())
		getGUI().displayShape(PROPERTY_TREEVIEW_ENTRY_SELECTION_SHAPE,getLocalRect());

	if(isMarked() && getFirstChild()) {
		Geometry::Rect r=getFirstChild()->getLocalRect();
		r.setWidth(myTreeView->getWidth());
		getGUI().displayShape(PROPERTY_TREEVIEW_ENTRY_MARKING_SHAPE,r);
	}
	auto shape_activeIndentation = getGUI().getStyleManager().getShape(PROPERTY_TREEVIEW_ACTIVE_INDENTATION_SHAPE);
	auto shape_passiveIndentation = getGUI().getStyleManager().getShape(PROPERTY_TREEVIEW_PASSIVE_INDENTATION_SHAPE);
	auto shape_subgroup = getGUI().getStyleManager().getShape(PROPERTY_TREEVIEW_SUBROUP_SHAPE);

	disableLocalDisplayProperties();

	for (Component * c=getFirstChild(); c; c=c->getNext()) {
		if(c->isEnabled()) {
			c->display(region);
			// if item is collapsed show only first element
			if(isCollapsed())
				break;
		}
	}
	if(getContentsCount() > 1 && dynamic_cast<TreeViewEntry*>(getParent())) {
		float h=getFirstChild()->getHeight();
		if(isCollapsed())
			shape_activeIndentation->display(Geometry::Rect(4,0,1,getHeight()));
		else
			shape_passiveIndentation->display(Geometry::Rect(4,0,1,getHeight()));

		shape_subgroup->display(Geometry::Rect(1,1,8,h-2),isCollapsed() ? 0 : AbstractShape::ACTIVE);
	}else{
		shape_activeIndentation->display(Geometry::Rect(4,0,1,getHeight()));
	}

}

bool TreeView::TreeViewEntry::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if(myTreeView==nullptr)
		return false;
	if(!buttonEvent.pressed)
		return false;
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_LEFT) {
		const Geometry::Vec2 localPos = Geometry::Vec2(buttonEvent.x, buttonEvent.y) - getAbsPosition();
		if(localPos.getX()<10&&localPos.getY()<20&& getContentsCount()>1) {
			if(isCollapsed())
				open();
			else
				collapse();
		} else if(getFirstChild() && localPos.getY()<getFirstChild()->getHeight()) {
			activate();
			select();
			if(getGUI().isCtrlPressed()) {
				if(isMarked())
					myTreeView->unmarkEntry(this);
				else
					myTreeView->markEntry(this);
			} else {
				myTreeView->unmarkAll();
				myTreeView->markEntry(this);
			}
			myTreeView->markingChanged();
		}
		return true;
	} else if(buttonEvent.button == Util::UI::MOUSE_BUTTON_RIGHT) {
		myTreeView->unmarkAll();
		myTreeView->markingChanged();
		return true;
	}
	return false;
}

const Util::StringIdentifier TreeView::TreeViewEntry::ACTION_TreeViewEntry_collapse("TreeViewEntry_collapse");
const Util::StringIdentifier TreeView::TreeViewEntry::ACTION_TreeViewEntry_open("TreeViewEntry_open");

void TreeView::TreeViewEntry::collapse(){
	getGUI().componentActionPerformed(this,ACTION_TreeViewEntry_collapse);
	setFlag(COLLAPSED_ENTRY, true);
	invalidateLayout();
	if(hasParent())
		getParent()->invalidateLayout();
	if(myTreeView){
		myTreeView->invalidateLayout();
		myTreeView->invalidateRegion();
	}
}
void TreeView::TreeViewEntry::open(){
	getGUI().componentActionPerformed(this,ACTION_TreeViewEntry_open);
	setFlag(COLLAPSED_ENTRY, false);
	invalidateLayout();
	if(hasParent())
		getParent()->invalidateLayout();
	if(myTreeView){
		myTreeView->invalidateLayout();
		myTreeView->invalidateRegion();
	}
}

//! [TreeView::TreeViewEntry] ---|> Container
void TreeView::TreeViewEntry::insertAfter(const Ref & child,const Ref & after) {
	if(child.isNull()) {
		return;
	} else if(!getFirstChild() && !(myTreeView && myTreeView->getRootEntry()==this) ){ // if the entry is empty and is not the root node, use the child as entry
		if(myTreeView!=nullptr)
			myTreeView->invalidateRegion();
		_insertAfter(child,after);
	}else{
		TreeViewEntry * e=dynamic_cast<TreeViewEntry*>(child.get());
		if(e==nullptr) {
			e=new TreeViewEntry(getGUI(),myTreeView,child.get());
		} else {
			e->setTreeView(myTreeView);
		}
		if(myTreeView!=nullptr)
			myTreeView->invalidateRegion();
		_insertAfter(e,after);
	}
}
//
//! [TreeView::TreeViewEntry] ---|> Container
void TreeView::TreeViewEntry::addContent(const Ref & child) {
	insertAfter(child,getLastChild());
}

//! [TreeView::TreeViewEntry] ---|> Container
void TreeView::TreeViewEntry::insertBefore(const Ref & child,const Ref & after) {
	if(child.isNull()) return;

//	std::cout << " <addEntry2 ";
	TreeViewEntry * e=dynamic_cast<TreeViewEntry*>(child.get());
	if(e==nullptr) {
		e=new TreeViewEntry(getGUI(),myTreeView,child.get());
	} else {
		e->setTreeView(myTreeView);
	}
	if(myTreeView!=nullptr)
		myTreeView->invalidateRegion();
	Container::insertBefore(e,after);
}

//! [TreeView::TreeViewEntry] ---|> Container
void TreeView::TreeViewEntry::removeContent(const Ref & child) {
	if(child.isNull())
		return;

	unmarkSubtree(child.get());
	TreeViewEntry * e = dynamic_cast<TreeViewEntry*>(child.get());

	if(e==nullptr)
		e = dynamic_cast<TreeViewEntry*>(child->getParent());

	if(e==nullptr){
		std::cerr << "Wrong parent!";
		return;
	}
	Container::removeContent(e);
}

//! [TreeView::TreeViewEntry] ---|> Container
void TreeView::TreeViewEntry::clearContents() {
	unmarkSubtree(this);
	Container::clearContents();
}

//! [TreeView::TreeViewEntry] ---|> Container
std::vector<Component*> TreeView::TreeViewEntry::getContents(){
	std::vector<Component*> children;
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext()){
			children.push_back(c);
	}
	return children;
}

//! (TreeView::TreeViewEntry)
TreeView::TreeViewEntry * TreeView::TreeViewEntry::getFirstSubentry()const{
	for (Component * c=getFirstChild();c!=nullptr;c=c->getNext()) {
		TreeViewEntry * e=dynamic_cast<TreeViewEntry*>(c);
		if(e)
			return e;
	}
	return nullptr;
}

//! (TreeView::TreeViewEntry)
void TreeView::TreeViewEntry::setComponent(const Ref & c){
	Component * first = getFirstChild();
	if(c!=first){
		if(dynamic_cast<TreeViewEntry*>(first)==nullptr) // there is an old component? -> remove it
			Container::removeContent(first);
		Container::insertBefore(c,getFirstChild());
		if(myTreeView!=nullptr)
			myTreeView->invalidateRegion();
	}
}

//! (TreeView::TreeViewEntry) ---|> Component
void TreeView::TreeViewEntry::doLayout() {
	if(myTreeView==nullptr)
		return;
	uint32_t h=0;

	// \note Even if isCollapsed(), the other entries need to be moved out of the way 
	//   so that they do not grab the click events for the first component.
	for (Component * c=getFirstChild();c!=nullptr;c=c->getNext()) {
		c->setPosition(Geometry::Vec2(10,h));
		h += static_cast<uint32_t>(c->getHeight());
	}
	setSize( getParent()->getWidth()-getPosition().x(),
			(isCollapsed()&&getFirstChild()!=nullptr) ? 
				getFirstChild()->getHeight() : h);
}

//! (TreeView::TreeViewEntry)
void TreeView::TreeViewEntry::setTreeView( TreeView * tv) {
	myTreeView=tv;

	for (Component * c=getFirstChild();c!=nullptr;c=c->getNext()) {
		TreeViewEntry * e=dynamic_cast<TreeViewEntry*>(c);
		if(e) {
			e->setTreeView(myTreeView);
		}
	}
}

//! (TreeView::TreeViewEntry) ( internal)
void TreeView::TreeViewEntry::unmarkSubtree(Component * subroot)const{
	if(getTreeView()==nullptr) // no treeView set -> no marking possible
		return;

	struct MyVisitor : public Visitor {
		TreeView * treeView;
		MyVisitor(TreeView * tw) : treeView(tw){}
		virtual ~MyVisitor() {};

		visitorResult_t visit(Component & c) override{
			TreeViewEntry * e = dynamic_cast<TreeViewEntry *>(&c);
			if(e!=nullptr && e->isMarked() && e->getTreeView()==treeView){
				treeView->unmarkEntry(e);
			}
			return CONTINUE_TRAVERSAL;
		};
	}visitor(getTreeView());
	subroot->traverseSubtree(visitor);
}

//// ------------------------------------------------------------------------------

//! (ctor)
TreeView::TreeView(GUI_Manager & _gui,const Geometry::Rect & _r,const std::string & _actionName,flag_t _flags/*=0*/) :
		Container(_gui,_r,_flags),
		actionName(_actionName),root(new TreeViewEntry(_gui,this)),scrollPos(0),multiSelect(true),scrollBar(nullptr),
		keyListener(createKeyListener(_gui, this, &TreeView::onKeyEvent)),
		mouseButtonListener(createMouseButtonListener(_gui, this, &TreeView::onMouseButton)),
		optionalMouseMotionListener(createOptionalMouseMotionListener(_gui, this, &TreeView::onMouseMove)) {
	setFlag(SELECTABLE,true);

	_addChild(root.get());

	setFlag(USE_SCISSOR,true);
	setFlag(LOWERED_BORDER,true);
}

//! (dtor)
TreeView::~TreeView() {
	if(optionalScrollBarListener) {
		getGUI().removeDataChangeListener(scrollBar.get(),
										  std::move(*optionalScrollBarListener.get()));
		optionalScrollBarListener.reset();
	}
	// destroy root
	root = nullptr;
}

//! ---|> Component
void TreeView::doLayout() {
	if(root->getHeight()>=getHeight()){
		if(scrollBar.isNull()){
			scrollBar = new Scrollbar(getGUI(), Scrollbar::VERTICAL);
			scrollBar->setExtLayout( 	ExtLayouter::POS_X_ABS|ExtLayouter::REFERENCE_X_RIGHT|ExtLayouter::ALIGN_X_RIGHT|
											ExtLayouter::POS_Y_ABS|ExtLayouter::REFERENCE_Y_TOP|ExtLayouter::ALIGN_Y_TOP |
											ExtLayouter::WIDTH_ABS|ExtLayouter::HEIGHT_ABS,
											Geometry::Vec2(1,2),Geometry::Vec2(getGUI().getGlobalValue(PROPERTY_SCROLLBAR_WIDTH),-4));
			optionalScrollBarListener.reset(new DataChangeListenerHandle(getGUI().addDataChangeListener(
												scrollBar.get(),
												[this](Component *) {
													if(scrollBar.isNotNull()) {
														invalidateRegion();
														invalidateLayout();
														scrollTo(scrollBar->getScrollPos());
													}
												})));
			_addChild(scrollBar.get());
		}
		const int maxScrollPos = std::max(0,static_cast<int>(root->getHeight()-getHeight()));
		scrollPos = std::min(root->getHeight()-getHeight(),scrollPos);
		scrollBar->setMaxScrollPos(maxScrollPos); 
		scrollBar->setScrollPos( scrollPos );
	}else{
		scrollPos = 0;
		if(scrollBar.isNotNull()){
			getGUI().removeDataChangeListener(scrollBar.get(), 
											  std::move(*optionalScrollBarListener.get()));
			optionalScrollBarListener.reset();
			Component::destroy(scrollBar.get());
			scrollBar = nullptr;
		}
	}
	root->setPosition(Geometry::Vec2(-10,-scrollPos));

}

//! ---|> Component
void TreeView::doDisplay(const Geometry::Rect & region) {
	enableLocalDisplayProperties();
	displayDefaultShapes();
	
	if(isSelected()) {
		Geometry::Rect r = getLocalRect();
		r.changeSizeCentered(-2, -2);
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,r);
	}
	auto shape_scrollableMarkerTop = getGUI().getStyleManager().getShape(PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE);
	auto shape_scrollableMarkerBottom = getGUI().getStyleManager().getShape(PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE);
	disableLocalDisplayProperties();
	
	displayChildren(region,true);
	if(scrollBar.isNotNull()){
		if(scrollPos>0)
			shape_scrollableMarkerTop->display( getLocalRect() );
		if(scrollPos<scrollBar->getMaxScrollPos())
			shape_scrollableMarkerBottom->display( getLocalRect() );
	}
}

/**
 * ---|> KeyListener
 * TODO: Handle collapsed entries, allow collapsing of entries, scroll to marked entry, allow multiple selection with keyboard...
 */
bool TreeView::onKeyEvent(const Util::UI::KeyboardEvent & keyEvent) {
	if(!keyEvent.pressed)
		return true;
	else if(keyEvent.key == Util::UI::KEY_TAB){
		return false;
	}
	else if(keyEvent.key == Util::UI::KEY_UP) {
//		scroll(-15);
		if(markedEntries.size()==1){
			TreeViewEntry * m = markedEntries.front().get();
			TreeViewEntry * newEntry=dynamic_cast<TreeViewEntry*>(m->getPrev());

			if(newEntry){
				if(dynamic_cast<TreeViewEntry*>(newEntry->getLastChild())){
					newEntry=dynamic_cast<TreeViewEntry*>(newEntry->getLastChild());
				}
			}else{
				if(dynamic_cast<TreeViewEntry*>(m->getParent())){
					newEntry=dynamic_cast<TreeViewEntry*>(m->getParent());
				}
			}
			if(newEntry){
				unmarkEntry(m);
				markEntry(newEntry);
				markingChanged();
			}
		}

	}
	else if(keyEvent.key == Util::UI::KEY_DOWN) {
//		scroll(15);
		if(markedEntries.size()==1){
			TreeViewEntry * newEntry = nullptr;
			TreeViewEntry * m = markedEntries.front().get();
			if(m->getContentsCount()>1 && dynamic_cast<TreeViewEntry*>(m->getFirstChild()->getNext())){
				newEntry=dynamic_cast<TreeViewEntry*>(m->getFirstChild()->getNext());
			}else{
				newEntry=dynamic_cast<TreeViewEntry*>(m->getNext());
				if(!newEntry){
					if(dynamic_cast<TreeViewEntry*>(m->getParent())){
						newEntry=dynamic_cast<TreeViewEntry*>(m->getParent()->getNext());
					}
				}
			}
			if(newEntry){
				unmarkEntry(m);
				markEntry(newEntry);
				markingChanged();
			}
		}
	}
	return true;
}

//! ---|> Container
void TreeView::addContent(const Ref & child) {
	if(child.isNull()) return;
	invalidateRegion();
	root->addContent(child);
}

//! ---|> Container
void TreeView::removeContent(const Ref & child) {
	if(child.isNull()) return;
	invalidateRegion();
	root->removeContent(child);
}

//! ---|> Container
void TreeView::clearContents() {
	unmarkAll();
	root->clearContents();
	scrollPos = 0;
}

bool TreeView::onMouseButton(Component * /*component*/, const Util::UI::ButtonEvent & buttonEvent) {
	if(buttonEvent.button == Util::UI::MOUSE_BUTTON_MIDDLE && scrollBar.isNotNull()) {
		if(buttonEvent.pressed) {
			optionalMouseMotionListener.enable();
		} else {// !pressed
			optionalMouseMotionListener.disable();
		}
		return true;
	} else if(buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_WHEEL_UP) {
//		scroll(-getHeight()*0.25);

		getGUI().stopAnimations(this);
		getGUI().addAnimationHandler(new TV_ScrollAnimation(this, scrollPos-getHeight()*0.5 ,0.3));

		return true;
	}else if(buttonEvent.pressed && buttonEvent.button == Util::UI::MOUSE_WHEEL_DOWN) {
//		scroll(+getHeight()*0.25);
		getGUI().stopAnimations(this);
		getGUI().addAnimationHandler(new TV_ScrollAnimation(this, scrollPos+getHeight()*0.5 ,0.3));
		return true;
	}else{
		return false;
	}
}

bool TreeView::onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent) {
	if(!(motionEvent.buttonMask & Util::UI::MASK_MOUSE_BUTTON_MIDDLE)) {
		optionalMouseMotionListener.disable();
		return false;
	}
	scroll(motionEvent.deltaY * -2.0);
	return true;
}

void TreeView::scroll(float amount) {
	scrollTo(scrollPos+amount);
}


void TreeView::scrollTo(float _position) {
	const float oldScrollPos=scrollPos;
	scrollPos=_position;
	if(scrollPos<0 || root->getHeight()<getHeight() ){
		
		scrollPos=0;
	}else if(scrollPos>root->getHeight() - getHeight()){
		scrollPos=root->getHeight() - getHeight();
	}
	if(scrollPos!=oldScrollPos){
		invalidateLayout();
		invalidateRegion();
	}
}

void TreeView::scrollToSelection() {
	if(!markedEntries.empty()) {
		TreeViewEntry * entry = markedEntries.front().get();
		const float yPos = entry->getAbsPosition().y()-getAbsPosition().y()+scrollPos;


		if( yPos < scrollPos+entry->getHeight()*0.5 ){
			getGUI().stopAnimations(this);
			getGUI().addAnimationHandler(new TV_ScrollAnimation(this,yPos - entry->getHeight()*0.5 ,0.4));
		}
		else if( yPos+entry->getHeight()*2.0 > scrollPos+getHeight() ){
			getGUI().stopAnimations(this);
			getGUI().addAnimationHandler(new TV_ScrollAnimation(this,yPos - std::max(getHeight()-entry->getHeight()*3.0,getHeight()*0.3 ),0.4));
		}
	}
//	std::cout << getHeight();
}


void TreeView::markEntry(TreeViewEntry * entry) {
	if(entry && entry->getTreeView()==this && !entry->isMarked()){
		if(!multiSelect)
			unmarkAll();
		markedEntries.push_back(entry);
		entry->_setMarked(true);

		if(markedEntries.size()==1)
			scrollToSelection();
	}
}


void TreeView::unmarkEntry(TreeViewEntry * entry) {
	if(entry && entry->isMarked()){
		const auto pos = std::find(markedEntries.begin(),markedEntries.end(),entry);
		if(pos!=markedEntries.end()){
			markedEntries.erase(pos);
			entry->_setMarked(false);
		}
	}
}


void TreeView::unmarkAll() {
	for(auto entry : markedEntries)
		entry->_setMarked(false);
	markedEntries.clear();
}


void TreeView::markComponent(Component * c){
	if(c!=nullptr) 
		markEntry(dynamic_cast<TreeViewEntry *>(c->getParent()));
}


void TreeView::unmarkComponent(Component * c){
	if(c!=nullptr)
		unmarkEntry(dynamic_cast<TreeViewEntry *>(c->getParent()));
}


std::vector<Component*> TreeView::getMarkedComponents(){
	std::vector<Component*> arr;
	for(auto & entry : markedEntries) {
		Component * c = entry->getFirstChild();
		if( dynamic_cast<TreeViewEntry *>(c) == nullptr ) // should be the real content, not an entry...
			arr.push_back(c);
	}
	return arr;
}


void TreeView::markingChanged(){
	getGUI().componentDataChanged(this);
}

//! ---|> Container
std::vector<Component*> TreeView::getContents(){
	return root->getContents();
}

}
