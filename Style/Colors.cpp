/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Colors.h"

namespace GUI{

const Util::Color4ub Colors::TRANSPARENT_COLOR(0xFF,0xFF,0xFF,0x00);
const Util::Color4ub Colors::BORDER_COLOR(0x00,0x00,0x00,0x80);
const Util::Color4ub Colors::LIGHT_BORDER_COLOR(0x00,0x00,0x00,0x50);
const Util::Color4ub Colors::TEXT_COLOR(0x00,0x00,0x00,0xFF);
const Util::Color4ub Colors::LIGHT_TEXT_COLOR(0x80,0x80,0x80,0x80);
const Util::Color4ub Colors::ACTIVE_TITLE_BG_COLOR_1(0xFF,0xFF,0xFF,0xE0);//Color4ub(0x60,0x60,0x60,0x60);
const Util::Color4ub Colors::ACTIVE_TITLE_BG_COLOR_2(0xF0,0xF0,0xF0,0x20);
const Util::Color4ub Colors::TITLE_BG_COLOR_1(0xF8,0xF8,0xF8,0x40);//Color4ub(187,186,186,0x50);//Color4ub(0x60,0x60,0x60,0x05);
const Util::Color4ub Colors::TITLE_BG_COLOR_2(0xF0,0xF0,0xF0,0x50);
const Util::Color4ub Colors::TITLE_TEXT_COLOR(0x00,0x00,0x00,0x80);

const Util::Color4ub Colors::WINDOW_BG_ACTIVE(0xE8,0xE8,0xE8,0xF8);
const Util::Color4ub Colors::WINDOW_BG_PASSIVE(0xFF,0xFF,0xFF,0xF0);

const Util::Color4ub Colors::COMPONENT_COLOR_1(0xA0,0xA0,0xA0,0x30);
const Util::Color4ub Colors::COMPONENT_COLOR_2(0xF8,0xF8,0xF8,0x90);

const Util::Color4ub Colors::SELECT_COLOR(0,37,79,0xA0);//Color4ub(0xF0,0xF0,0xF0,0x80);
const Util::Color4ub Colors::BRIGHT_COLOR(0xFF,0xFF,0xFF,0x60);
const Util::Color4ub Colors::LIGHT_BRIGHT_COLOR(0xA0,0xA0,0xA0,0x50);
const Util::Color4ub Colors::DARK_COLOR(0x40,0x40,0x40,0x60);
const Util::Color4ub Colors::ALMOST_BLACK_COLOR(0x10,0x10,0x10,0xA0);

const Util::Color4ub Colors::SELECTED_TEXT_BG(0x00,0x80,0xFF,0x60);
const Util::Color4ub Colors::TEXTFIELD_BG(0xFF,0xFF,0xFF,0x80);
const Util::Color4ub Colors::TEXTFIELD_BORDER(0x00,0x00,0x00,0x30);

const Util::Color4ub Colors::BUTTON_BG_1(0xF8,0xF8,0xF8,0x90);
const Util::Color4ub Colors::BUTTON_BG_2(0xE0,0xE0,0xE0,0x70);

const Util::Color4ub Colors::ACTIVE_COLOR_1(0,37,79,0xA0);
const Util::Color4ub Colors::ACTIVE_COLOR_2(255,195,65,0xA0);
const Util::Color4ub Colors::ACTIVE_COLOR_3(251,112,22,0xA0);
const Util::Color4ub Colors::ACTIVE_COLOR_4(94,165,38,0xA0);
const Util::Color4ub Colors::PASSIVE_COLOR_1(124,156,191,0x50);
const Util::Color4ub Colors::PASSIVE_COLOR_2(187,186,186,0x50);
const Util::Color4ub Colors::PASSIVE_COLOR_3(181,178,141,0x50);

const Util::Color4ub Colors::PASSIVE_COLOR_4(0xc4,0xcc,0xd5,0x50); // blue grey #b7c6d5 c4ccd5

}
