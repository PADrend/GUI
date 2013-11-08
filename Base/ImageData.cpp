/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "ImageData.h"

#include "Draw.h"
#include <Util/Graphics/PixelAccessor.h>
#include <iostream>

namespace GUI{

//! (ctor)
ImageData::ImageData(Util::Reference<Util::Bitmap> _bitmap):
		ReferenceCounter_t(),
		bitmap(std::move(_bitmap)),
		textureId(0),
		dataHasChanged(false) {
	dataChanged();
}


//! (dtor)
ImageData::~ImageData() {
	removeGLData();
}


bool ImageData::uploadGLTexture() {
	if( textureId==0 )
		textureId = Draw::generateTextureId();
	if( textureId==0 )
		return false;

	Draw::uploadTexture(textureId,bitmap->getWidth(),bitmap->getHeight(),bitmap->getPixelFormat(),getLocalData());

	dataHasChanged=false;
	return true; 
}

uint8_t * ImageData::getLocalData() {
	return bitmap->data();
}

const uint8_t * ImageData::getLocalData() const {
	return bitmap->data();
}

bool ImageData::enable() {
	if( (textureId == 0 || dataHasChanged) && !uploadGLTexture()  )
		return false;

	Draw::enableTexture(textureId);
	return true;
}

void ImageData::disable() {
	if (textureId!=0) 
		Draw::disableTexture();
}

void ImageData::removeGLData() {
	if(textureId!=0)
		Draw::destroyTexture(textureId);
	textureId=0;
}

void ImageData::dataChanged() {
	dataHasChanged=true;
}

void ImageData::updateData(const Util::Bitmap & _bitmap) {
	if(_bitmap.getPixelFormat() != getBitmap()->getPixelFormat()){
		WARN("updateData: Different pixel formats!");
		return;
	}
	std::copy(_bitmap.data(), _bitmap.data() + std::min<size_t>(_bitmap.getDataSize(), getBitmap()->getDataSize()), getLocalData());
	dataChanged();
}

Util::Reference<Util::PixelAccessor> ImageData::createPixelAccessor(){
	return Util::PixelAccessor::create(getBitmap());
}

}
