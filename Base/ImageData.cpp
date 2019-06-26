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
#include <Rendering/Texture/Texture.h>
#include <Rendering/Texture/TextureUtils.h>
#include <iostream>

namespace GUI {
using namespace Rendering;

//! (ctor)
ImageData::ImageData(Util::Reference<Util::Bitmap> _bitmap):
		ReferenceCounter_t(),
		texture(TextureUtils::createTextureFromBitmap(*_bitmap.get())) { }

//! (ctor)
ImageData::ImageData(Util::Reference<Rendering::Texture> _texture):
		ReferenceCounter_t(),
		texture(std::move(_texture)) { }

//! (dtor)
ImageData::~ImageData() { }


uint8_t * ImageData::getLocalData() {
	return texture->openLocalData(Draw::getRenderingContext());
}

const uint8_t * ImageData::getLocalData() const {
	return texture->openLocalData(Draw::getRenderingContext());
}

const Util::Reference<Util::Bitmap> ImageData::getBitmap() const {
	texture->openLocalData(Draw::getRenderingContext());
	return texture->getLocalBitmap();
}

bool ImageData::enable() {
	Draw::enableTexture(texture.get());
	return true;
}

void ImageData::disable() {
	Draw::disableTexture();
}

void ImageData::dataChanged() {
	texture->dataChanged();
}

void ImageData::updateData(const Util::Bitmap & _bitmap) {
	if(_bitmap.getPixelFormat() != texture->getLocalBitmap()->getPixelFormat()){
		WARN("updateData: Different pixel formats!");
		return;
	}
	std::copy(_bitmap.data(), _bitmap.data() + std::min<size_t>(_bitmap.getDataSize(), getBitmap()->getDataSize()), getLocalData());
		texture->dataChanged();
}

Util::Reference<Util::PixelAccessor> ImageData::createPixelAccessor(){
	return Util::PixelAccessor::create(getBitmap());
}

}
