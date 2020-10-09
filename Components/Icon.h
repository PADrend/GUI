/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ICON_H
#define GUI_ICON_H

#include "Component.h"
#include "../Base/ImageData.h"

namespace GUI{


/*! A rectangular part of an Image.
   Icon ---|> Component
	 |------> Image
 */
class Icon: public Component	{
	PROVIDES_TYPE_NAME(Icon)
	public:
		GUIAPI Icon(GUI_Manager & gui,const Geometry::Vec2 & pos, Util::WeakPointer<ImageData> imageData,const Geometry::Rect & imageRect,flag_t flags=0);
		GUIAPI Icon(GUI_Manager & gui,const Geometry::Rect & r,flag_t flags=0);
		GUIAPI virtual ~Icon();

		void setImageData(Util::WeakPointer<ImageData> newImage)	{	imageData=newImage;	}
		void setImageRect(const Geometry::Rect & newImageRect)		{   imageRect=newImageRect;	}

		ImageData * getImageData()const								{	return imageData.get();	}
		Geometry::Rect getImageRect()const							{	return imageRect;	}

	private:
		// ---|> Component
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

	private:
		Util::Reference<ImageData> imageData;
		Geometry::Rect imageRect;
};

}

#endif // GUI_ICON_H
