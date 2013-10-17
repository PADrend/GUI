/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "ExtLayouter.h"
#include "../../Components/Container.h"
#include <iostream>

namespace GUI{

static Geometry::Vec2 getChildrensSize(Component * c){
	Container * container = dynamic_cast<Container*>(c);
	float x = 0.0f, y = 0.0f;
	if(container){
		for(Component * child=container->getFirstChild();child!=nullptr;child=child->getNext()){
			x = std::max( x, child->getWidth()+child->getPosition().x() );
			y = std::max( y, child->getHeight()+child->getPosition().y() );
		}
	}
	return Geometry::Vec2(x,y);
}

//! ---|> AbstractLayouter
void ExtLayouter::layout(Util::WeakPointer<Component> component){
	if( !component->hasParent() )
		return;

	Geometry::Rect rect = component->getRect();
	
	float parentWidth = component->getParent()->getWidth();
	float parentHeight = component->getParent()->getHeight();
	
	if(component->getParent()->getFlag(Component::IS_CLIENT_AREA) && component->getParent()->hasParent() ){
		parentWidth = component->getParent()->getParent()->getWidth();
		parentHeight = component->getParent()->getParent()->getHeight();
	}

	Geometry::Vec2 childrensSize;

	// apply size
	if( flags & (WIDTH_REL|WIDTH_ABS|WIDTH_CHILDREN_ABS|WIDTH_CHILDREN_REL|WIDTH_FILL_REL|WIDTH_FILL_ABS )){
		if( flags&WIDTH_REL ){
			rect.setWidth( parentWidth * extSize.getWidth() );
		}else if( flags&WIDTH_ABS){
			rect.setWidth( extSize.getWidth()<0 ?
					parentWidth+extSize.getWidth():
					extSize.getWidth() );
		}else if( flags&WIDTH_CHILDREN_ABS){
			childrensSize = getChildrensSize(component.get());
			rect.setWidth( childrensSize.x() + extSize.getWidth() );
		}else if( flags&WIDTH_CHILDREN_REL){
			childrensSize = getChildrensSize(component.get());
			rect.setWidth( childrensSize.x() * extSize.getWidth() );
		}else if( flags&WIDTH_FILL_REL){
			const float space = parentWidth - rect.getX();
			rect.setWidth( space*extSize.getWidth() );
		}else if( flags&WIDTH_FILL_ABS){
			const float space = parentWidth - rect.getX();
			rect.setWidth( space-extSize.getWidth() );
		}

	}

	if( flags & (HEIGHT_REL|HEIGHT_ABS|HEIGHT_CHILDREN_ABS|HEIGHT_CHILDREN_REL|HEIGHT_FILL_REL|HEIGHT_FILL_ABS )){
		if( flags&HEIGHT_REL ){
			rect.setHeight( parentHeight * extSize.getHeight() );
		}else if( flags&HEIGHT_ABS){
			rect.setHeight( extSize.getHeight()<0 ?
					parentHeight+extSize.getHeight():
					extSize.getHeight() );
		}else if( flags&HEIGHT_CHILDREN_ABS){
			if( childrensSize.x() == 0)
				childrensSize = getChildrensSize(component.get());
			rect.setHeight( childrensSize.y() + extSize.getHeight() );
		}else if( flags&HEIGHT_CHILDREN_REL){
			if( childrensSize.x() == 0)
				childrensSize = getChildrensSize(component.get());
			rect.setHeight( childrensSize.y() * extSize.getHeight() );
		}else if( flags&HEIGHT_FILL_REL){
			const float space = parentHeight - rect.getY();
			rect.setHeight( space*extSize.getHeight() );
		}else if( flags&HEIGHT_FILL_ABS){
			const float space = parentHeight - rect.getY();
			rect.setHeight( space-extSize.getHeight() );
		}
	}


	// apply x pos
	float x=getPosition().getX();
	if( (flags&POS_X_ABS) || (flags&POS_X_REL) ){
		x = (flags&POS_X_ABS) ?
				extPos.getX() :
				parentWidth * extPos.getX();

		if( flags&REFERENCE_X_CENTER ){
			x -= rect.getWidth()*0.5;
		}else if( flags&REFERENCE_X_RIGHT  && !(flags&ALIGN_X_RIGHT) ){
			x -= rect.getWidth();
		} else if( flags&REFERENCE_X_LEFT  && !(flags&ALIGN_X_LEFT) ){
			x -= rect.getWidth();
		}

		if( flags&ALIGN_X_RIGHT ){
			x = parentWidth - x - rect.getWidth();
		}else if( flags&ALIGN_X_CENTER ){
			x = parentWidth*0.5 + x;
		}//else ALIGN_X_LEFT: default
	}

	// apply y pos
	float y = rect.getMinY();
	if( (flags&POS_Y_ABS) || (flags&POS_Y_REL) ){
		y = (flags&POS_Y_ABS) ?
				extPos.getY() :
				parentHeight * extPos.getY();

		if( flags&REFERENCE_Y_CENTER ){
			y -= rect.getHeight()*0.5;
		}else if( flags&REFERENCE_Y_BOTTOM && !(flags&ALIGN_Y_BOTTOM) ){
			y -= rect.getHeight();
		}else if( flags&REFERENCE_Y_TOP && !(flags&ALIGN_Y_TOP) ){
			y -= rect.getHeight();
		}

		if( flags&ALIGN_Y_BOTTOM ){
			y = parentHeight - y - rect.getHeight();
		}else if( flags&ALIGN_Y_CENTER ){
			y = parentHeight*0.5 + y;
		}//else REFERENCE_Y_TOP: default
	}
	rect.setPosition(Geometry::Vec2(x,y));
	component->setRect(rect);
}

} // namespace GUI
