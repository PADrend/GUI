/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ListView_H
#define GUI_ListView_H

#include "Container.h"
#include "../Base/ListenerHelper.h"
#include <memory>
#include <set>
#include <vector>

namespace GUI {
class Scrollbar;
/***
 **     ListView ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class ListView : public Container {
		PROVIDES_TYPE_NAME(ListView)

	// ------------------

	//! @name MAIN
	//	@{
public:
		// flags
		GUIAPI static const flag_t AT_LEAST_ONE_MARKING;
		GUIAPI static const flag_t AT_MOST_ONE_MARKING;

		GUIAPI ListView(GUI_Manager & gui,flag_t flags=0);
		GUIAPI virtual ~ListView();

		// ---|> Component
		Container * getContentContainer()const					{	return clientArea.get();	}
		float getEntryHeight()const							{	return entryHeight;	}

		// ---|> Component
		GUIAPI virtual void doLayout() override;

		void setEntryHeight(float h) {
			entryHeight=h;
			resetPositions(0);
		}

		size_t getNumVisibleEntries()const					{	return static_cast<size_t>(getHeight()/entryHeight+1);	}
	private:
		void assertIsChild(Component * c)const;
		class ListViewClientArea : public Container{
				ListView & myListView;
			public:

				ListViewClientArea(GUI_Manager & _gui,ListView &_l):
						Container(_gui),myListView(_l){}
				ListViewClientArea(const ListViewClientArea & other,ListView &_l):
						Container(other),myListView(_l){}
				virtual ~ListViewClientArea() {}

				// ---|> Component
				GUIAPI virtual void doLayout() override;

			private:
				// ---|> Component
				GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

		};

		// ---|> Component
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

		float entryHeight;
		Util::WeakPointer<ListViewClientArea> clientArea;
	//	@}

	// ------------------

	//! @name Children
	//	@{
	public:
		// ---|> Container
		GUIAPI virtual void addContent(const Ref & child) override;
		// ---|> Container
		GUIAPI virtual void clearContents() override;
		// ---|> Container
		virtual std::vector<Component*> getContents() override 				{	return clientArea->getContents();		}

		//! Return entry with given index or nullptr if there is no such entry.
		Component * getEntry(size_t i)const							{	return i<entryRegistry.size() ? entryRegistry[i] : nullptr;}

		// ---|> Container
		GUIAPI virtual void insertAfter(const Ref & child,const Ref & after) override;
		// ---|> Container
		GUIAPI virtual void insertBefore(const Ref & child,const Ref & after) override;
		// ---|> Container
		virtual size_t getContentsCount()const override						{   return clientArea->getContentsCount();	}
		// ---|> Container
		GUIAPI virtual void removeContent(const Ref & child) override;


	private:
		GUIAPI void rebuildRegistry();
		GUIAPI void resetPositions(size_t beginningIndex);

		static const size_t npos = static_cast<size_t>(-1);

		//! returns npos if no element is at the given position
		GUIAPI size_t getEntryIndexByPosition(const Geometry::Vec2 & p)const;
		// a collection of all components, mapping the index(=the row) to the component
		std::vector<Component *> entryRegistry; //
	//	@}

	// ------------------

	//! @name Cursor
	//	@{
	public:
		Component * getCursorEntry()const					{	return getEntry(cursor);	}
		size_t getCursorIndex()const						{	return cursor;	}
		GUIAPI void moveCursor(int delta);
		GUIAPI void scrollToCursor();
		void setCursorIndex(size_t i)						{	cursor = i;	}
	private:
		size_t cursor;
	//	@}

	// ------------------

	//! @name Event handling
	//	@{
	private:
		KeyListener keyListener;
		MouseButtonListener mouseButtonListener;
		OptionalMouseMotionListener optionalMouseMotionListener;

		GUIAPI bool onKeyEvent(const Util::UI::KeyboardEvent & keyEvent);
		GUIAPI bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
		GUIAPI bool onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent);
	//	@}

	// ------------------

	//! @name Marking
	//	@{
	public:
		typedef std::set<Component *> markingSet_t;
		typedef std::list<Component *> markingList_t;

		//! Add a marking to the given component and call marking changed.
		GUIAPI void addMarking(Component * c);
		GUIAPI void clearMarkings(bool forced=false);
		const markingList_t & getMarkings()const			{	return markingList;	};
		bool isMarked(Component * c)const					{	return markingSet.count(c)>0;	}
		// ---o
		GUIAPI virtual void markingChanged();
		GUIAPI void removeMarking(Component * c,bool forced=false);
		GUIAPI void setMarking(Component * c);
		GUIAPI void setMarkings(const markingList_t & markings);

	private:
		GUIAPI void doAddMarking(Component * c);
		GUIAPI bool doClearMarking(bool forced);
		GUIAPI bool doRemoveMarking(Component * c,bool forced);
		GUIAPI void performMarkingAction(const size_t index,const bool accumulative,const bool grouping);

		markingSet_t markingSet;
		markingList_t markingList;
		size_t initialMarkingIndex;
	//	@}

	// ------------------

	//! @name Scrolling
	//	@{
	public:
		GUIAPI void scrollTo(const Geometry::Vec2 & pos,float duration);
		GUIAPI void setScrollingPosition(const Geometry::Vec2 & pos);
		const Geometry::Vec2 & getScrollPos()const			{	return scrollPos;	}
	private:
		GUIAPI void finishScrolling();
		Util::WeakPointer<Scrollbar> scrollBar;
		std::unique_ptr<DataChangeListenerHandle> optionalScrollBarListener;
		Geometry::Vec2 scrollPos;
		Geometry::Vec2 maxScrollPos;
	//	@}
};
}
#endif // GUI_ListView_H
