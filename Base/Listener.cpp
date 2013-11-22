/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Listener.h"

namespace GUI {

/*! (static, singleton) */
RegisteredListenerRegistry<MouseMotionListener> & MouseMotionListener::getListenerRegistry(){
	static RegisteredListenerRegistry<MouseMotionListener> l;
	return l;
}

/*! (static, singleton) */
RegisteredListenerRegistry<MouseButtonListener> & MouseButtonListener::getListenerRegistry(){
	static RegisteredListenerRegistry<MouseButtonListener> l;
	return l;
}

/*! (static, singleton) */
RegisteredListenerRegistry<MouseClickListener> & MouseClickListener::getListenerRegistry(){
	static RegisteredListenerRegistry<MouseClickListener> l;
	return l;
}

}
