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

namespace Util{
class Bitmap;
class PixelAccessor;
}
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

		ImageData(Util::Reference<Util::Bitmap> _bitmap);
		~ImageData();

	public:
		bool uploadGLTexture();

		uint8_t * getLocalData();
		const uint8_t * getLocalData() const;

		const Util::Reference<Util::Bitmap> & getBitmap() const {
			return bitmap;
		}

		void updateData(const Util::Bitmap & bitmap);

		bool enable();
		void disable();
		void dataChanged();

		void removeGLData();

		Util::Reference<Util::PixelAccessor> createPixelAccessor();

	private:
		Util::Reference<Util::Bitmap> bitmap;
		uint32_t textureId;
		bool dataHasChanged;
};
}
#endif // GUI_IMAGE_DATA_H
