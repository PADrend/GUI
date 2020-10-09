/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_SPLITTER_H
#define GUI_SPLITTER_H

#include "Component.h"
#include "../Base/ListenerHelper.h"

namespace GUI{

class Image;

/*! Divides a container into two resizable parts.
	The Splitter should always be the second child of overall three children of the parent.
   Splitter ---|> Component
 */
class Splitter: public Component {
	PROVIDES_TYPE_NAME(Splitter)
	public:
		enum splittingDirection_t { VERTICAL, HORIZONTAL };
		GUIAPI Splitter(GUI_Manager & gui,splittingDirection_t _direction,flag_t flags=0);
		GUIAPI virtual ~Splitter();

		splittingDirection_t getDirection()const				{	return direction;	}

		// ---|> Component
		GUIAPI virtual void doLayout() override;
	private:
		const splittingDirection_t direction;

		MouseButtonListener mouseButtonListener;
		OptionalMouseMotionListener optionalMouseMotionListener;

		// ---|> Component
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

		GUIAPI bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
		GUIAPI bool onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent);
};

}

#endif // GUI_SPLITTER_H
