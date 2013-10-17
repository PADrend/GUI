/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_LayoutHelper_H
#define GUI_LayoutHelper_H

#include "Component.h"

namespace GUI {

class NextRow: public Component	{
		PROVIDES_TYPE_NAME(NextRow)
	public:
		NextRow(GUI_Manager & _gui,float _additionalSpacing) : Component(_gui),additionalSpacing(_additionalSpacing) {}
		NextRow(const NextRow & c) : Component(c),additionalSpacing(c.additionalSpacing) {}
		virtual ~NextRow(){}

		// ---|> Component
		virtual Component * clone()const 		{ 	return new NextRow(*this);	}
		float additionalSpacing;
};

class NextColumn: public Component	{
		PROVIDES_TYPE_NAME(NextColumn)
	public:
		NextColumn(GUI_Manager & _gui,float _additionalSpacing) : Component(_gui),additionalSpacing(_additionalSpacing) {}

		NextColumn(const NextColumn & c) : Component(c),additionalSpacing(c.additionalSpacing) {}
		virtual ~NextColumn() {}

		// ---|> Component
		virtual Component * clone()const 		{	return new NextColumn(*this);	}
		float additionalSpacing;
};

}
#endif // GUI_LayoutHelper_H
