/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <Util/Graphics/Color.h>
#include <Util/IO/FileName.h>
#include "Components/Component.h"

#include <list>
#include <stack>
#include <list>

// Forward declarations
namespace Util {
class Bitmap;
namespace UI {
union Event;
struct ButtonEvent;
class EventContext;
struct KeyboardEvent;
struct MotionEvent;
class Window;
}
}

namespace GUI {

class AbstractFont;
class AbstractShape;
class Button;
class Checkbox;
class Connector;
class EditorPanel;
class Icon;
class Image;
class ImageData;
class Label;
class ListView;
class NextColumn;
class NextRow;
class Menu;
class Panel;
class AbstractProperty;
class Slider;
class Splitter;
class TabbedPanel;
class Textarea;
class Textfield;
class Window;
class TreeView;
class Entry;
struct ActionListener;
struct DataChangeListener;
struct MouseMotionListener;
struct FrameListener;
class AnimationHandler;
class Style;
class MouseCursorHandler;
class MouseCursor;

/***
 ** GUI_Manager
 **/
class GUI_Manager {
	public:

	// ----------

	//! @name Clipboard access
	//	@{
		void copyStringToClipboard(const std::string & s);
		std::string getStringFromClipboard() const;
	//	@}

	// ----------

	//! @name  Main
	//	@{
	public:
		typedef Component::flag_t flag_t;

		/**
		 * Create a new GUI manager and associate it with the given event
		 * context to receive user interface events.
		 */
		GUI_Manager(Util::UI::EventContext & context);
		~GUI_Manager();
		bool handleEvent(const Util::UI::Event & e);
		void display();
		Geometry::Rect getScreenRect()const;

		//! Associate a window (e.g. X11 or SDL) to the GUI manager
		void setWindow(Util::UI::Window * newWindow) {
			window = newWindow;
		}

		//! Access to the associated window
		Util::UI::Window * getWindow() {
			return window;
		}
	private:
		Util::UI::EventContext & eventContext;

		Util::UI::Window * window;
	//	@}

	// --------------------------------------------------------------------------------

	//! @name Animation handling
	//	@{
	public:
		void addAnimationHandler(AnimationHandler * );
		void finishAnimations(Component * c);
		void stopAnimations(Component * c);
	private:
		void executeAnimations();
//		typedef std::list<Util::SinglePointer<AnimationHandler> > animationHandlerList_t; // BUG!
		typedef std::list<AnimationHandler* > animationHandlerList_t;
		animationHandlerList_t animationHandlerList;
	//	@}

	// ----------

	//! @name Cleanup
	//	@{
	public:
		void markForRemoval(Component *c);
		void cleanup();
	private:
		std::list<Util::Reference<Component> > removalList;
	//	@}

	// ----------

	//! @name  Component management
	//	@{
	private:
		Component::Ref activeComponent;
		Util::Reference<Container> globalContainer;
	public:
		void registerWindow(Component * w);
		void unregisterWindow(Component *w);

		void unselectAll();
		void setActiveComponent(Component * c);
		bool isActiveComponent(const Component * c)const 			{	return activeComponent==c;	}
		Component * getComponentAtPos(const Geometry::Vec2 & pos);
		void selectNext(Component * c);
		void selectPrev(Component * c);
		bool selectFirst(Component * c);
		bool selectLast(Component * c);

		Component * getActiveComponent()							{	return activeComponent.get();	}

		//! Check if the component and all its parents are enabled and contained int the global container.
		bool isCurrentlyEnabled(Component * c)const;

		void closeAllMenus();
	//	@}

	// ----------

	//! @name Debug
	//	@{	
	private:
		uint8_t debugMode; // 0 off, 1 some, 2 all
	public:
		uint8_t getDebugMode()const									{	return debugMode;	}
		void setDebugMode(uint8_t m)								{	debugMode = m;	}
	//	@}

	// ----------

	//! @name Event handling & Listener
	//	@{
	public:
		typedef std::list<ActionListener *> actionListenerList;
		actionListenerList actionListener;
		void registerActionListener(ActionListener * a) {
			if (a)
				actionListener.push_back(a);
		}
		void removeActionListener(ActionListener * a) 				{	actionListener.remove(a);	}

		//----

		typedef std::list<DataChangeListener *> dataChangeListenerList;
		dataChangeListenerList dataChangeListener;
		void registerDataChangeListener(DataChangeListener * a) {
			if (a)
				dataChangeListener.push_back(a);
		}
		void removeDataChangeListener(DataChangeListener * a)		{	dataChangeListener.remove(a);	}

		//----

		typedef std::list<MouseButtonListener *> mouseButtonListenerList;
		mouseButtonListenerList mouseButtonListeners;
		void addMouseButtonListener(MouseButtonListener * a) {
			if (a)
				mouseButtonListeners.push_front(a);
		}
		void removeMouseButtonListener(MouseButtonListener * a)		{	mouseButtonListeners.remove(a);	}

		//----

		typedef std::list<MouseMotionListener *> mouseListenerList;
		mouseListenerList mouseMoveListener;
		void addMouseMotionListener(MouseMotionListener * a) {
			if (a)
				mouseMoveListener.push_back(a);
		}
		void removeMouseMotionListener(MouseMotionListener * a)		{	mouseMoveListener.remove(a);	}

