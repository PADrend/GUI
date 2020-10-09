/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef COMPONENT_H
#define COMPONENT_H

#include "../Base/Layouters/AbstractLayouter.h"
#include "../Base/AbstractProperty.h"

#include <Geometry/Rect.h>
#include <Geometry/Vec2.h>

#include <Util/Macros.h>
#include <Util/Graphics/Color.h>
#include <Util/ReferenceCounter.h>
#include <Util/TypeNameMacro.h>
#include <Util/GenericAttribute.h>
#include <Util/AttributeProvider.h>

#include <string>

// Geometry

namespace GUI {

class Container;
class GUI_Manager;

/***
 **  Component
 **/
class Component: public Util::AttributeProvider, public Util::ReferenceCounter<Component> {
		PROVIDES_TYPE_NAME(Component)

	/*!	@name Main	*/
	// @{
	private:
		GUI_Manager & gui;
	public:
		typedef uint32_t flag_t;
		typedef Util::Reference<Component> Ref;

		GUIAPI Component(GUI_Manager & gui,flag_t flags=0);
		GUIAPI Component(GUI_Manager & gui,const Geometry::Rect & relRect,flag_t flags=0);
		GUIAPI virtual ~Component();

		GUI_Manager & getGUI() const {
			return gui;
		}

		GUIAPI static void destroy(Component * c);

		// ---o
		GUIAPI virtual std::string toString()const;
	// @}

	// -----------------------------------

	/*!	@name Tree management	*/
	// @{
	private:
		Util::WeakPointer<Container> parent;
		Ref prev;
		Ref next;
	protected:
		void setNext(const Ref & newNext)	{	next=newNext;	}
		void setPrev(const Ref & newPrev)	{	prev=newPrev;	}

	public:
		void _setParent(const Util::WeakPointer<Container> & c) 	{	parent = c;	invalidateLayout(); };
		GUIAPI void _updateNeighbors(const Ref & newPrev,const Ref & newNext);

		GUIAPI void bringToFront();

		Container * getParent()const		{	return parent.get();	}
		Component * getNext()const			{	return next.get();	}
		Component * getPrev()const			{	return prev.get();	}
		bool hasParent()const				{	return !parent.isNull();	}
	// @}

	// -----------------------------------

	/*!	@name Flags	*/
	// @{
	private:
		flag_t flags;

	public:
		// note: bits>=24 are reserved for special component flags
		static const flag_t DISABLED=1<<0;
		static const flag_t TRANSPARENT_COMPONENT=1<<1; //!< component is not found by getComponentAtPos(...); its not clickable!
		static const flag_t AUTO_MAXIMIZE=1<<2;
		static const flag_t AUTO_MINIMIZE=1<<3; //!< Experimental: The component should resize to enclose all children.
		static const flag_t BORDER=1<<4;
		static const flag_t RAISED_BORDER=1<<4 | 1<<5;
		static const flag_t LOWERED_BORDER=1<<4 | 1<<6;
		static const flag_t BACKGROUND=1<<7;
		static const flag_t USE_SCISSOR=1<<8;
		static const flag_t SELECTABLE=1<<9;
		static const flag_t IS_CLIENT_AREA=1<<11; //!< Used to mark internal components so that the external layout can use the parent of the component as reference
		static const flag_t ALWAYS_ON_TOP=1<<12; //!< Used to mark (top-level) components which should never be behind non ALWAYS_ON_TOP components
		static const flag_t LOCKED=1<<13; //!< Input components are read only.
		static const flag_t HAS_MOUSECURSOR_PROPERTY=1<<14;
		// status
		static const flag_t DESTROYED=1<<19;
		static const flag_t ABS_POSITION_VALID=1<<20;
		static const flag_t LAYOUT_VALID=1<<21;
		static const flag_t SUBTREE_LAYOUT_VALID=1<<22;
		static const flag_t SELECTED=1<<23;

