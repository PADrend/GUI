/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_EMBEDDED_FONTS_H
#define GUI_EMBEDDED_FONTS_H


/* Font-info:

	Tempesta Seven Fonts 
	---------------------
	Author: Yusuke Kamiyamane
	See: http://p.yusukekamiyamane.com/fonts/
	License text from website: "Diese Schriftarten dürfen unentgeltlich für persönliche und kommerzielle Projekte verwendet werden."
	2012-04-05
*/

namespace GUI {
class BitmapFont;

namespace EmbeddedFonts{

BitmapFont * createFont();
BitmapFont * createBoldFont();

}

}

#endif // GUI_EMBEDDED_FONTS_H
