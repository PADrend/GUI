/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "StyleManager.h"
#include "BasicColors.h"

#include <Util/UI/Cursor.h>
#include <memory>
#include <unordered_map>

namespace GUI{

StyleManager::~StyleManager() = default;

//------------------------------------------------------
// colors

Util::Color4ub StyleManager::getColor(propertyId_t type)const{
	if(type>=colorRegistry.size())
		return Colors::NO_COLOR;
	const colorStack_t & stack(colorRegistry[type]);
	return stack.empty() ? Colors::NO_COLOR : stack.back();
}

//! (internal)
void StyleManager::initColors(size_t newSize){
	const size_t oldSize = colorRegistry.size();
	colorRegistry.resize(newSize);
	for(size_t i=oldSize;i<newSize;++i)
		colorRegistry[i].push_back(Colors::NO_COLOR);
}

void StyleManager::pushColor(propertyId_t type,const Util::Color4ub & c){
	if(type>=colorRegistry.size())
		initColors(type+1);
	colorRegistry[type].push_back(c);
}

void StyleManager::popColor(propertyId_t type){
	if(type>=colorRegistry.size() || colorRegistry[type].size()<=1){
		WARN("Empty property stack.");
	}else{
		colorRegistry[type].pop_back();
	}
}

void StyleManager::setDefaultColor(propertyId_t type,const Util::Color4ub & c){
	if(type>=colorRegistry.size())
		initColors(type+1);
	colorRegistry[type][0]=c;
}

//------------------------------------------------------
// fonts

AbstractFont * StyleManager::getDefaultFont(propertyId_t type)const{
	if(type>=fontRegistry.size()){
		WARN("No Font!");
		return nullptr;
	}
	const fontStack_t & stack(fontRegistry[type]);
	if(stack.empty()||stack.front().isNull()){
		WARN("No Font!");
		return nullptr;
	}
	return stack.front().get();
}

AbstractFont * StyleManager::getFont(propertyId_t type)const{
	if(type>=fontRegistry.size()){
		WARN("No Font!");
		return nullptr;
	}
	const fontStack_t & stack(fontRegistry[type]);
	if(stack.empty()||stack.back().isNull()){
		WARN("No Font!");
		return nullptr;
	}
	return stack.back().get();
}

//! (internal)
void StyleManager::initFonts(size_t newSize){
	const size_t oldSize = fontRegistry.size();
	fontRegistry.resize(newSize);
	for(size_t i=oldSize;i<newSize;++i)
		fontRegistry[i].push_back(nullptr);
}

void StyleManager::pushFont(propertyId_t type,AbstractFont * f){
	if(type>=fontRegistry.size())
		initFonts(type+1);
	fontRegistry[type].push_back(f);
}

void StyleManager::popFont(propertyId_t type){
	if(type>=fontRegistry.size() || fontRegistry[type].size()<=1){
		WARN("Empty font stack.");
	}else{
		fontRegistry[type].pop_back();
	}
}

void StyleManager::setDefaultFont(propertyId_t type,AbstractFont * f){
	if(type>=fontRegistry.size())
		initFonts(type+1);
	fontRegistry[type][0]=f;
}
//------------------------------------------------------
// mouse cursor

// registry for mouse cursor

void StyleManager::setMouseCursor(propertyName_t name, Util::UI::Cursor * cursor) {
	if(name.empty()) {
		WARN("Invalid name or cursor.");
	} else {
		cursorRegistry[name].reset(cursor);
	}
}

Util::UI::Cursor * StyleManager::getMouseCursor(propertyName_t name) const{
	const auto it = cursorRegistry.find(name);
	return (it == cursorRegistry.end() || !it->second) ? nullptr : it->second.get();
}
	
void StyleManager::removeMouseCursor(propertyName_t name) {
	cursorRegistry.erase(name);
}

//------------------------------------------------------

//! (internal)
void StyleManager::initShapes(size_t newSize){
	const size_t oldSize = shapeRegistry.size();
	shapeRegistry.resize(newSize);
	for(size_t i=oldSize;i<newSize;++i)
		shapeRegistry[i].push_back(NullShape::instance());
}

AbstractShape * StyleManager::getShape(propertyId_t type)const{
	if(type>=shapeRegistry.size())
		return NullShape::instance();
	const shapeStack_t & stack(shapeRegistry[type]);
	return stack.empty() ? NullShape::instance() : stack.back().get();
}

void StyleManager::pushShape(propertyId_t type,AbstractShape * s){
	if(type>=shapeRegistry.size())
		initShapes(type+1);
	shapeRegistry[type].push_back(s);

}

void StyleManager::popShape(propertyId_t type){
	if(type>=shapeRegistry.size() || shapeRegistry[type].size()<=1){
		WARN("Empty shape stack.");
	}else{
		shapeRegistry[type].pop_back();
	}
}

void StyleManager::setDefaultShape(propertyId_t type,AbstractShape * f){
	if(type>=shapeRegistry.size())
		initShapes(type+1);
	shapeRegistry[type][0]=f;
}
//------------------------------------------------------

float StyleManager::getGlobalValue(propertyId_t type)const{
	if(type>=valueRegistry.size())
		return 0.0f;
	return valueRegistry[type];
}

void StyleManager::setGlobalValue(propertyId_t type,float value){
	if(type>=valueRegistry.size())
		valueRegistry.resize(type+1);
	valueRegistry[type] = value;
}


//------------------------------------------------------

}
