/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Icon.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "../Base/BasicColors.h"
#include "ComponentPropertyIds.h"
#include "Image.h"
#include <Util/Graphics/Bitmap.h>
#include <iostream>

namespace GUI {

//! (ctor)
Icon::Icon(GUI_Manager & _gui,const Geometry::Vec2 & _pos, Util::WeakPointer<ImageData> _image,const Geometry::Rect & _imageRect,flag_t _flags/*=0*/):
	Component(_gui,_flags){
	setImageData(_image);
	setImageRect(_imageRect);
	setPosition(_pos);
	setSize(_imageRect.getWidth(),_imageRect.getHeight());
	//ctor
}

//! (ctor)
Icon::Icon(GUI_Manager & _gui,const Geometry::Rect & _r,flag_t _flags/*=0*/):
	Component(_gui,_r,_flags){
	//ctor
}


//! (dtor)
Icon::~Icon(){
	//dtor
}

//! ---|> Component
void Icon::doDisplay(const Geometry::Rect & /*region*/){
	if(imageData.isNotNull() && imageData->enable()){
		const size_t width = imageData->getBitmap()->getWidth();
		const size_t height = imageData->getBitmap()->getHeight();
		const float u=imageRect.getMinX()/width;
		const float v=imageRect.getMinY()/height;
		const Geometry::Rect uvRect(u,v,imageRect.getMaxX()/width-u,imageRect.getMaxY()/height-v);
		Draw::drawTexturedRect(Geometry::Rect_i(getLocalRect()),uvRect,getGUI().getActiveColor(PROPERTY_ICON_COLOR),true);
		imageData->disable();
	}else{
		Draw::drawFilledRect(getLocalRect(),Colors::WHITE); // \todo Use Placeholder-shape
	}
}

}
