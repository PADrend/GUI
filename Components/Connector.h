/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_Connector_H
#define GUI_Connector_H

#include "Container.h"
#include "../Base/ListenerHelper.h"

namespace GUI {
/***
 **     Connector ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class Connector: public Container{
		PROVIDES_TYPE_NAME(Connector)
	public:
		/*! Find all Connectors in container which are connected the Component enpoint on one side. */
		static void findConnectors(Container * container,const Component * endpoint,
									std::list<Connector*> & connectors);

		Connector(GUI_Manager & gui,flag_t flags=0);
		virtual ~Connector();

		// ---|> Component
		virtual void doLayout() override;
		virtual bool coversLocalPos(const Geometry::Vec2 & pos) override;
	private:
		// ---|> Component
		virtual void doDisplay(const Geometry::Rect & region) override;


	public:

		float getLength()const;
		void addConnectorPoint();

		Component * getFirstComponent()const	{	return firstComponent.get();	}
		Component * getSecondComponent()const	{	return secondComponent.get();	}
		void setFirstComponent(Component * c)	{	firstComponent=c;	}
		void setSecondComponent(Component * c)	{	secondComponent=c;	}

		/*! (internal) One pont on a path of a component-Connector. */
		class ConnectorPoint: public Container{
				PROVIDES_TYPE_NAME(ConnectorPoint)
			public:
				ConnectorPoint(GUI_Manager & gui,flag_t flags=0);
				ConnectorPoint(const ConnectorPoint & c);
				virtual ~ConnectorPoint();

//				// ---|> Component
//				virtual Component * clone()const;

		};

	protected:
		Util::Reference<Component> firstComponent;
		Util::Reference<Component> secondComponent;

};
}
#endif // GUI_Connector_H
