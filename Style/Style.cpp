/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012,2015 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Style.h"
#include "../Base/BasicColors.h"
#include "../Base/Fonts/AbstractFont.h"
#include "../Base/Fonts/BitmapFont.h"

#include "../Base/StyleManager.h"
#include "../Components/ComponentPropertyIds.h"

#include "EmbeddedFonts.h"
#include "StdShapes.h"
#include "Colors.h"

#include <Util/References.h>

namespace GUI {

//! (static)
AbstractShape * Style::createButtonShape(float _roundnessTL,float _roundnessTR,float _roundnessBL,float _roundnessBR){
	return new Rounded3dRectShape(Colors::BUTTON_BG_1,Colors::BUTTON_BG_2,false,_roundnessTL,_roundnessTR,_roundnessBL,_roundnessBR);
}


//! (static)
AbstractShape * Style::getButtonShape(){
	static Util::Reference<AbstractShape> shape = createButtonShape(2,2,2,2);
	return shape.get();
}

//! (static)
void Style::initStyleManager(StyleManager & m){
	Util::Reference<BitmapFont> defaultFont = EmbeddedFonts::createFont();
	Util::Reference<BitmapFont> boldFont = EmbeddedFonts::createBoldFont();

	// general
	m.setDefaultColor(PROPERTY_TEXT_COLOR , Colors::BLACK);
	m.setDefaultColor(PROPERTY_TOOLTIP_TEXT_COLOR , Colors::WHITE);

	m.setDefaultShape(PROPERTY_COMPONENT_RAISED_BORDER_SHAPE , new Rect3dShape(Colors::NO_COLOR,Colors::NO_COLOR,false,false));
	m.setDefaultShape(PROPERTY_COMPONENT_LOWERED_BORDER_SHAPE , new Rect3dShape(Colors::NO_COLOR,Colors::NO_COLOR,false,true));
	m.setDefaultShape(PROPERTY_COMPONENT_BORDER_SHAPE , new RectShape(Colors::NO_COLOR,Colors::BORDER_COLOR,false));
	m.setDefaultShape(PROPERTY_SELECTION_RECT_SHAPE , new RectShape( Colors::NO_COLOR,Colors::SELECT_COLOR,false));
	
	const Util::Color4ub scrollableColor(0x00,0x00,0x00,0x30);
	m.setDefaultShape(PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE , new ScrollableMarkerShape( 
				scrollableColor,Colors::NO_COLOR,Colors::NO_COLOR,Colors::NO_COLOR,7));
	m.setDefaultShape(PROPERTY_SCROLLABLE_MARKER_RIGHT_SHAPE , new ScrollableMarkerShape( 
				Colors::NO_COLOR,scrollableColor,Colors::NO_COLOR,Colors::NO_COLOR,7));
	m.setDefaultShape(PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE , new ScrollableMarkerShape( 
				Colors::NO_COLOR,Colors::NO_COLOR,scrollableColor,Colors::NO_COLOR,7));
	m.setDefaultShape(PROPERTY_SCROLLABLE_MARKER_LEFT_SHAPE , new ScrollableMarkerShape( 
				Colors::NO_COLOR,Colors::NO_COLOR,Colors::NO_COLOR,scrollableColor,7));

	m.setDefaultFont(PROPERTY_DEFAULT_FONT , defaultFont.get());
	m.setDefaultFont(PROPERTY_TOOLTIP_FONT , defaultFont.get());

	// button
	static Util::Reference<AbstractShape> buttonShape = new Rounded3dRectShape(Colors::BUTTON_BG_1,Colors::BUTTON_BG_2,false,2,2,2,2);
	m.setDefaultShape(PROPERTY_BUTTON_SHAPE,buttonShape.get());
	m.setDefaultShape(PROPERTY_BUTTON_HOVERED_SHAPE,buttonShape.get());
	m.setDefaultColor(PROPERTY_BUTTON_HOVERED_TEXT_COLOR , Colors::WHITE);
	m.setDefaultColor(PROPERTY_BUTTON_ENABLED_COLOR , Colors::WHITE);

	// connector
	m.setDefaultShape(PROPERTY_CONNECTOR_LINE_SHAPE,new StraightLineShape(Colors::BORDER_COLOR,2.0));
	m.setDefaultShape(PROPERTY_CONNECTOR_LINE_SHAPE,new SmoothConnectorShape(Colors::BORDER_COLOR,2.0));

	// icon
	m.setDefaultColor(PROPERTY_ICON_COLOR , Colors::WHITE);

	// menu
	m.setDefaultShape(PROPERTY_MENU_SHAPE , new RectShape(Colors::PASSIVE_COLOR_2,Colors::ACTIVE_COLOR_1,false));
	m.setDefaultShape(PROPERTY_MENU_OUTER_SHAPE , new OuterRectShadowShape(-2,5,-2,5,Util::Color4ub(0x00,0x00,0x00,0x70)));
	m.setDefaultColor(PROPERTY_MENU_TEXT_COLOR , Colors::BLACK);

	// checkbox
	m.setDefaultShape(PROPERTY_CHECKBOX_SHAPE , new Rect3dShape(Colors::BUTTON_BG_1,Colors::BUTTON_BG_2,false));
	m.setDefaultShape(PROPERTY_CHECKBOX_MARKER_SHAPE , new CrossShape(Colors::ACTIVE_COLOR_1,Colors::ACTIVE_COLOR_3,4));
	m.setGlobalValue(PROPERTY_CHECKBOX_LABEL_INDENTATION , 16);

	// ListView
	m.setDefaultShape(PROPERTY_LISTVIEW_SHAPE , new Rect3dShape(Colors::NO_COLOR,Colors::NO_COLOR,false,true));
	m.setDefaultShape(PROPERTY_LISTVIEW_MARKED_ENTRY_SHAPE , new RectShape( Colors::COMPONENT_COLOR_1,Colors::LIGHT_BORDER_COLOR,true));
	m.setGlobalValue(PROPERTY_LISTVIEW_DEFAULT_ENTRY_HEIGHT , 15);

	// Scrollbar
	m.setGlobalValue(PROPERTY_SCROLLBAR_WIDTH , 8);
	m.setDefaultShape(PROPERTY_SCROLLBAR_HORIZONTAL_BAR_SHAPE , new RectShape( Colors::COMPONENT_COLOR_1,Colors::NO_COLOR,true));
	m.setDefaultShape(PROPERTY_SCROLLBAR_VERTICAL_BAR_SHAPE , new RectShape( Colors::COMPONENT_COLOR_1,Colors::NO_COLOR,true));
	m.setDefaultShape(PROPERTY_SCROLLBAR_HORIZONTAL_MARKER_SHAPE , new SliderMarkerShape(Colors::COMPONENT_COLOR_1,Colors::COMPONENT_COLOR_2));
	m.setDefaultShape(PROPERTY_SCROLLBAR_VERTICAL_MARKER_SHAPE , new SliderMarkerShape(Colors::COMPONENT_COLOR_1,Colors::COMPONENT_COLOR_2));


	// slider
	m.setDefaultShape(PROPERTY_SLIDER_BAR_SHAPE , new RectShape( Colors::COMPONENT_COLOR_1,Colors::LIGHT_BORDER_COLOR,true));
	m.setDefaultShape(PROPERTY_SLIDER_MARKER_SHAPE , new SliderMarkerShape(Colors::COMPONENT_COLOR_1,Colors::COMPONENT_COLOR_2));
	m.setDefaultShape(PROPERTY_SLIDER_ZERO_BAR_SHAPE , new RectShape(Colors::NO_COLOR,Colors::ACTIVE_COLOR_1,false));
	m.setGlobalValue(PROPERTY_SLIDER_BUTTON_SIZE , 15);

	// splitter
	m.setDefaultShape(PROPERTY_SPLITTER_SHAPE , new Rect3dShape(Colors::BUTTON_BG_1,Colors::BUTTON_BG_2,true));

	// tab
	m.setDefaultShape(PROPERTY_TAB_BODY_SHAPE , new Rect3dShape(Colors::COMPONENT_COLOR_1, Colors::COMPONENT_COLOR_2,false));
	m.setDefaultShape(PROPERTY_TAB_HEADER_ACTIVE_SHAPE , new TabHeaderShape(Colors::DARK_COLOR, Colors::BRIGHT_COLOR, Colors::COMPONENT_COLOR_1));
	m.setDefaultShape(PROPERTY_TAB_HEADER_PASSIVE_SHAPE , new TabHeaderShape(Colors::NO_COLOR, Colors::COMPONENT_COLOR_1, Colors::BRIGHT_COLOR));
	m.setDefaultColor(PROPERTY_TAB_HEADER_ACTIVE_TEXT_COLOR , Colors::BLACK);
	m.setDefaultColor(PROPERTY_TAB_HEADER_PASSIVE_TEXT_COLOR , Colors::ALMOST_BLACK_COLOR);
	m.setGlobalValue(PROPERTY_TAB_HEADER_HEIGHT , 16);

	// textfield
	m.setDefaultShape(PROPERTY_TEXTFIELD_SHAPE , new RectShape(Colors::TEXTFIELD_BG,Colors::TEXTFIELD_BORDER,true));
	m.setDefaultShape(PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE , new RectShape(Colors::SELECTED_TEXT_BG,Colors::NO_COLOR,true));
	m.setDefaultShape(PROPERTY_TEXTFIELD_CURSOR_SHAPE , new RectShape(Colors::BLACK,Colors::NO_COLOR,false));
	m.setDefaultShape(PROPERTY_TEXTFIELD_CLEAR_TEXT_SHAPE , new CrossShape(Colors::PASSIVE_COLOR_3,Colors::NO_COLOR,2));
	m.setDefaultColor(PROPERTY_TEXTFIELD_TEXT_COLOR , Colors::BLACK);
	m.setDefaultColor(PROPERTY_TEXTFIELD_OPTIONS_TEXT_COLOR , Colors::PASSIVE_COLOR_1);
	m.setGlobalValue(PROPERTY_TEXTFIELD_INDENTATION , 3);
	m.setGlobalValue(PROPERTY_TEXTFIELD_CLEAR_BUTTON_SIZE , 6);

	// treeview
	m.setDefaultShape(PROPERTY_TREEVIEW_SUBROUP_SHAPE , new TriangleSelectorShape(Colors::ACTIVE_COLOR_1));
	m.setDefaultShape(PROPERTY_TREEVIEW_ENTRY_SELECTION_SHAPE , new RectShape( Colors::NO_COLOR,Colors::BRIGHT_COLOR,true));
	m.setDefaultShape(PROPERTY_TREEVIEW_ENTRY_MARKING_SHAPE , new RectShape( Colors::SELECTED_TEXT_BG,Colors::NO_COLOR,true));
	m.setDefaultShape(PROPERTY_TREEVIEW_ACTIVE_INDENTATION_SHAPE , new RectShape( Colors::ACTIVE_COLOR_1,Colors::NO_COLOR,true));
	m.setDefaultShape(PROPERTY_TREEVIEW_PASSIVE_INDENTATION_SHAPE , new RectShape( Colors::PASSIVE_COLOR_2,Colors::NO_COLOR,true));

	// window
	m.setDefaultShape(PROPERTY_WINDOW_RESIZER_SHAPE , new ResizerShape(Colors::DARK_COLOR,true));
	m.setDefaultShape(PROPERTY_WINDOW_ACTIVE_SHAPE , new Rounded3dRectShape(Colors::WINDOW_BG_ACTIVE,Colors::WINDOW_BG_ACTIVE,true,3,3,0,0));
	m.setDefaultShape(PROPERTY_WINDOW_PASSIVE_SHAPE , new Rounded3dRectShape(Colors::WINDOW_BG_PASSIVE,Colors::COMPONENT_COLOR_1,true,3,3,0,0));
	m.setDefaultShape(PROPERTY_WINDOW_BUTTON_SHAPE , new RectShape(Colors::COMPONENT_COLOR_1,Colors::COMPONENT_COLOR_1,true));
	m.setDefaultShape(PROPERTY_WINDOW_PASSIVE_OUTER_SHAPE , new OuterRectShadowShape(3,5,3,5,Util::Color4ub(0x00,0x00,0x00,0x10)));
	m.setDefaultShape(PROPERTY_WINDOW_ACTIVE_OUTER_SHAPE , new OuterRectShadowShape(-4,5,-4,5,Util::Color4ub(0x00,0x00,0x00,0x40)));
	m.setDefaultColor(PROPERTY_WINDOW_TITLE_COLOR , Colors::BLACK);
	
	m.setDefaultFont(PROPERTY_WINDOW_TITLE_FONT , boldFont.get() );
	m.setGlobalValue(PROPERTY_WINDOW_BORDER_SIZE , 2);
	m.setGlobalValue(PROPERTY_WINDOW_TITLEBAR_HEIGHT , 16);

	// mousecursor
	m.setDefaultMouseCursor(PROPERTY_MOUSECURSOR_DEFAULT);

	// misc
	m.setGlobalValue(PROPERTY_KEY_REPEAT_DELAY_1 , 0.5 );
	m.setGlobalValue(PROPERTY_KEY_REPEAT_DELAY_2 , 0.05 );

}

}
