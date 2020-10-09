/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_MENU_H
#define GUI_MENU_H

#include "Container.h"
#include "../Base/ListenerHelper.h"
#include <memory>

namespace GUI {

/***
 **     Menu ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class Menu: public Container {
		PROVIDES_TYPE_NAME(Menu)
	public:
		static const flag_t ONE_TIME_MENU=1<<24;

		GUIAPI Menu(GUI_Manager & gui,flag_t flags=0);
		GUIAPI virtual ~Menu();

		// ---|> Component
		GUIAPI virtual void doLayout() override;

		GUIAPI virtual bool onSelect() override;
		GUIAPI virtual bool onUnselect() override;
	private:
		// ---|> Component
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

		GUIAPI bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		GUIAPI bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

		KeyListener keyListener;
		MouseButtonListener mouseButtonListener;
		std::unique_ptr<FrameListenerHandle> optionalFrameListener;

	public:
		// ---o
		GUIAPI virtual void open(const Geometry::Vec2 &pos);
		GUIAPI virtual void close();
};
}
#endif // GUI_MENU_H
