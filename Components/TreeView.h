/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
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
#include "../Base/ListenerHelper.h"
#include <memory>

namespace GUI {
class Scrollbar;

/***
 **     TreeView ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class TreeView: public Container {
		PROVIDES_TYPE_NAME(TreeView)
	public:
		/***
		 **     TreeView::TreeViewEntry ---|> Container ---|> Component
		 **                            0..1 ------------> *
		 **/
		class TreeViewEntry: public Container {
				PROVIDES_TYPE_NAME(TreeViewEntry)
			public:

				GUIAPI static const Util::StringIdentifier ACTION_TreeViewEntry_collapse;
				GUIAPI static const Util::StringIdentifier ACTION_TreeViewEntry_open;
				
				static const flag_t COLLAPSED_ENTRY = 1<<24;

				GUIAPI TreeViewEntry(GUI_Manager & gui,TreeView * myTreeView,Component * c=nullptr,flag_t flags=0);
				GUIAPI virtual ~TreeViewEntry();

				//! ---|> Component
				GUIAPI virtual void doLayout() override;

				//! (internal) should only be called from within the owning TreeView
				void _setMarked(bool b)			{	marked=b;	}
				bool isMarked()const 			{	return marked;	}
				TreeView * getTreeView()const 	{	return myTreeView;	}
				GUIAPI TreeViewEntry * getFirstSubentry()const;
				bool isCollapsed()const			{	return getFlag(COLLAPSED_ENTRY);	}
				GUIAPI void collapse();
				GUIAPI void open();

				// change the first component of the entry (which is normally only set in the constructor and never changed.)
				GUIAPI void setComponent(const Ref & c);

				// ---|> Container
				GUIAPI virtual void addContent(const Ref & child) override;
				GUIAPI virtual void clearContents() override;
				GUIAPI virtual void removeContent(const Ref & child) override;
				GUIAPI virtual std::vector<Component*> getContents() override;
				GUIAPI virtual void insertAfter(const Ref & child,const Ref & after) override;
				GUIAPI virtual void insertBefore(const Ref & child,const Ref & after) override;

			private:
				// ---|> Component
				GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

				GUIAPI bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);

				GUIAPI void setTreeView( TreeView * myTreeView);
				TreeView * myTreeView;
				bool marked;
				MouseButtonListener mouseButtonListener;

				GUIAPI void unmarkSubtree(Component * root)const;
		};

		GUIAPI TreeView(GUI_Manager & gui,const Geometry::Rect & r,const std::string & actionName="",flag_t flags=0);
		GUIAPI virtual ~TreeView();

		TreeViewEntry * getRootEntry()const					{	return root.get();	}
		Util::StringIdentifier getActionName()const			{	return actionName;	}

		GUIAPI void scroll(float amount);
		GUIAPI void scrollTo(float position);
		GUIAPI void scrollToSelection();
		float getScrollPos()const 							{	return scrollPos;	}

		GUIAPI void markEntry(TreeViewEntry * e);
		GUIAPI void unmarkEntry(TreeViewEntry * e);
		GUIAPI void unmarkAll();

		GUIAPI void markComponent(Component * c);
		GUIAPI void unmarkComponent(Component * c);
		GUIAPI std::vector<Component*> getMarkedComponents();
		GUIAPI void markingChanged();

		// ---|> Container
		GUIAPI virtual void addContent(const Ref & child) override;
		GUIAPI virtual void clearContents() override;
		GUIAPI virtual void removeContent(const Ref & child) override;
		GUIAPI virtual std::vector<Component*> getContents() override;

		// ---|> Component
		GUIAPI virtual void doLayout() override;

	private:
		// ---|> Component
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

		GUIAPI bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		GUIAPI bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
		GUIAPI bool onMouseMove(Component * /*component*/, const Util::UI::MotionEvent & motionEvent);

		Geometry::Vec2 currentMousePos;

	protected:
		std::vector<Util::Reference<TreeViewEntry>> markedEntries;
		Util::StringIdentifier actionName;
		Util::WeakPointer<TreeViewEntry> root;
		float scrollPos;
		bool multiSelect;
		Util::WeakPointer<Scrollbar> scrollBar;
		std::unique_ptr<DataChangeListenerHandle> optionalScrollBarListener;

		KeyListener keyListener;
		MouseButtonListener mouseButtonListener;
		OptionalMouseMotionListener optionalMouseMotionListener;
};
}
#endif // GUI_TreeView_H
