/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_Scrollbar_H
#define GUI_Scrollbar_H

#include "Container.h"
#include "../Base/ListenerHelper.h"

namespace GUI {
class Button;

/***
 **     Scrollbar ---|> Container
 **/
class Scrollbar : public Container {
		PROVIDES_TYPE_NAME(Scrollbar)
	public:
		// flags
		static const flag_t VERTICAL=1<<24;

		Scrollbar(GUI_Manager & gui, flag_t flags = 0);
		virtual ~Scrollbar();

		int getMarkerSize()const;
		uint32_t getMaxScrollPos()const						{	return maxScrollPos;	}
		bool isVertical()const 								{	return getFlag(VERTICAL);	}
		void setMaxScrollPos(const uint32_t maxScrollPos);
		void setScrollPos(const uint32_t f);
		uint32_t getScrollPos()const						{	return scrollPos;	}

		//! Sets the scrollPos and issues an dataChanged event to registered listeners
		void updateScrollPos(const int32_t f);

		// ---|> Component
		virtual void doLayout() override;

	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;

		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

		float getMarkerPosFromScrollPos(float value)const;

		uint32_t maxScrollPos;
		uint32_t scrollPos;

		Util::WeakPointer<Component> marker;

		MouseButtonListener mouseButtonListener;
};
}
#endif // GUI_Scrollbar_H