		GUIAPI void activate();
		GUIAPI void deactivate();
		GUIAPI void disable();
		GUIAPI void enable();
		bool getFlag(flag_t f)const			{	return (flags&f)>0;	}
		GUIAPI bool isActive()const;
		bool isDestroyed()const				{	return getFlag(DESTROYED);	}
		bool isEnabled()const				{	return !getFlag(DISABLED);	}
		bool isLocked()const				{	return getFlag(LOCKED);	}
		bool isSelected()const				{	return getFlag(SELECTED);	}
		bool isSelectable()const			{	return getFlag(SELECTABLE) && isEnabled();	}
		GUIAPI bool isVisible()const;
		void setEnabled(bool e)				{	e ? enable() : disable();	}
		void setFlag(flag_t f,bool value)	{	flags = value ? (flags|f) : flags^(flags&f); }
		void setLocked(bool b)				{	setFlag(LOCKED,b);	}
		
		// selection
		void unselect()						{	if(isSelected() && onUnselect())setFlag(SELECTED,false);	}
		GUIAPI void select();
		GUIAPI void unselectSubtree();
	 // @}

	// -----------------------------------

	/*!	@name Layout	*/
	// @{
	private:
		std::vector<Util::Reference<AbstractLayouter>> layouters;
	public:
		void addLayouter(Util::Reference<AbstractLayouter> layouter)	{	layouters.push_back(layouter);	}
		void clearLayouters()											{	layouters.clear();	}

		virtual void doLayout()											{	}
		std::vector<Util::Reference<AbstractLayouter>> getLayouters()const	{	return layouters;	}
		
		template<class Layouter_t> 
		Layouter_t * getLayouter()const{
			for(auto & l : layouters){
				Layouter_t * l2 = dynamic_cast<Layouter_t*>(l.get());
				if(l2)
					return l2;
			}
			return nullptr;
		}
		

		bool hasLayouter()const											{	return !layouters.empty();	}

		GUIAPI void invalidateLayout();
		GUIAPI void invalidateSubtreeLayout();

		/*! The size of the component is set correctly (if necessary) and all children are layouted recursivly. */
		GUIAPI uint32_t layout();
		GUIAPI uint32_t layoutChildren();
		
		GUIAPI void removeLayouter(Util::WeakPointer<AbstractLayouter> layouter);
		
		template<class Layouter_t> 
		bool removeLayouter(){
			for(auto it=layouters.begin();it!=layouters.end();++it){
				if(dynamic_cast<Layouter_t*>(it->get())){
					layouters.erase(it);
					return true;
				}
			}
			return false;
		}
	// @}

	// -----------------------------------

	/*!	@name Display properties	*/
	// @{
	public:
		typedef std::vector<Util::Reference<DisplayProperty> > properties_t;
		void addProperty(DisplayProperty * p)									{	recursiveDisplayProperties.push_back(p);	}
		GUIAPI void removeProperty(DisplayProperty * p);
		void clearProperties()													{	recursiveDisplayProperties.clear(); }
		const properties_t & getProperties()const								{	return recursiveDisplayProperties;	}
		void addLocalProperty(DisplayProperty * p)								{	localDisplayProperties.push_back(p);	}
		GUIAPI void removeLocalProperty(DisplayProperty * p);
		void clearLocalProperties()												{	localDisplayProperties.clear(); }
		const properties_t & getLocalProperties()const							{	return localDisplayProperties;	}

	private:
		properties_t recursiveDisplayProperties;
		properties_t localDisplayProperties;
	// @}

	// -----------------------------------

	/*!	@name Display	*/
	// @{
	public:
		GUIAPI void display(const Geometry::Rect & region);

	private:
		// ---o
		GUIAPI virtual void doDisplay(const Geometry::Rect & region);
		
	protected:
		GUIAPI void enableLocalDisplayProperties();
		GUIAPI void disableLocalDisplayProperties();
		GUIAPI void displayDefaultShapes();
		
	 // @}

	// -----------------------------------

	/*!	@name Position and bounds*/
	// @{
	private:
		Geometry::Vec2 absPosition;
		Geometry::Rect relRect;
		bool isAbsPosValid()const							{	return getFlag(ABS_POSITION_VALID);	}

	public:
		bool coversAbsPosition(const Geometry::Vec2 & p)	{	return coversLocalPosition(p-getAbsPosition());	}
		// ---o
		GUIAPI virtual bool coversLocalPosition(const Geometry::Vec2 & localPos);

