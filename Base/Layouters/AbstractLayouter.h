/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ABSTRACT_LAYOUTER_H
#define GUI_ABSTRACT_LAYOUTER_H

#include <Util/ReferenceCounter.h>
#include <Util/TypeNameMacro.h>

namespace GUI{

class Component;

//! AbstractLayouter
class AbstractLayouter : public Util::ReferenceCounter<AbstractLayouter> {
		PROVIDES_TYPE_NAME(AbstractLayouter)

	public:
		virtual ~AbstractLayouter() {}

		virtual void layout(Util::WeakPointer<Component> component) = 0;
		
};

}

#endif // GUI_ABSTRACT_LAYOUTER_H
