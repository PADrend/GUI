/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_IMAGE_DATA_H
#define GUI_IMAGE_DATA_H

#include <Util/ReferenceCounter.h>
#include <Util/References.h>
#include <cstdint>
#include <memory>

namespace Util{
class Bitmap;
class PixelAccessor;
}
#ifdef GUI_BACKEND_RENDERING
namespace Rendering {
class Texture;
} /* Rendering */
#endif
namespace GUI {

/***
 ** ImageData ---|> ReferenceCounter<ImageData>
 **
 **	Coordinates:
 **
 ** (0,0)                  (width,0)
 **       +---------------+
 **       |      /\       |
 **       |     /  \      |
 **       |      ||       |
 **       |      ||       |
 **       +---------------+
 ** (0,height)             (width,height)
 **
 ** \note the coordinates are the same as used in Util::Bitmap
 **/
class ImageData: public Util::ReferenceCounter<ImageData> {
	public:

		GUIAPI ImageData(Util::Reference<Util::Bitmap> _bitmap);
#ifdef GUI_BACKEND_RENDERING
		GUIAPI ImageData(Util::Reference<Rendering::Texture> _texture);
#endif
		GUIAPI ~ImageData();

	public:
		GUIAPI uint8_t * getLocalData();
		GUIAPI const uint8_t * getLocalData() const;

		GUIAPI const Util::Reference<Util::Bitmap> getBitmap() const;
#ifdef GUI_BACKEND_RENDERING
		GUIAPI const Util::Reference<Rendering::Texture> & getTexture() const;
#endif
		GUIAPI void updateData(const Util::Bitmap & bitmap);

		GUIAPI bool enable();
		GUIAPI void disable();
		GUIAPI void dataChanged();

		GUIAPI Util::Reference<Util::PixelAccessor> createPixelAccessor();
		
		GUIAPI bool uploadGLTexture();
		GUIAPI void removeGLData();
		GUIAPI uint32_t getTextureId();
	private:
		struct InternalData;
		std::unique_ptr<InternalData> data;
};
}
#endif // GUI_IMAGE_DATA_H
