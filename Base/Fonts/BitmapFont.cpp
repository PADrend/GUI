/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012,2015 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "BitmapFont.h"
#include "../Draw.h"
#include "../BasicColors.h"
#include <Util/Graphics/Bitmap.h>
#include <Util/Graphics/FontRenderer.h>
#include <Util/Graphics/PixelAccessor.h>
#include <Util/IO/FileName.h>
#include <Util/StringUtils.h>

using namespace Geometry;

namespace GUI {

//! (static) Factory
Util::Reference<BitmapFont> BitmapFont::createFont(const Util::FileName & fontFile,uint32_t fontSize,const std::string & charMap_utf8){
	Util::FontRenderer fontRenderer(fontFile.getPath());
	const auto charMap_utf32 = Util::StringUtils::utf8_to_utf32(charMap_utf8);
	auto bitmapAndFontInfo = fontRenderer.createGlyphBitmap(fontSize,charMap_utf32);
	Util::Reference<Util::Bitmap> bitmap = bitmapAndFontInfo.first;
	if(bitmap->getPixelFormat().getComponentCount()!=4){ // alpha values are required!
		const uint32_t width = bitmap->getWidth();
		const uint32_t height = bitmap->getHeight();
		Util::Reference<Util::Bitmap> convertedBitmap = new Util::Bitmap(width,height,Util::PixelFormat::RGBA);
		
		Util::Reference<Util::PixelAccessor> reader( Util::PixelAccessor::create(bitmap.get()));
		Util::Reference<Util::PixelAccessor> writer( Util::PixelAccessor::create(convertedBitmap.get()));
		for(uint32_t y = 0;y<height;++y ){
			for(uint32_t x = 0;x<width;++x ){
				const uint8_t alpha = reader->readSingleValueByte(x,y);
				writer->writeColor(x,y,Util::Color4ub(255,255,255,alpha));
			}
		}
		bitmap = convertedBitmap;
	}
	
	
	Util::Reference<BitmapFont> font = new BitmapFont(new ImageData(bitmap.get()),bitmapAndFontInfo.second.height);
	for(const auto glyph : bitmapAndFontInfo.second.glyphMap){
		font->addGlyph(glyph.first, 
				static_cast<uint32_t>(glyph.second.size.first), static_cast<uint32_t>(glyph.second.size.second), 
				Geometry::Vec2i(glyph.second.position.first, glyph.second.position.second),
				Geometry::Vec2i(glyph.second.offset.first,bitmapAndFontInfo.second.height- glyph.second.offset.second),
				glyph.second.xAdvance);
		
	}
	auto spaceGlyph = font->getGlyph(static_cast<uint32_t>(' '));
	if(spaceGlyph.isValid())
		font->setTabWidth(spaceGlyph.xAdvance*4);

	for(const auto & kerningMapEntry : fontRenderer.createKerningMap(charMap_utf32))
		font->setKerning(kerningMapEntry.first.first, kerningMapEntry.first.second, static_cast<int16_t>(kerningMapEntry.second));
	return font;
}

//!	(ctor)
BitmapFont::BitmapFont(Util::Reference<ImageData> _bitmap,int _lineHeight):
		AbstractFont(_lineHeight),bitmap(std::move(_bitmap)),tabWidth(24){
	//ctor
}

//!	(dtor)
BitmapFont::~BitmapFont(){
	//dtor
}

void BitmapFont::addGlyph(uint32_t characterCode,uint32_t width, uint32_t height, const Geometry::Vec2i & textureOffset,const Geometry::Vec2i & screenOffset, int xAdvance){
	if(bitmap.isNull()){
		glyphs.emplace(characterCode, Glyph(xAdvance));
	}else{
		const uint32_t bitmapWidth = bitmap->getBitmap()->getWidth();
		const uint32_t bitmapHeight = bitmap->getBitmap()->getHeight();
		const Geometry::Rect uvRect(	static_cast<float>(textureOffset.x()) / bitmapWidth,
										static_cast<float>(textureOffset.y()) / bitmapHeight,
										static_cast<float>(width) / bitmapWidth,
										static_cast<float>(height) / bitmapHeight);
		const Geometry::Rect_i screenRect(	screenOffset.x(),screenOffset.y(),width,height );

		glyphs.emplace(characterCode, Glyph(uvRect,screenRect,xAdvance));
	}
}

//!	---|> AbstractFont
void BitmapFont::enable(){
	if(bitmap.isNotNull())
		bitmap->enable();
}

//!	---|> AbstractFont
void BitmapFont::disable(){
	if(bitmap.isNotNull())
		bitmap->disable();
}

//!	---|> AbstractFont
void BitmapFont::renderText( const Vec2 & _pos, const std::string & text, const Util::Color4ub & color){
	std::vector<float> posAndUV;
	posAndUV.reserve(text.length()*24);

	Vec2 pos(round(_pos.getX()),round(_pos.getY()));
	
	uint32_t prevChar = 0;
	size_t cursor = 0;
	while(true){
		auto codePoint = Util::StringUtils::readUTF8Codepoint(text,cursor);
		if(codePoint.second==0) // end of string
			break;

		if(codePoint.first==static_cast<uint32_t>('\n')){
			pos.setY(pos.getY()+getLineHeight());
			pos.setX(_pos.getX());
		}else{
			const Glyph & type = getGlyph(codePoint.first);
			float dx = 0;
			if(!type.isValid()){
				if( codePoint.first == static_cast<uint32_t>('\t') ){ // tab
					dx = static_cast<float>(tabWidth - static_cast<int>(pos.x() - _pos.x())%tabWidth);
				}else{
					const Geometry::Rect r(std::floor(pos.getX()+1.0f) , std::floor(pos.getY()+1.0f) , 5.0f, static_cast<float>(getLineHeight()-1));
					Draw::drawLineRect(r,Colors::WHITE,false);
					dx = 7.0;
				}
			}else{
				const auto kerningIt( kerning.find(std::make_pair(prevChar,codePoint.first)) );
				if(kerningIt!=kerning.end())
					pos.x( pos.x()+kerningIt->second );
				const Geometry::Rect rect = Geometry::Rect(
												std::floor(pos.getX()) + static_cast<float>(type.screenRect.getX()) ,
												std::floor(pos.getY()) + static_cast<float>(type.screenRect.getY()) ,
												static_cast<float>(type.screenRect.getWidth()) ,
												static_cast<float>(type.screenRect.getHeight()));

				posAndUV.push_back(rect.getMinX());	posAndUV.push_back(rect.getMaxY());	posAndUV.push_back(type.uvRect.getMinX());	posAndUV.push_back(type.uvRect.getMaxY());
				posAndUV.push_back(rect.getMaxX());	posAndUV.push_back(rect.getMaxY());	posAndUV.push_back(type.uvRect.getMaxX());	posAndUV.push_back(type.uvRect.getMaxY());
				posAndUV.push_back(rect.getMaxX());	posAndUV.push_back(rect.getMinY());	posAndUV.push_back(type.uvRect.getMaxX());	posAndUV.push_back(type.uvRect.getMinY());

				posAndUV.push_back(rect.getMaxX());	posAndUV.push_back(rect.getMinY());	posAndUV.push_back(type.uvRect.getMaxX());	posAndUV.push_back(type.uvRect.getMinY());
				posAndUV.push_back(rect.getMinX());	posAndUV.push_back(rect.getMinY());	posAndUV.push_back(type.uvRect.getMinX());	posAndUV.push_back(type.uvRect.getMinY());
				posAndUV.push_back(rect.getMinX());	posAndUV.push_back(rect.getMaxY());	posAndUV.push_back(type.uvRect.getMinX());	posAndUV.push_back(type.uvRect.getMaxY());

				dx = static_cast<float>(type.xAdvance);
			}
			pos.setX(pos.getX()+dx);
		}
		
		cursor += codePoint.second;
		prevChar = codePoint.first;
	}

	Draw::drawTexturedTriangles(posAndUV,color,true);
}

//!	---|> AbstractFont
Vec2 BitmapFont::getRenderedTextSize( const std::string & text ){
	float maxX = 0;
	float x = 0;
	float y = 0;

	if(text.length() > 0)
		y=static_cast<float>(getLineHeight());
	
	uint32_t prevChar = 0;
	size_t cursor = 0;
	while(true){
		auto codePoint = Util::StringUtils::readUTF8Codepoint(text,cursor);
		if(codePoint.second==0) // end of string
			break;
	
		if(codePoint.first==static_cast<uint32_t>('\n')){
			y += getLineHeight();
			x = 0;
		}else{
			const auto kerningIt( kerning.find(std::make_pair(prevChar,codePoint.first)) );
			if(kerningIt!=kerning.end())
				x+=kerningIt->second;
			const Glyph & type=getGlyph(codePoint.first);
			if(type.isValid()){
				x += type.xAdvance;
			}else if( codePoint.first == static_cast<uint32_t>('\t') ){ // tab
				x += tabWidth - (static_cast<int>(x)%tabWidth);
			}else{
				x+=6.0f;
			}

			if(x>maxX) maxX = x;
		}
		cursor += codePoint.second;
		prevChar = codePoint.first;
	}
	return Vec2(maxX,y);
}

}
