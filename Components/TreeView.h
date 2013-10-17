/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_TREEVIEW_H
#define GUI_TREEVIEW_H

#include "Container.h"
#include "Label.h"
#include "../Base/Listener.h"

namespace GUI {
class Scrollbar;

/***
 **     TreeView ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class TreeView: public Container,public DataChangeListener,public MouseButtonListener,public MouseMotionListener, public KeyListener{
		PROVIDES_TYPE_NAME(TreeView)
	public:
		/***
		 **     TreeView::TreeViewEntry ---|> Container ---|> Component
		 **                            0..1 ------------> *
		 **/
		class TreeViewEntry: public Container,public MouseButtonListener {
				PROVIDES_TYPE_NAME(TreeViewEntry)
			public:

				static const Util::StringIdentifier ACTION_TreeViewEntry_collapse;
				static const Util::StringIdentifier ACTION_TreeViewEntry_open;
				
				static const flag_t COLLAPSED_ENTRY = 1<<24;

				TreeViewEntry(GUI_Manager & gui,TreeView * myTreeView,Component * c=nullptr,flag_t flags=0);
				virtual ~TreeViewEntry();

				//! ---|> Component
				virtual void doLayout();

				//! (internal) should only be called from within the owning TreeView
				void _setMarked(bool b)			{	marked=b;	}
				bool isMarked()const 			{	return marked;	}
				TreeView * getTreeView()const 	{	return myTreeView;	}
				TreeViewEntry * getFirstSubentry()const;
				bool isCollapsed()const			{	return getFlag(COLLAPSED_ENTRY);	}
				void collapse();
				void open();

				// change the first component of the entry (which is normally only set in the constructor and never changed.)
				void setComponent(const Ref & c);

				// ---|> MouseButtonListener
				virtual listenerResult_t onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

				// ---|> Container
				virtual void addContent(const Ref & child);
				virtual void clearContents();
				virtual void removeContent(const Ref & child);
				virtual std::vector<Component*> getContents();
				virtual void insertAfter(const Ref & child,const Ref & after);
				virtual void insertBefore(const Ref & child,const Ref & after);

			private:
				// ---|> Component
				virtual void doDisplay(const Geometry::Rect & region);

				void setTreeView( TreeView * myTreeView);
				TreeView * myTreeView;
				bool marked;

				void unmarkSubtree(Component * root)const;
		};


		TreeView(GUI_Manager & gui,const std::string & actionName="",flag_t flags=0);
		TreeView(GUI_Manager & gui,const Geometry::Rect & r,const std::string & actionName="",flag_t flags=0);
		virtual ~TreeView();

		TreeViewEntry * getRootEntry()const					{	return root.get();	}
		Util::StringIdentifier getActionName()const			{	return actionName;	}

		void scroll(float amount);
		void scrollTo(float position);
		void scrollToSelection();
		float getScrollPos()const 							{	return scrollPos;	}

		void markEntry(TreeViewEntry * e);
		void unmarkEntry(TreeViewEntry * e);
		void unmarkAll();

		void markComponent(Component * c);
		void unmarkComponent(Component * c);
		std::vector<Component*> getMarkedComponents();
		void markingChanged();


		// ---|> DataChangeListener
		virtual void handleDataChange(Component *,const Util::StringIdentifier & actionName);
		// ---|> MouseButtonListener
		virtual listenerResult_t onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
		// ---|> MouseMotionListener
		virtual listenerResult_t onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent);
		// ---|> KeyListener
		virtual bool onKeyEvent(Component * component, const Util::UI::KeyboardEvent & keyEvent);

		// ---|> Container
		virtual void addContent(const Ref & child);
		virtual void clearContents();
		virtual void removeContent(const Ref & child);
		virtual std::vector<Component*> getContents();

		// ---|> Component
		virtual void doLayout();

	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region);

		Geometry::Vec2 currentMousePos;

	protected:
		void init();

		std::vector<Util::Reference<TreeViewEntry>> markedEntries;
		Util::StringIdentifier actionName;
		Util::WeakPointer<TreeViewEntry> root;
		float scrollPos;
		bool multiSelect;
		Util::WeakPointer<Scrollbar> scrollBar;
};
}
#endif // GUI_TreeView_H
