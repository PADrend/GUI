/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef CONTAINER_H
#define CONTAINER_H

#include "Component.h"
#include <list>

namespace GUI {
/***
 **     Container ---|> Component
 **       0..1 ------------> *
 **/
class Container : public Component {
		PROVIDES_TYPE_NAME(Container)
	public:
		GUIAPI Container(GUI_Manager & gui,flag_t flags=0);
		GUIAPI Container(GUI_Manager & gui,const Geometry::Rect & r,flag_t flags=0);

		typedef Util::Reference<Container> ContainerRef;

	protected:
		GUIAPI virtual ~Container();

	public:
		void _addChild(const Ref & child)	{	_insertAfter(child,getLastChild());	}
		GUIAPI void _insertAfter(const Ref & child,const Ref & after);
		GUIAPI void _insertBefore(const Ref & child,const Ref & before);
		GUIAPI void _removeChild(const Ref & child);

		Component * getFirstChild()const	{	return firstChild.get();	}
		Component * getLastChild()const 	{	return lastChild.get();	}

		/*! This is called by a child @p c whenever its rect is changed, it's added or it's removed.
			The LAYOUT_VALID flag is cleared.	*/
		GUIAPI void childRectChanged(Component * c);

		// ---o
		virtual void addContent(const Ref & child) 		{	_addChild(child);	}
		// ---o
		virtual void removeContent(const Ref & child)	{	_removeChild(child);	}
		// ---o
		GUIAPI virtual void clearContents();

		/*! Remove all children and mark them for removal .
			The childrens' subtrees will be dissolved and their attributes will be removed.
			\note this does (or should not) not remove the internal children like scroll bars.*/
		GUIAPI void destroyContents();

		// ---o
		virtual size_t getContentsCount()const 			{	return contentsCount;	}
		// ---o
		GUIAPI virtual void bringChildToFront(Component * c);
		// ---o
		GUIAPI virtual std::vector<Component*> getContents();
		// ---o
		virtual void insertAfter(const Ref & child,const Ref & after){
			_insertAfter(child,after);
		}
		// ---o
		virtual void insertBefore(const Ref & child,const Ref & after){
			_insertBefore(child,after);
		}

		// ---|> Component
		GUIAPI virtual std::string toString()const override;
		GUIAPI virtual visitorResult_t traverseChildren(Visitor & v) override;
		GUIAPI virtual visitorResult_t traverseSubtree(Visitor & v) override;

	private:
		GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

	protected:
		GUIAPI void displayChildren(const Geometry::Rect & region,bool useScissor=false);
		GUIAPI void copyChildrenTo(Container & target)const;

		Ref firstChild;
		Ref lastChild;
		size_t contentsCount;
};
}
#endif // CONTAINER_H
