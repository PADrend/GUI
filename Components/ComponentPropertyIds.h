/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012,2015 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef PROPERTYIDS_H_INCLUDED
#define PROPERTYIDS_H_INCLUDED

#include <cstdint>

namespace GUI {

typedef uint8_t propertyId_t;
typedef std::string propertyName_t;

// ---
// colors

static const propertyId_t PROPERTY_BUTTON_ENABLED_COLOR 			= 1;
static const propertyId_t PROPERTY_BUTTON_HOVERED_TEXT_COLOR 		= 2;
static const propertyId_t PROPERTY_ICON_COLOR				 		= 3;
static const propertyId_t PROPERTY_MENU_TEXT_COLOR					= 4;
static const propertyId_t PROPERTY_TEXT_COLOR 						= 5;
static const propertyId_t PROPERTY_TOOLTIP_TEXT_COLOR 				= 6;
static const propertyId_t PROPERTY_TAB_HEADER_ACTIVE_TEXT_COLOR 	= 7;
static const propertyId_t PROPERTY_TAB_HEADER_PASSIVE_TEXT_COLOR 	= 8;
static const propertyId_t PROPERTY_TEXTFIELD_TEXT_COLOR 			= 9;
static const propertyId_t PROPERTY_TEXTFIELD_OPTIONS_TEXT_COLOR 	= 10;
static const propertyId_t PROPERTY_WINDOW_TITLE_COLOR 				= 11;
// ---
// fonts
static const propertyId_t PROPERTY_DEFAULT_FONT			 			= 1;
static const propertyId_t PROPERTY_TOOLTIP_FONT			 			= 2;
static const propertyId_t PROPERTY_WINDOW_TITLE_FONT			 	= 3;

// ---
// shapes

static const propertyId_t PROPERTY_SELECTION_RECT_SHAPE 			= 1;
static const propertyId_t PROPERTY_BUTTON_SHAPE 					= 2;
static const propertyId_t PROPERTY_BUTTON_HOVERED_SHAPE				= 3;
static const propertyId_t PROPERTY_CHECKBOX_SHAPE 					= 4;
static const propertyId_t PROPERTY_CHECKBOX_MARKER_SHAPE 			= 5;
static const propertyId_t PROPERTY_COMPONENT_BACKGROUND_SHAPE 		= 10;
static const propertyId_t PROPERTY_COMPONENT_ADDITIONAL_BACKGROUND_SHAPE	= 11;
static const propertyId_t PROPERTY_COMPONENT_BORDER_SHAPE 			= 12;
static const propertyId_t PROPERTY_COMPONENT_RAISED_BORDER_SHAPE 	= 13;
static const propertyId_t PROPERTY_COMPONENT_LOWERED_BORDER_SHAPE 	= 14;
static const propertyId_t PROPERTY_COMPONENT_HOVER_SHAPE		 	= 15;
static const propertyId_t PROPERTY_CONNECTOR_LINE_SHAPE		 		= 16;
static const propertyId_t PROPERTY_MENU_SHAPE 						= 17;
static const propertyId_t PROPERTY_LISTVIEW_SHAPE					= 18;
static const propertyId_t PROPERTY_LISTVIEW_MARKED_ENTRY_SHAPE		= 19;
static const propertyId_t PROPERTY_SCROLLBAR_HORIZONTAL_BAR_SHAPE	= 20;
static const propertyId_t PROPERTY_SCROLLBAR_HORIZONTAL_MARKER_SHAPE= 21;
static const propertyId_t PROPERTY_SCROLLBAR_VERTICAL_BAR_SHAPE		= 22;
static const propertyId_t PROPERTY_SCROLLBAR_VERTICAL_MARKER_SHAPE	= 23;
static const propertyId_t PROPERTY_SCROLLABLE_MARKER_TOP_SHAPE		= 30;
static const propertyId_t PROPERTY_SCROLLABLE_MARKER_BOTTOM_SHAPE	= 31;
static const propertyId_t PROPERTY_SCROLLABLE_MARKER_LEFT_SHAPE		= 32;
static const propertyId_t PROPERTY_SCROLLABLE_MARKER_RIGHT_SHAPE	= 33;
static const propertyId_t PROPERTY_SLIDER_BAR_SHAPE 				= 34;
static const propertyId_t PROPERTY_SLIDER_MARKER_SHAPE 				= 35;
static const propertyId_t PROPERTY_SLIDER_ZERO_BAR_SHAPE 			= 36;
static const propertyId_t PROPERTY_SPLITTER_SHAPE 					= 37;
static const propertyId_t PROPERTY_TAB_HEADER_SHAPE 				= 38;
static const propertyId_t PROPERTY_TAB_BODY_SHAPE 					= 39;
static const propertyId_t PROPERTY_TEXTFIELD_SHAPE 					= 50;
static const propertyId_t PROPERTY_TEXTFIELD_CLEAR_TEXT_SHAPE 		= 51;
static const propertyId_t PROPERTY_TEXTFIELD_TEXT_SELECTION_SHAPE 	= 52;
static const propertyId_t PROPERTY_TEXTFIELD_CURSOR_SHAPE 			= 53;
static const propertyId_t PROPERTY_TREEVIEW_SUBROUP_SHAPE			= 60;
static const propertyId_t PROPERTY_TREEVIEW_ENTRY_SELECTION_SHAPE	= 61;
static const propertyId_t PROPERTY_TREEVIEW_ENTRY_MARKING_SHAPE		= 62;
static const propertyId_t PROPERTY_TREEVIEW_ACTIVE_INDENTATION_SHAPE= 63;
static const propertyId_t PROPERTY_TREEVIEW_PASSIVE_INDENTATION_SHAPE=64;
static const propertyId_t PROPERTY_WINDOW_RESIZER_SHAPE				= 65;
static const propertyId_t PROPERTY_WINDOW_ACTIVE_SHAPE				= 66;
static const propertyId_t PROPERTY_WINDOW_PASSIVE_SHAPE				= 67;
static const propertyId_t PROPERTY_WINDOW_BUTTON_SHAPE				= 68;
    
// ---
// MouseCursors
static const propertyName_t PROPERTY_MOUSECURSOR_DEFAULT			= "DEFAULT";
static const propertyName_t PROPERTY_MOUSECURSOR_COMPONENTS			= "COMPONENTCURSOR";
static const propertyName_t PROPERTY_MOUSECURSOR_TEXTFIELD			= "TEXTFIELD";
static const propertyName_t PROPERTY_MOUSECURSOR_RESIZEDIAGONAL		= "RESIZE_DIAGONAL";

// ---
// values
static const propertyId_t PROPERTY_CHECKBOX_LABEL_INDENTATION		= 1;
static const propertyId_t PROPERTY_LISTVIEW_DEFAULT_ENTRY_HEIGHT	= 2;
static const propertyId_t PROPERTY_SCROLLBAR_WIDTH		 			= 3;
static const propertyId_t PROPERTY_SLIDER_BUTTON_SIZE	 			= 4;
static const propertyId_t PROPERTY_TAB_HEADER_HEIGHT	 			= 5;
static const propertyId_t PROPERTY_TEXTFIELD_CLEAR_BUTTON_SIZE		= 6;
static const propertyId_t PROPERTY_TEXTFIELD_INDENTATION 			= 7;

static const propertyId_t PROPERTY_KEY_REPEAT_DELAY_1 				= 8;
static const propertyId_t PROPERTY_KEY_REPEAT_DELAY_2 				= 9;
static const propertyId_t PROPERTY_WINDOW_BORDER_SIZE				= 10;
static const propertyId_t PROPERTY_WINDOW_TITLEBAR_HEIGHT			= 11;
}

#endif // PROPERTYIDS_H_INCLUDED
