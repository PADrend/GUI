/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_IMAGE_H
#define GUI_IMAGE_H

#include "Component.h"
#include "../Base/ImageData.h"

namespace GUI {

/***
 ** Image ---|> Component
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
class Image: public Component {
		PROVIDES_TYPE_NAME(Image)
	public:

		Image(GUI_Manager & gui,const Geometry::Rect & relRect,flag_t flags=0);
		Image(GUI_Manager & gui, const Util::Bitmap & bitmap, flag_t flags=0);

		virtual ~Image();

	public:
		Util::Reference<Util::PixelAccessor> createPixelAccessor();
		void dataChanged(){	
			data->dataChanged();	
			invalidateRegion();
		}
		ImageData * getImageData()const				{	return data.get();	}
		const Util::Reference<Util::Bitmap> & getBitmap() const {
			return data->getBitmap();
		}
		void updateData(const Util::Bitmap & bitmap) {
			data->updateData(bitmap);
			invalidateRegion();
		}

	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region);

		Util::Reference<ImageData> data;

};
}
#endif // GUI_IMAGE_H
