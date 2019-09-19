/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include "Base/Listener.h"
#include "Components/Component.h"
#include <GUI/config.h>
#include <Util/Graphics/Color.h>
#include <Util/Registry.h>
#include <Util/AttributeProvider.h>

#include <list>
#include <stack>
#include <utility>

// Forward declarations
namespace Util {
class Bitmap;
class FileName;
namespace UI {
union Event;
struct ButtonEvent;
class EventContext;
struct KeyboardEvent;
struct MotionEvent;
class Window;
}
}

namespace Rendering {
class RenderingContext;
} /* Rendering */

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
class DisplayProperty;
class Slider;
class Splitter;
class TabbedPanel;
class Textarea;
class Textfield;
class Window;
class TreeView;
class Entry;
class AnimationHandler;
class Style;
class MouseCursorHandler;
class MouseCursor;
class TooltipHandler;

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
		GUI_Manager(Util::UI::EventContext * context=nullptr);
		~GUI_Manager();
		bool handleEvent(const Util::UI::Event & e);
	#ifdef GUI_BACKEND_RENDERING
		void display(Rendering::RenderingContext& rc);
	#else // GUI_BACKEND_RENDERING
		void display();
	#endif // GUI_BACKEND_RENDERING
		Geometry::Rect getScreenRect()const;

		//! Associate a window (e.g. X11 or SDL) to the GUI manager
		void setWindow(Util::UI::Window * newWindow) {
			window = newWindow;
		}

		//! Access to the associated window
		Util::UI::Window * getWindow() {
			return window;
		}
		
		Util::AttributeProvider userData;
	private:
		Util::UI::EventContext * eventContext;
		Util::UI::Window * window;
		std::string alternativeClipboard; // used if no window is available to provide the clipboard.
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
		typedef std::vector<std::unique_ptr<AnimationHandler>> animationHandlerList_t;
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
	private:
		ActionListenerRegistry actionListener;
	public:
		ActionListenerHandle addActionListener(HandleActionFun fun) {
			return actionListener.registerElement(std::move(fun));
		}
		void removeActionListener(ActionListenerHandle handle) {
			actionListener.unregisterElement(std::move(handle));
		}

	private:
		typedef std::unordered_map<const Component *, ComponentDestructionListenerRegistry> ComponentDestructionListenerMap;
		ComponentDestructionListenerMap componentDestructionListener;
	public:
		ComponentDestructionListenerHandle addComponentDestructionListener(const Component * component, 
																		   HandleComponentDestructionFun fun) {
			return componentDestructionListener[component].registerElement(std::move(fun));
		}
		void removeComponentDestructionListener(const Component * component, 
												ComponentDestructionListenerHandle handle) {
			const auto it = componentDestructionListener.find(component);
			if(it != componentDestructionListener.cend()) {
				it->second.unregisterElement(std::move(handle));
				if(it->second.getElements().empty()) {
					componentDestructionListener.erase(it);
				}
			}
		}

	private:
		typedef std::unordered_map<Component *, DataChangeListenerRegistry> DataChangeListenerMap;
		DataChangeListenerMap dataChangeListener;
	public:
		DataChangeListenerHandle addDataChangeListener(Component * component, HandleDataChangeFun fun) {
			return dataChangeListener[component].registerElement(std::move(fun));
		}
		void removeDataChangeListener(Component * component, DataChangeListenerHandle handle) {
			const auto it = dataChangeListener.find(component);
			if(it != dataChangeListener.cend()) {
				it->second.unregisterElement(std::move(handle));
				if(it->second.getElements().empty()) {
					dataChangeListener.erase(it);
				}
			}
		}
		DataChangeListenerHandle addGlobalDataChangeListener(HandleDataChangeFun fun) {
			// Use nullptr as component to access global registry.
			return addDataChangeListener(nullptr, std::move(fun));
		}
		void removeGlobalDataChangeListener(DataChangeListenerHandle handle) {
			// Use nullptr as component to access global registry.
			removeDataChangeListener(nullptr, std::move(handle));
		}

	private:
		FrameListenerRegistry frameListener;
	public:
		FrameListenerHandle addFrameListener(FrameListenerFun fun) {
			return frameListener.registerElement(std::move(fun));
		}
		void removeFrameListener(FrameListenerHandle handle) {
			frameListener.unregisterElement(std::move(handle));
		}

	private:
		typedef std::unordered_map<Component *, KeyListenerRegistry> KeyListenerMap;
		KeyListenerMap keyListener;
	public:
		KeyListenerHandle addKeyListener(Component * component, HandleKeyFun fun) {
			return keyListener[component].registerElement(std::move(fun));
		}
		void removeKeyListener(Component * component, KeyListenerHandle handle) {
			const auto it = keyListener.find(component);
			if(it != keyListener.cend()) {
				it->second.unregisterElement(std::move(handle));
				if(it->second.getElements().empty()) {
					keyListener.erase(it);
				}
			}
		}

	private:
		typedef std::unordered_map<Component *, MouseButtonListenerRegistry> MouseButtonListenerMap;
		MouseButtonListenerMap mouseButtonListener;
	public:
		MouseButtonListenerHandle addMouseButtonListener(Component * component, HandleMouseButtonFun fun) {
			return mouseButtonListener[component].registerElement(std::move(fun));
		}
		void removeMouseButtonListener(Component * component, MouseButtonListenerHandle handle) {
			const auto it = mouseButtonListener.find(component);
			if(it != mouseButtonListener.cend()) {
				it->second.unregisterElement(std::move(handle));
				if(it->second.getElements().empty()) {
					mouseButtonListener.erase(it);
				}
			}
		}
		MouseButtonListenerHandle addGlobalMouseButtonListener(HandleMouseButtonFun fun) {
			// Use nullptr as component to access global registry.
			return addMouseButtonListener(nullptr, std::move(fun));
		}
		void removeGlobalMouseButtonListener(MouseButtonListenerHandle handle) {
			// Use nullptr as component to access global registry.
			removeMouseButtonListener(nullptr, std::move(handle));
		}

	private:
		typedef std::unordered_map<Component *, MouseClickListenerRegistry> MouseClickListenerMap;
		MouseClickListenerMap mouseClickListener;
	public:
		MouseClickListenerHandle addMouseClickListener(Component * component, HandleMouseClickFun fun) {
			return mouseClickListener[component].registerElement(std::move(fun));
		}
		void removeMouseClickListener(Component * component, MouseClickListenerHandle handle) {
			const auto it = mouseClickListener.find(component);
			if(it != mouseClickListener.cend()) {
				it->second.unregisterElement(std::move(handle));
				if(it->second.getElements().empty()) {
					mouseClickListener.erase(it);
				}
			}
		}

	private:
		MouseMotionListenerRegistry globalMouseMotionListener;
	public:
		MouseMotionListenerHandle addGlobalMouseMotionListener(HandleMouseMotionFun fun) {
			return globalMouseMotionListener.registerElement(std::move(fun));
		}
		void removeGlobalMouseMotionListener(MouseMotionListenerHandle handle) {
			globalMouseMotionListener.unregisterElement(std::move(handle));
		}



		void componentActionPerformed(Component *c,const Util::StringIdentifier & actionName);
		void componentDataChanged(Component * c);
		void componentDestruction(const Component * component);

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
		Slider * createSlider(const Geometry::Rect & r,float left=0,float right=1,int steps=10,flag_t flags=0);
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
	
		void disableProperty(const Util::Reference<DisplayProperty> & p)const;
		void enableProperty(const Util::Reference<DisplayProperty> & p)const;
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

	//!	@name Internal state
	//	@{
	private:
		std::unique_ptr<MouseCursorHandler> mouseCursorHandler;
		std::unique_ptr<TooltipHandler> tooltipHandler;
	//	@}

};
}
#endif // GUI_MANAGER_H
