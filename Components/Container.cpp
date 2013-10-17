/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Container.h"
#include "../GUI_Manager.h"
#include <iostream>

namespace GUI {

//! (ctor)
Container::Container(GUI_Manager & _gui,flag_t _flags/*=0*/) :
		Component(_gui,_flags),contentsCount(0) {
	//ctor
}

//! (ctor)
Container::Container(GUI_Manager & _gui,const Geometry::Rect & _r,flag_t _flags/*=0*/) :
		Component(_gui,_r,_flags),contentsCount(0) {
	//ctor
}

//! (dtor)
Container::~Container() {
	std::vector<Ref> refHolders;
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext())
		refHolders.push_back(c);

	for(const auto & refHolder : refHolders) {
		refHolder->_setParent(nullptr);
		refHolder->_updateNeighbors(nullptr, nullptr);
	}

	firstChild = lastChild = nullptr;
	contentsCount = 0;
	//dtor
}

void Container::_insertAfter(const Ref & child,const Ref & after){
	if (child.isNull() || child==after) return;

	if(child->getParent()!=this){
		++contentsCount;
		if(child->hasParent())
			child->getParent()->_removeChild(child);
		child->_setParent(this);
		child->invalidateAbsPosition();
	}
	if(after==getLastChild())
		lastChild=child;
	if(firstChild==child)
		firstChild=child->getNext();
//    child->insertAfter(after);
	child->_updateNeighbors(after,after.isNull() ? nullptr : after->getNext());
	if(firstChild==nullptr)
		firstChild=child;
	if(lastChild==nullptr)
		lastChild=child;

	if(getFirstChild()->getPrev()!=nullptr){
		firstChild=getFirstChild()->getPrev();
	}
	if(getLastChild()->getNext()!=nullptr){
		lastChild=getLastChild()->getNext();
	}

	childRectChanged(child.get());
	invalidateLayout();
}

void Container::_insertBefore(const Ref & child,const Ref & before){
	if (child.isNull() || child==before) return;

	if(child->getParent()!=this){
		++contentsCount;
		if(child->hasParent()){
			child->getParent()->_removeChild(child);
		}
		child->_setParent(this);
		childRectChanged(child.get());
		child->invalidateAbsPosition();
	}
	if(before==getFirstChild())
		firstChild=child;
//    child->insertBefore(before);
	child->_updateNeighbors(before.isNull() ? nullptr : before->getPrev(),before);

	if(firstChild==nullptr)
		firstChild=child;
	if(lastChild==nullptr)
		lastChild=child;

	if(getFirstChild()->getPrev()!=nullptr){
		firstChild=getFirstChild()->getPrev();
	}
	if(getLastChild()->getNext()!=nullptr){
		lastChild=getLastChild()->getNext();
	}
	childRectChanged(child.get());
	invalidateLayout();
}

void Container::_removeChild(const Ref & child) {
	if (child.isNull() || (child->getParent()!=this) ) {
		if(!child.isNull()){
			std::cout << "Container::_removeChild: Component is not a child. this:" <<this->getTypeName()<<" component:"<<child->getTypeName()<<"\n";
		}
		return;
	}
	if(child==getFirstChild()){
		firstChild = child->getNext();
	}
	if(child==getLastChild()){
		lastChild = child->getPrev();
	}

	child->_updateNeighbors(nullptr,nullptr);
	child->_setParent(nullptr);

	--contentsCount;
	childRectChanged(child.get());
	invalidateLayout();
}

//! ---|> Component
std::string Container::toString()const {
	std::ostringstream s;
	size_t i = getContentsCount();
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext()){
		s<<c->toString();
		if (--i>0)s<<",";
	}
	s<<"]";
	return s.str();
}

void Container::displayChildren(const Geometry::Rect & region,bool useScissor/*=false*/)   {
	const Geometry::Rect myRegion=region.isValid() ? getAbsRect().clipBy(region) : getAbsRect();
	if(myRegion.isInvalid())
		return;
	if(useScissor){
		Geometry::Rect_i scissorRect = Geometry::Rect_i(getAbsRect());
		scissorRect.changeSize(-2,-2);
		scissorRect.moveRel(1,1);
		getGUI().pushScissor(scissorRect);
	}
		
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext()){
		if (c->isEnabled() && myRegion.intersects(c->getAbsRect()))
			c->display(region);
	}
	if(useScissor)
		getGUI().popScissor();
}

//! ---|> Component
void Container::doDisplay(const Geometry::Rect & region) {
//	if(hasShape())
//		getShape()->display(getLocalRect());
	displayChildren(region);
}

//! ---|> Component
Component::visitorResult_t Container::traverseSubtree(Visitor & v) {
	switch(v.visit(*this)){
		case BREAK_TRAVERSAL:
			return CONTINUE_TRAVERSAL;
		case EXIT_TRAVERSAL:
			return EXIT_TRAVERSAL;
		case CONTINUE_TRAVERSAL:
			for(Component * c = getFirstChild();c!=nullptr;c = c->getNext()){
				if(c->traverseSubtree(v) == EXIT_TRAVERSAL)
					return EXIT_TRAVERSAL;
			}
		default:
			return CONTINUE_TRAVERSAL;
			
	}
}

//! ---|> Component
Component::visitorResult_t Container::traverseChildren(Visitor & v) {
	for(Component * c = getFirstChild();c!=nullptr;c = c->getNext()){
		if( v.visit(*c)==EXIT_TRAVERSAL )
			return EXIT_TRAVERSAL;
	}
	return CONTINUE_TRAVERSAL;
}

//! ---o
void Container::bringChildToFront(Component * /*c*/) {
	if (hasParent())
		getParent()->bringChildToFront(this);
}

//! ---o
void Container::clearContents(){
	for(Component::Ref childRef : getContents()) { // don't use the children directly, but use this virtual function getContents to work with specialized types.
		removeContent(childRef.get());
	}
}

void Container::destroyContents(){
	for(Component::Ref childRef : getContents()) {  // don't use the children directly, but use this virtual function getContents to work with specialized types.
		Component::destroy(childRef.get());
	}
//		getGUI().markForRemoval(*it);

//	this->clear();
}

//! ---o
std::vector<Component*> Container::getContents() {
	std::vector<Component*> children;
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext())
		children.push_back(c);
	return children;
}

void Container::childRectChanged(Component * /*c*/){
	// if(getFlag(LAYOUT_DEPENDS_ON_CHILDREN)) !!!!!!!!!!!!!!!!!!
	invalidateLayout();
	// if  is sensible to child changes && internal layout is valid
	//	 invalidate internal layout, inform parent
}

}
