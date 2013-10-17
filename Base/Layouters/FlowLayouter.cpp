/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "FlowLayouter.h"
#include "../../Components/Container.h"
#include "../../Components/LayoutHelper.h"
#include "../../GUI_Manager.h"
#include <iostream>

// ------------------------------------------------------------------------------
namespace GUI {



//! ---|> AbstractLayouter
void FlowLayouter::layout(Util::WeakPointer<Component> component){
	Container * container = dynamic_cast<Container*>(component.get());
	if(!container){
		throw std::invalid_argument("FlowLayouter can only be applied to Containers.");
	}

	std::vector<float> columnStarts;

	{    // Calculate columns' widths
		std::vector<float> columnWidths;

		unsigned int columnNr=0;
		float currentWidth = 0;
		for(Component * c=container->getFirstChild();c!=nullptr;c=c->getNext()){
			// next row
			if(dynamic_cast<NextRow*>(c)){
				columnNr = 0;
				currentWidth = 0;
			} // next column
			else if(NextColumn *nc=dynamic_cast<NextColumn*>(c)){
				currentWidth += nc->additionalSpacing;

				if( columnNr >= columnWidths.size() ){
					columnWidths.push_back(currentWidth);
				}else if(currentWidth > columnWidths[columnNr]){
					columnWidths[columnNr] = currentWidth;
				}

				++columnNr;
				currentWidth = 0;

			} // other component
			else{
				currentWidth += c->getWidth()+padding;
			}
		}
		columnStarts.reserve(columnWidths.size());

		// init columns' starting positions
		float x=margin;
		columnStarts.push_back(x);

		for(const auto & columnWidth : columnWidths) {
			x += columnWidth;
			columnStarts.push_back(x);
		}
	}
	const bool autoBreak = getAutoBreak();
	const int rightBorder = getMaximize() && container->hasParent() ? container->getParent()->getWidth() : container->getWidth();

	// reorder children
	{
		unsigned int columnNr=0;
		Geometry::Vec2 cursor(columnStarts.front(),margin);

		float maxY=cursor.getY();
		float maxX=0;
		for(Component * c=container->getFirstChild();c!=nullptr;c=c->getNext()){
			// next row
			if(NextRow *nr=dynamic_cast<NextRow*>(c)){
				columnNr=0;
				cursor.setX(columnStarts.front());
				cursor.setY(maxY+padding+nr->additionalSpacing);
			} // next column
			else if(dynamic_cast<NextColumn*>(c)){
				++columnNr;
				cursor.setX(columnStarts[columnNr]);
			} // other component
			else{
				if(autoBreak && cursor.getX()>columnStarts.front() && 
						cursor.getX()+c->getWidth()>rightBorder){
					cursor.setX(columnStarts.front());
					cursor.setY(maxY+padding);
				}
					
				c->setPosition(cursor);
				cursor.setX(cursor.getX()+c->getWidth()+padding);
				maxY = std::max( maxY, cursor.getY()+c->getHeight() );
				maxX = std::max( maxX, c->getPosition().getX()+c->getWidth() );
			}
		}
		if(getMaximize()){
			maxX+=margin;
			maxY+=margin;
			
			// if ...
			if(container->hasParent()){
//				const Geometry::Rect parentsRect = container->getParent()->getInnerRect();
//				maxX = std::max(maxX,parentsRect.getWidth());
//				maxY = std::max(maxY,parentsRect.getHeight());
			
				maxX = std::max(maxX,container->getParent()->getWidth());
				maxY = std::max(maxY,container->getParent()->getHeight());
			}
			
			container->setSize(maxX,maxY);
		}
	}

}

}
