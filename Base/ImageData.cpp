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

#ifdef GUI_BACKEND_RENDERING
#include <Rendering/Texture/Texture.h>
#include <Rendering/Texture/TextureUtils.h>
#else // GUI_BACKEND_RENDERING
#include <GL/glew.h>
#endif // GUI_BACKEND_RENDERING

namespace GUI {
	
//-------------------------------------------------------------------------------------
#ifdef GUI_BACKEND_RENDERING

struct ImageData::InternalData {
	InternalData(Util::Reference<Rendering::Texture> _texture) : texture(_texture) {}
	Util::Reference<Rendering::Texture> texture;
};

//! (ctor)
ImageData::ImageData(Util::Reference<Util::Bitmap> _bitmap):
		ReferenceCounter_t(), data(new InternalData(Rendering::TextureUtils::createTextureFromBitmap(*_bitmap.get()))) { }

//! (ctor)
ImageData::ImageData(Util::Reference<Rendering::Texture> _texture):
		ReferenceCounter_t(), data(new InternalData(_texture)) { }

uint8_t * ImageData::getLocalData() {
	return data->texture->openLocalData(Draw::getRenderingContext());
}

const uint8_t * ImageData::getLocalData() const {
	return data->texture->openLocalData(Draw::getRenderingContext());
}

const Util::Reference<Util::Bitmap> ImageData::getBitmap() const {
	data->texture->openLocalData(Draw::getRenderingContext());
	return data->texture->getLocalBitmap();
}

const Util::Reference<Rendering::Texture>& ImageData::getTexture() const {
	return data->texture;
}

bool ImageData::enable() {
	Draw::enableTexture(this);
	return true;
}

void ImageData::disable() {
	Draw::disableTexture();
}

void ImageData::dataChanged() {
	data->texture->dataChanged();
}

bool ImageData::uploadGLTexture() {
	// ignore
	return true;
}

void ImageData::removeGLData() {
	// ignore
}

uint32_t ImageData::getTextureId() {
	return data->texture->getGLId();
}

//-------------------------------------------------------------------------------------
#else // GUI_BACKEND_RENDERING

struct ImageData::InternalData {
	InternalData(Util::Reference<Util::Bitmap>& _bitmap) : bitmap(_bitmap) {}
	Util::Reference<Util::Bitmap> bitmap;
	uint32_t textureId = 0;
	bool dataHasChanged = true;
};

//! (ctor)
ImageData::ImageData(Util::Reference<Util::Bitmap> _bitmap):
		ReferenceCounter_t(), data(new InternalData(_bitmap)) { }

uint8_t * ImageData::getLocalData() {
	return data->bitmap->data();
}

const uint8_t * ImageData::getLocalData() const {
	return data->bitmap->data();
}

const Util::Reference<Util::Bitmap> ImageData::getBitmap() const {
	return data->bitmap;
}

bool ImageData::enable() {
	if( (data->textureId == 0 || data->dataHasChanged) && !uploadGLTexture() )
		return false;
	Draw::enableTexture(this);
	return true;
}

void ImageData::disable() {
	if (data->textureId != 0)
		Draw::disableTexture();
}

void ImageData::dataChanged() {
	data->dataHasChanged = true;
}

bool ImageData::uploadGLTexture() {
	if( data->textureId == 0 ) {
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1,&data->textureId);
		if(data->textureId != 0) {
			glBindTexture(GL_TEXTURE_2D, data->textureId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	if( data->textureId == 0 )
		return false;
	
	auto pixelFormat = data->bitmap->getPixelFormat();
	GLint glInternalFormat;
	GLint glFormat;
	if(pixelFormat==Util::PixelFormat::RGBA){
		glFormat = GL_RGBA;
		glInternalFormat = GL_RGBA;
	}else if(pixelFormat==Util::PixelFormat::BGRA){
		glFormat = GL_BGRA;
		glInternalFormat = GL_RGBA;
	}else if(pixelFormat==Util::PixelFormat::RGB){
		glFormat = GL_RGB;
		glInternalFormat = GL_RGB;
	}else if(pixelFormat==Util::PixelFormat::BGR){
		glFormat = GL_BGR;
		glInternalFormat = GL_RGB;
	}else if(pixelFormat==Util::PixelFormat::MONO){
		glFormat = GL_RED;
		glInternalFormat = GL_RED;
	}else{
		throw std::invalid_argument("ImageData::uploadGLTexture: Bitmap has unimplemented color format.");
	}
	GLenum glDataType;
	if( pixelFormat.getValueType() == Util::TypeConstant::UINT8 ){
		glDataType = GL_UNSIGNED_BYTE;
	}else if( pixelFormat.getValueType() == Util::TypeConstant::FLOAT ){
		glDataType = GL_FLOAT;
	}else{
		throw std::invalid_argument("ImageData::uploadGLTexture: Bitmap has invalid data format.");
	}

	glBindTexture(GL_TEXTURE_2D, data->textureId);
	glTexImage2D(GL_TEXTURE_2D,0, glInternalFormat,	data->bitmap->getWidth(), data->bitmap->getHeight(), /*border*/0, glFormat, glDataType, data->bitmap->data());
	glBindTexture(GL_TEXTURE_2D, 0);	

	data->dataHasChanged = false;
	return true; 
}

void ImageData::removeGLData() {
	if(data->textureId != 0) {
		GLuint glId = static_cast<GLuint>(data->textureId);
		glDeleteTextures(1,&glId);
	}
	data->textureId = 0;
}

uint32_t ImageData::getTextureId() {
	return data->textureId;
}

//-------------------------------------------------------------------------------------
#endif // GUI_BACKEND_RENDERING

//! (dtor)
ImageData::~ImageData() = default;

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
