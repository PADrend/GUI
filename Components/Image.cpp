/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Image.h"
#include "../GUI_Manager.h"
#include "../Base/Draw.h"
#include "../Base/BasicColors.h"
#include <Util/Graphics/PixelAccessor.h>
#include <iostream>

namespace GUI {

// -------------------------------------------------------------------------

//! (ctor)
Image::Image(GUI_Manager & _gui,const Geometry::Rect & _r,flag_t _flags/*=0*/):
		Component(_gui,_r,_flags),data(nullptr) {

	data = new ImageData(new Util::Bitmap( static_cast<uint32_t>(_r.getWidth()), static_cast<uint32_t>(_r.getHeight()), Util::PixelFormat::RGBA ));
	Util::Reference<Util::PixelAccessor> p = data->createPixelAccessor();
	p->fill(0,0,static_cast<uint32_t>(_r.getWidth()), static_cast<uint32_t>(_r.getHeight()),Colors::BLACK);
}

//! (ctor)
Image::Image(GUI_Manager & _gui, 
			 const Util::Bitmap & _bitmap, 
			 flag_t _flags/*=0*/):
		Component(_gui,_flags),
		data(new ImageData(new Util::Bitmap(_bitmap))) {
	setWidth(static_cast<float>(_bitmap.getWidth()));
	setHeight(static_cast<float>(_bitmap.getHeight()));
}

//! (dtor)
Image::~Image() = default;

void Image::doDisplay(const Geometry::Rect & /*region*/) {
	enableLocalDisplayProperties();
	displayDefaultShapes();	
	data->enable();
	Draw::drawTexturedRect(Geometry::Rect_i(getLocalRect()),Geometry::Rect(0,0,1,1),Colors::WHITE,true);
	data->disable();
	disableLocalDisplayProperties();
}

Util::Reference<Util::PixelAccessor> Image::createPixelAccessor() {
	return data->createPixelAccessor();
}

}