		GUIAPI Geometry::Vec2 getAbsPosition();
		Geometry::Rect getAbsRect()							{	return Geometry::Rect(getAbsPosition(),relRect.getSize());	}

		/*! ---o 
			The component's inner rectangle defines the area, that may be covered by children (=content) 
			(not regarding scrolling). It can be used as a hint for the maximum size of children.
		*/
		virtual Geometry::Rect getInnerRect()const 			{	return getLocalRect();	}
		
		float getHeight()const								{	return relRect.getHeight();	}
		Geometry::Rect getLocalRect()const					{	return Geometry::Rect(0,0,getWidth(),getHeight());	}
		Geometry::Rect getRect()const						{	return relRect;	}
		Geometry::Vec2 getPosition()const					{	return relRect.getPosition();	}
		float getWidth()const								{	return relRect.getWidth();	}

		GUIAPI void invalidateAbsPosition();
		// ---o
		GUIAPI virtual void invalidateRegion();

		void moveRel(const Geometry::Vec2 & v)				{	setPosition(getPosition()+v);	}

		void setPosition(const Geometry::Vec2 & newPos)		{	setRect( newPos,relRect.getSize() ); }
		GUIAPI void setRect(const Geometry::Rect & newRect);
		void setRect(const Geometry::Vec2 & pos,const Geometry::Vec2 & size)	{	setRect(Geometry::Rect(pos,size));	}

		// boundingRectangle
		void setSize(float width,float height)				{	setRect(relRect.getPosition(),Geometry::Vec2(width,height));	}
		void setSize(const Geometry::Vec2 & s)				{	setRect(relRect.getPosition(),s);	}
		void setHeight(float f)								{	setRect(relRect.getPosition(),Geometry::Vec2(getWidth(),f));	}
		void setWidth(float f)								{	setRect(relRect.getPosition(),Geometry::Vec2(f,getHeight()));	}

	// @}

	// -----------------------------------

		/*! Enable automatic layouting. */
		GUIAPI void setExtLayout(uint32_t flags, const Geometry::Vec2 & extPos, const Geometry::Vec2 & extRelSize);
		GUIAPI void setExtLayout(uint32_t flags, const Geometry::Vec2 & extPos );
		/*! Disable automatic layouting. */
		GUIAPI void removeExternalLayout();
	// @}
	
	// -----------------------------

	/*!	@name Helper	*/
	// @{
	public:
		GUIAPI Component * getComponentAtPos(const Geometry::Vec2 & pos);
		GUIAPI Component * findSelectedComponent();
	// @}

	// -----------------------------------

	/*!	@name Events Handler	*/
	// @{
	public:
		// ---o
		GUIAPI virtual bool onSelect();
		GUIAPI virtual bool onUnselect();
	 // @}

	// -----------------------------------

	/*!	@name Traversal	*/
	// @{
	public:
		enum visitorResult_t { CONTINUE_TRAVERSAL,BREAK_TRAVERSAL,EXIT_TRAVERSAL};

		struct Visitor {
			Visitor() {}
			virtual ~Visitor() {}
			
			virtual visitorResult_t visit(Component & c)=0;
		};
		// ---o
		virtual visitorResult_t traverseChildren(Visitor & )	{	return CONTINUE_TRAVERSAL;	}
		virtual visitorResult_t traverseSubtree(Visitor & v)	{	return v.visit(*this);	}
	 // @}

	// -----------------------------------
	
	/*!	@name MouseCursor	*/
	// @{
	
public:
	bool hasMouseCursorProperty() const						{	return getFlag(HAS_MOUSECURSOR_PROPERTY); }
	GUIAPI void setMouseCursorProperty(propertyName_t type);
	GUIAPI propertyName_t getMouseCursorProperty();
	
	// @}
	
	// -----------------------------------

	/*!	@name Tooltip */ 
	// @{
	public:
		GUIAPI virtual bool hasTooltip()const;
		GUIAPI virtual std::string getTooltip()const;
		GUIAPI virtual void setTooltip(const std::string & s);
		GUIAPI virtual void removeTooltip();
	 // @}

};



}

#endif // COMPONENT_H
