/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_MENU_H
#define GUI_MENU_H

#include "Container.h"
#include "../Base/Listener.h"
#include "../GUI_Manager.h"
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

		Menu(GUI_Manager & gui,flag_t flags=0);
		virtual ~Menu();

		// ---|> Component
		virtual void doLayout() override;

		virtual bool onSelect() override;
		virtual bool onUnselect() override;
	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

		GUI_Manager::KeyListenerHandle keyListenerHandle;
		GUI_Manager::MouseButtonListenerHandle mouseButtonListenerHandle;
		std::unique_ptr<GUI_Manager::FrameListenerHandle> optionalFrameListener;

	public:
		// ---o
		virtual void open(const Geometry::Vec2 &pos);
		virtual void close();
};
}
#endif // GUI_MENU_H