		//----

		typedef std::list<FrameListener *> frameListenerList;
		frameListenerList frameListener;
		void addFrameListener(FrameListener * a) {
			if (a)
				frameListener.push_back(a);
		}
		void removeFrameListener(FrameListener * a)					{	frameListener.remove(a);	}

		//----

		void componentActionPerformed(Component *c,const Util::StringIdentifier & actionName);
		void componentDataChanged(Component *c,const Util::StringIdentifier & actionName);
//		void componentMouseClicked(Component *c,unsigned int button,const Geometry::Vec2 &localPos,bool pressed);

		bool isCtrlPressed() const;
		bool isShiftPressed() const;
		
		void enableKeyRepetition(const Util::UI::KeyboardEvent & keyEvent);
		void disableKeyRepetition();
	private:
		std::unique_ptr<std::pair<float,Util::UI::KeyboardEvent>> keyRepeatInfo; // next activation time(sec), keyboard event

		bool handleMouseMovement(const Util::UI::MotionEvent & motionEvent);
		bool handleMouseButton(const Util::UI::ButtonEvent & buttonEvent);
		bool handleKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
	//	@}

	// ----------

	//! @name Factories
	//	@{
	public:
		Button * createButton(const std::string & text,flag_t flags=0);
		Container * createContainer(const Geometry::Rect & r,flag_t flags=0);
		Connector * createConnector(flag_t flags=0);
		EditorPanel * createEditorPanel(flag_t flags=0);
		Panel * createPanel(flag_t flags=0);
		Checkbox * createCheckbox(const std::string & text="",bool checked=false,flag_t flags=0);
		Icon * createIcon(const Geometry::Vec2 & pos, Util::WeakPointer<ImageData> imageData,const Geometry::Rect & imageRect,flag_t flags=0);
		Icon * createIcon(const Geometry::Rect & r,flag_t flags=0);
		Image * createImage(const Geometry::Rect & r,flag_t flags=0);
		Image * createImage(const Util::FileName & fileName,flag_t flags=0);
		Image * createImage(const Util::Bitmap & bitmap,flag_t flags=0);
		Label * createLabel(const Geometry::Rect & r,const std::string & text="",flag_t flags=0);
		Label * createLabel(const std::string & text="",flag_t flags=0);
		ListView * createListView(flag_t flags=0);
		Menu * createMenu(flag_t flags=0);
		NextColumn * createNextColumn(float additionalSpacing=0.0f);
		NextRow * createNextRow(float additionalSpacing=0.0f);
		Slider * createSlider(const Geometry::Rect & r,float left=0,float right=1,int steps=10,const std::string & dataName="",flag_t flags=0);
		Splitter * createVSplitter(flag_t flags=0);
		Splitter * createHSplitter(flag_t flags=0);
		TabbedPanel * createTabbedPanel(flag_t flags=0);
		Textarea * createTextarea(const std::string &text="",flag_t flags=0);
		Textfield * createTextfield(const std::string &text="",flag_t flags=0);
		TreeView * createTreeView(const Geometry::Rect & r,flag_t flags=0);
		Container * createTreeViewEntry(Component * c);
		Window * createWindow(const Geometry::Rect & r,const std::string & title="",flag_t flags=0);
	//	@}

	// ----------

	//! @name Invalidated regions
	//	@{
	public:
		void invalidateRegion(const Geometry::Rect & region);
		void enableLazyRendering()			{	lazyRendering = true;	}
		void disableLazyRendering()			{	lazyRendering = false;	}
		bool isLazyRenderingEnabled()const	{	return lazyRendering;	}
	private:
		Geometry::Rect invalidRegion;
		bool lazyRendering;
	//	@}

	// ----------

	//! @name Properties, shapes, style and mouse cursor
	//	@{
	private:
		std::unique_ptr<StyleManager> style;
	public:
		StyleManager & getStyleManager()const						{	return *style; }

		void displayLineShape(const propertyId_t id,const std::vector<Geometry::Vec2> & points,uint16_t flags=0);
		void displayShape(const propertyId_t id,const Geometry::Rect & rect,uint16_t flags=0);
	
		void disableProperty(const Util::Reference<AbstractProperty> & p)const;
		void enableProperty(const Util::Reference<AbstractProperty> & p)const;
		Util::Color4ub getActiveColor(const propertyId_t id)const;
		AbstractFont * getActiveFont(const propertyId_t id)const;
		AbstractFont * getDefaultFont(const propertyId_t id)const;

		float getGlobalValue(const propertyId_t id)const;

		void registerMouseCursor(const propertyName_t & id, const Util::Reference<Util::Bitmap> & bitmap, const uint32_t clickX, const uint32_t clickY);
		void removeMouseCursor(const propertyName_t & id);
		void setDefaultColor(const propertyId_t id,const Util::Color4ub & c);
		void setDefaultFont(const propertyId_t id,AbstractFont * f);
		void setDefaultShape(const propertyId_t id,AbstractShape * f);
		void setGlobalValue(const propertyId_t id,float v);
	//	@}

	// ----------

	//! @name Scissor
	//	@{
	public:
		std::stack<Geometry::Rect_i> scissors;
		void pushScissor(const Geometry::Rect_i & r);
		void popScissor();
	//	@}



};
}
#endif // GUI_MANAGER_H
