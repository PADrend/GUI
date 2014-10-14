/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Component.h"

#include "ComponentPropertyIds.h"
#include "../Base/ListenerHelper.h"
#include "../Base/Draw.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "ComponentTooltipFeature.h"
#include "../GUI_Manager.h"
#include "Container.h"
#include <algorithm>
#include <functional>
#include <iostream>

namespace GUI{

//! (ctor)
Component::Component(GUI_Manager & _gui,flag_t _flags/*=0*/)
		: Util::AttributeProvider(), Util::ReferenceCounter<Component>(), gui(_gui),prev(nullptr),next(nullptr),flags(_flags) { 
}

//! (ctor)
Component::Component(GUI_Manager & _gui,const Geometry::Rect & _relRect,flag_t _flags/*=0*/)
		: Util::AttributeProvider(), Util::ReferenceCounter<Component>(), gui(_gui),prev(nullptr),next(nullptr),flags(_flags) {
	setRect(_relRect);
	//ctor
}

//! (dtor)
Component::~Component() {
	getGUI().componentDestruction(this);
	removeExternalLayout();
	removeAttributes();
	//dtor
}

//! (static)
void Component::destroy(Component * component){
	struct MyVisitor : public Component::Visitor {
			std::vector<Util::Reference<Component> > components;
			// ---|>  Component::Visitor
			visitorResult_t visit(Component & c) override {
				components.push_back(&c);
				return Component::CONTINUE_TRAVERSAL;
			}
	}visitor;

	component->traverseSubtree(visitor);
	for(auto & c : visitor.components){
//		(*it)->setParent(nullptr);
		c->removeAttributes();
		c->clearProperties();
		if(c->hasParent())
			c->getParent()->_removeChild(c);
		c->setFlag(DESTROYED,true);
	}
}

//! (internal)
void Component::_updateNeighbors(const Ref & newPrev,const Ref & newNext){
	const Ref oldPrev=getPrev();
	const Ref oldNext=getNext();

	setPrev(newPrev);
	setNext(newNext);

	if(newPrev.isNotNull())
		newPrev->setNext(this);

	if(newNext.isNotNull())
		newNext->setPrev(this);

	if(oldPrev.isNotNull())
		oldPrev->setNext(oldNext);

	if(oldNext.isNotNull())
		oldNext->setPrev(oldPrev);
}

Geometry::Vec2 Component::getAbsPosition() {
	if (isAbsPosValid()) {
		return Geometry::Vec2(absPosition);
	}
	absPosition = getPosition();
	if (hasParent()) {
		absPosition+=getParent()->getAbsPosition();
	}
	setFlag(ABS_POSITION_VALID,true);
	return absPosition;
}

//! ---o
std::string Component::toString() const {
	std::ostringstream s;
	s<<getTypeName()<<"#"<<static_cast<const void*>(this);
	return s.str();
}

bool Component::isActive()const {
	return getGUI().isActiveComponent(this);
}

void Component::activate() {
	invalidateRegion();
	getGUI().setActiveComponent(this);
}

void Component::deactivate() {
	if (isActive()){
		invalidateRegion();
		getGUI().setActiveComponent(nullptr);
	}
}
void Component::enable(){	
	if(!isEnabled()){
		setFlag(DISABLED,false);	
		invalidateLayout();
		invalidateRegion();
	}
}
void Component::disable() {
	invalidateRegion();
	setFlag(DISABLED,true);	
}

void Component::invalidateAbsPosition() {
	if(isAbsPosValid()){
		struct MyVisitor : public Component::Visitor {
			// ---|> Component::Visitor
			visitorResult_t visit(Component & c) override {
				if (!c.isAbsPosValid()) {
					return Component::BREAK_TRAVERSAL;
				} else {
					c.setFlag(Component::ABS_POSITION_VALID,false);
				}
				return Component::CONTINUE_TRAVERSAL;
			}
		}visitor;
		traverseSubtree(visitor);
	}
}

void Component::setRect(const Geometry::Rect & newRect) {
	const Geometry::Rect oldRect = relRect;
	if(Geometry::Vec2i( oldRect.getSize()) != Geometry::Vec2i(newRect.getSize())){
		invalidateRegion(); // invalidate old rect

		relRect = newRect;

		if(oldRect.getPosition()!=newRect.getPosition())
			invalidateAbsPosition();

		//	traverseChildren(visitor); // this is not enough for certain nested layouts
		invalidateSubtreeLayout();

		if(hasParent())
			parent->childRectChanged(this);
		invalidateLayout();
		invalidateRegion(); // invalidate new rect
	}else if(Geometry::Vec2i( oldRect.getPosition()) != Geometry::Vec2i(newRect.getPosition())){
		invalidateRegion(); // invalidate old rect
		relRect = newRect;

		invalidateAbsPosition();
		if(hasParent())
			parent->childRectChanged(this);
		invalidateRegion(); // invalidate new rect
	}
}

void Component::enableLocalDisplayProperties() {
	for(auto& p: localDisplayProperties)
		getGUI().enableProperty( p );
}

void Component::disableLocalDisplayProperties() {
	for(auto it=localDisplayProperties.rbegin(); it!=localDisplayProperties.rend(); ++it)
		getGUI().disableProperty(*it);
}

void Component::displayDefaultShapes() {
	if(flags&BORDER){
		if((flags^BORDER)&RAISED_BORDER){
			getGUI().displayShape(PROPERTY_COMPONENT_RAISED_BORDER_SHAPE,getLocalRect());
		}else if((flags^BORDER)&LOWERED_BORDER){
			getGUI().displayShape(PROPERTY_COMPONENT_LOWERED_BORDER_SHAPE,getLocalRect());
		}else{
			getGUI().displayShape(PROPERTY_COMPONENT_BORDER_SHAPE,getLocalRect());
		}
	}
	getGUI().displayShape(PROPERTY_COMPONENT_ADDITIONAL_BACKGROUND_SHAPE,getLocalRect());
	if(flags&BACKGROUND)
		getGUI().displayShape(PROPERTY_COMPONENT_BACKGROUND_SHAPE,getLocalRect());
}
	
void Component::display(const Geometry::Rect & region) {
	// enable display properties
	for(auto& p: recursiveDisplayProperties)
		getGUI().enableProperty( p );

	// displayBegin();
	Draw::moveCursor(getPosition());

	if(flags&USE_SCISSOR){
		getGUI().pushScissor(Geometry::Rect_i(getAbsRect()));
	}

	doDisplay(region);

	//displayEnd();
	Draw::moveCursor(-getPosition());
	if( flags&USE_SCISSOR){
		getGUI().popScissor();
	}
	// disable display properties
	for(auto it=recursiveDisplayProperties.rbegin();it!=recursiveDisplayProperties.rend();++it)
		getGUI().disableProperty(*it);
}

//! ---o
void Component::doDisplay(const Geometry::Rect & /*region*/) {
	enableLocalDisplayProperties();
	displayDefaultShapes();

	if (isSelected()) {
		Geometry::Rect r=getLocalRect();
		r.changeSize(-4,-4);
		r.moveRel(2,2);
		getGUI().displayShape(PROPERTY_SELECTION_RECT_SHAPE,r);
	}
	disableLocalDisplayProperties();
}

//! ---o
bool Component::onSelect() {
	return true;
}

//! ---o
bool Component::onUnselect() {
	return true;
}

void Component::select() {
	if (isSelected()) {
		if (hasParent())
			getParent()->select();
		return;
	}
	if(!onSelect()){
		return;
	}

	if (hasParent()) {
		getParent()->unselectSubtree();
		// recheck parent: the parent can be lost on unselectSubtree() as this can cause an onUnselect-action to be performed which deletes the current subtree.
		if (hasParent()) {
			getParent()->select();
		}
	}
	setFlag(SELECTED,true);
}

void Component::unselectSubtree() {
	struct V : public Visitor {
		std::vector<Component::Ref> selectedComponents;
		V():Visitor() {}
		virtual ~V() {}

		// ---|> Component::Visitor
		visitorResult_t visit(Component & c) override {
			if(!c.isSelected()) 
				return Component::BREAK_TRAVERSAL;
			selectedComponents.push_back(&c);
			return Component::CONTINUE_TRAVERSAL;
		}
	}collector;
	for(traverseSubtree(collector); !collector.selectedComponents.empty() ; collector.selectedComponents.pop_back())
		collector.selectedComponents.back()->unselect();
}

void Component::invalidateRegion(){
	if(getGUI().isLazyRenderingEnabled()){
		for(Component * c = this;c!=nullptr;c=c->getParent())
			if(!c->isEnabled())
				return;
		getGUI().invalidateRegion(getAbsRect());
	}
}


Component * Component::getComponentAtPos(const Geometry::Vec2 & pos) {
	struct MyVisitor : public Component::Visitor {
		const Geometry::Vec2 & pos;
		Component * found;

		MyVisitor(const Geometry::Vec2 & _pos):Visitor(),pos(_pos),found(nullptr) {}
		virtual ~MyVisitor() {}

		// ---|> Component::Visitor
		visitorResult_t visit(Component & c) override {
			if (c.isEnabled() && c.coversAbsPosition(pos)) {
				if (!c.getFlag(Component::TRANSPARENT_COMPONENT))
					found=&c;
				return Component::CONTINUE_TRAVERSAL;
			}
			return Component::BREAK_TRAVERSAL;
		}
	}visitor(pos);
	traverseSubtree(visitor);
	return visitor.found;
}



Component * Component::findSelectedComponent() {
	struct MyVisitor : public Component::Visitor {
		Component * found;

		MyVisitor() : Visitor(),found(nullptr){}
		virtual ~MyVisitor() {}

		// ---|> Component::Visitor
		visitorResult_t visit(Component & c) override {
			if( c.isEnabled() && c.isSelected() ) {
				found=&c;
				return CONTINUE_TRAVERSAL;
			}
			return BREAK_TRAVERSAL;
		}
	}visitor;
	
	traverseSubtree(visitor);
	return visitor.found;
}

void Component::bringToFront() {
	invalidateRegion();
	if (hasParent())
		getParent()->bringChildToFront(this);
}

bool Component::isVisible()const {
	if (!isEnabled())
		return false;
	else if (hasParent())
		return getParent()->isVisible();
	else
		return true;
}


//! ---o
bool Component::coversLocalPosition(const Geometry::Vec2 & localPos){
	return getLocalRect().contains(localPos);
}

// -----------------------------------
// ---- MouseCursor
static const Util::StringIdentifier attrName_mouseCursor("_MOUSECURSOR_");
    
void Component::setMouseCursorProperty(propertyName_t type){
    setFlag(HAS_MOUSECURSOR_PROPERTY, true);
    setAttribute(attrName_mouseCursor, Util::GenericAttribute::createString(type));
}

propertyName_t Component::getMouseCursorProperty(){
    Util::GenericAttribute *attr = getAttribute(attrName_mouseCursor);
    return (attr == nullptr) ? PROPERTY_MOUSECURSOR_DEFAULT : attr->toString();
}

// -----------------------------------
// ---- Tooltip

bool Component::hasTooltip()const					{	return hasComponentTooltip(*this);	}
std::string Component::getTooltip()const			{	return getComponentTooltip(*this);	}
void Component::setTooltip(const std::string & s)	{	setComponentTooltip(*this,s);	}
void Component::removeTooltip()						{	removeComponentTooltip(*this);	}

// -----------------------------------
// ---- Layout

uint32_t Component::layout(){
		// enable display properties
	for(auto & prop : recursiveDisplayProperties)
		getGUI().enableProperty(prop);
		
	const bool wasValid = getFlag(LAYOUT_VALID);
	setFlag(LAYOUT_VALID,true);
	
	uint32_t count = 0;
	
	if(!getFlag(SUBTREE_LAYOUT_VALID)){
		setFlag(SUBTREE_LAYOUT_VALID,true);
		count += layoutChildren();
	}
		
	if(!wasValid || !getFlag(LAYOUT_VALID)){
		
		for(auto & layouter : layouters)
			layouter->layout(this);

		// \note external layouter should not be used in combination with AUTO_MAXIMIZE
		if (getFlag(AUTO_MAXIMIZE)){ // deprecated!
			if (hasParent()) {
				setRect( getParent()->getInnerRect() );
			}
		}

		enableLocalDisplayProperties();
		doLayout();
		disableLocalDisplayProperties();

//		if(!getFlag(SUBTREE_LAYOUT_VALID)){
//			setFlag(SUBTREE_LAYOUT_VALID,true);
//			count += layoutChildren();
//		}
		++count;
	}
	// disable display properties
	for(auto & prop : recursiveDisplayProperties)
		getGUI().disableProperty(prop);	
	return count;
}


void Component::invalidateLayout(){
	setFlag(LAYOUT_VALID,false);
	for(Container * c=getParent();c!=nullptr && c->getFlag(SUBTREE_LAYOUT_VALID) ;c=c->getParent()){
		c->setFlag(SUBTREE_LAYOUT_VALID,false);
	}
}

void Component::invalidateSubtreeLayout(){
	struct MyVisitor:public Component::Visitor {
		// ---|> Component::Visitor
		visitorResult_t visit(Component & c) override {
			c.setFlag(LAYOUT_VALID,false);
			c.setFlag(SUBTREE_LAYOUT_VALID,false);
			return Component::CONTINUE_TRAVERSAL;
		}
	}visitor;
	traverseSubtree(visitor);
	invalidateLayout();
}

uint32_t Component::layoutChildren(){
	struct MyVisitor:public Component::Visitor {
		uint32_t count;
		MyVisitor():Visitor(),count(0){}
		
		// ---|> Component::Visitor
		visitorResult_t visit(Component & c) override {
			if( c.isEnabled() ){
				count += c.layout();
			}
			return Component::CONTINUE_TRAVERSAL;
		}
	}visitor;
	traverseChildren(visitor);
	return visitor.count;
}

void Component::removeLayouter(Util::WeakPointer<AbstractLayouter> layouter){
	auto pos = std::find(layouters.begin(),layouters.end(),layouter.get());
	if(pos!=layouters.end()){
		layouters.erase(pos);
	}
}
// -----------------------------------
// ---- Properties
void Component::removeProperty(DisplayProperty * p){
	auto pos = std::find(recursiveDisplayProperties.begin(),recursiveDisplayProperties.end(),p);
	if(pos!=recursiveDisplayProperties.end())
		recursiveDisplayProperties.erase(pos);
}
void Component::removeLocalProperty(DisplayProperty * p){
	auto pos = std::find(localDisplayProperties.begin(),localDisplayProperties.end(),p);
	if(pos!=localDisplayProperties.end())
		localDisplayProperties.erase(pos);
}

// ---------------------------------------
// ---- external layout

void Component::setExtLayout(uint32_t _flags,const Geometry::Vec2 & extPos){
	setExtLayout(_flags,extPos,Geometry::Vec2(0,0));
}

void Component::setExtLayout(uint32_t _flags,const Geometry::Vec2 & extPos, const Geometry::Vec2 & extSize){
	removeExternalLayout();
	auto l = new ExtLayouter;
	l->setFlags(_flags);
	l->setPosition(extPos);
	l->setSize(extSize);
	addLayouter(l);
}

void Component::removeExternalLayout(){
	removeLayouter<ExtLayouter>();
}

} // namespace GUI
