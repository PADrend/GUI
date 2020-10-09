/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_Panel_H
#define GUI_Panel_H

#include "ScrollableContainer.h"
#include "LayoutHelper.h"

namespace GUI {
class Scrollbar;
class FlowLayouter;
/***
 **     Panel ---|> ScrollableContainer
 **/
class Panel: public ScrollableContainer {
		PROVIDES_TYPE_NAME(Panel)
	public:
		typedef GUI::NextRow NextRow; //! @deprecated  Only for backward compatibility!
		typedef GUI::NextColumn NextColumn; //! @deprecated  Only for backward compatibility!
		
		static const flag_t AUTO_LAYOUT=1<<24; //! unused!

		GUIAPI Panel(GUI_Manager & gui,flag_t flags=0);
		GUIAPI virtual ~Panel();

		GUIAPI void nextRow(float additionalSpacing=0);
		GUIAPI void nextColumn(float additionalSpacing=0);

		GUIAPI void disableAutoBreak();
		GUIAPI void enableAutoBreak();

		GUIAPI void setMargin(int _margin);
		GUIAPI void setPadding(int _padding);

	private:
		GUIAPI FlowLayouter & accessLayouter();
};
}
#endif // GUI_Panel_H
