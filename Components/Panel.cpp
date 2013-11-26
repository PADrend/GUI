/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Panel.h"
#include "../GUI_Manager.h"
#include "../Base/AnimationHandler.h"
#include "../Base/Layouters/FlowLayouter.h"
#include "../Base/Layouters/ExtLayouter.h"
#include "Scrollbar.h"
#include <algorithm>
#include <iostream>

// ------------------------------------------------------------------------------
namespace GUI {


static FlowLayouter * getDefaultLayouter(){
	static Util::Reference<FlowLayouter> defaultLayouter;
	if(defaultLayouter.isNull()){
		defaultLayouter = new FlowLayouter;
		defaultLayouter->setMargin(10);
		defaultLayouter->setPadding(4);
		defaultLayouter->setMaximize(true);
	}
	return defaultLayouter.get();
}

//! (ctor)
Panel::Panel(GUI_Manager & _gui,flag_t _flags/*=0*/) : ScrollableContainer(_gui,_flags){
	getContentContainer()->addLayouter(getDefaultLayouter());
}

Panel::~Panel() = default;

void Panel::disableAutoBreak(){
	accessLayouter().setAutoBreak(false);
}

void Panel::enableAutoBreak(){
	accessLayouter().setAutoBreak(true);
}

//! (internal) Replaces the layouter on each access.
FlowLayouter & Panel::accessLayouter(){
	const auto oldLayouter = getContentContainer()->getLayouter<FlowLayouter>();
	auto newLayouter = new FlowLayouter(*(oldLayouter ? oldLayouter : getDefaultLayouter()));

	if(oldLayouter)
		getContentContainer()->removeLayouter(oldLayouter);
	getContentContainer()->addLayouter(newLayouter);
	return *newLayouter;
}

void Panel::setMargin(int _margin){
	accessLayouter().setMargin(_margin);
}

void Panel::setPadding(int _padding){
	accessLayouter().setPadding(_padding);
}


void Panel::nextRow(float additionalSpacing/*=0*/){
	addContent( new NextRow(getGUI(),additionalSpacing) );
}

void Panel::nextColumn(float additionalSpacing/*=0*/){
	addContent( new NextColumn(getGUI(),additionalSpacing) );
}


}
