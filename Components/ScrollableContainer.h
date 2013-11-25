/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_ScrollableContainer_H
#define GUI_ScrollableContainer_H

#include "Container.h"
#include "LayoutHelper.h"
#include "../Base/Listener.h"
#include "../Base/Layouters/FlowLayouter.h"
#include "../GUI_Manager.h"
#include <iostream>
#include <memory>

namespace GUI {
class Scrollbar;
/***
 **     ScrollableContainer ---|> Container ---|> Component
 **/
class ScrollableContainer: public Container {
		PROVIDES_TYPE_NAME(ScrollableContainer)
	public:

		ScrollableContainer(GUI_Manager & gui,flag_t flags=0);
		virtual ~ScrollableContainer();

		void scrollTo(const Geometry::Vec2 & pos);
		void scrollTo(const Geometry::Vec2 & pos,float duration);
		const Geometry::Vec2 & getScrollPos()const						{	return scrollPos;	}

		Container * getContentContainer()const							{	return contentContainer.get();	}

		// ---|> Container
		virtual void addContent(const Ref & child) override						{	contentContainer->addContent(child);	}
		virtual void clearContents() override									{	contentContainer->clearContents();	}
		virtual std::vector<Component*> getContents() override 					{	return contentContainer->getContents();		}
		virtual void removeContent(const Ref & child) override					{	contentContainer->removeContent(child);	}
		virtual void insertAfter(const Ref & child,const Ref & after) override	{	contentContainer->insertAfter(child,after);	}
		virtual void insertBefore(const Ref & child,const Ref & after) override	{	contentContainer->insertBefore(child,after);	}
		virtual size_t getContentsCount()const override							{   return contentContainer->getContentsCount();	}

		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;
		virtual void doLayout() override;
		
	private:
		Util::WeakPointer<Container> contentContainer;
		Util::WeakPointer<Scrollbar> vScrollBar;
		std::unique_ptr<GUI_Manager::DataChangeListenerHandle> optionalScrollBarListener;
		Geometry::Vec2 scrollPos;
		Geometry::Vec2 maxScrollPos;
		GUI_Manager::MouseButtonListenerHandle mouseButtonListenerHandle;
		GUI_Manager::MouseMotionListenerHandle mouseMotionListenerHandle;
		bool listenOnMouseMove;

		bool onMouseButton(Component * component, const Util::UI::ButtonEvent & buttonEvent);
		bool onMouseMove(Component * component, const Util::UI::MotionEvent & motionEvent);
};
}
#endif // GUI_ScrollableContainer_H
