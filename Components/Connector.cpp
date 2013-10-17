/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "Connector.h"
#include "../GUI_Manager.h"
#include "ComponentPropertyIds.h"

namespace GUI {

//! (static)
void Connector::findConnectors(Container * container,const Component * endpoint,std::list<Connector*> & connectors){
	if(!container)
		return;

	struct MyVisitor : public Component::Visitor {
		const Component * endpoint;
		std::list<Connector*> & connectors;

		MyVisitor(const Component * _endpoint,std::list<Connector*> & _connectors):
			endpoint(_endpoint),connectors(_connectors){}

		virtual ~MyVisitor(){}

		// ---|>  Component::Visitor
		visitorResult_t visit(Component & c) override {
			Connector * connector=dynamic_cast<Connector*>(&c);
			if(connector && ( connector->getFirstComponent()==endpoint || connector->getSecondComponent()==endpoint ))
				connectors.push_back( connector );
			return Component::CONTINUE_TRAVERSAL;
		}
	} visitor(endpoint,connectors);

	container->traverseSubtree(visitor);
}


// ----------------

//! (ctor)
Connector::Connector(GUI_Manager & _gui,flag_t _flags/*=0*/)
		:Container(_gui,_flags){
	init();
	//ctor
}

//! (dtor)
Connector::~Connector(){
	firstComponent=nullptr;
	secondComponent=nullptr;
	//dtor
}

void Connector::init(){
	if(getContentsCount()==0){
		addConnectorPoint();
		addConnectorPoint();
	}
}

//! ---|> Component
void Connector::doDisplay(const Geometry::Rect & region){
	displayChildren(region);
	
	std::vector<Geometry::Vec2> points;
	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext())
		points.emplace_back(c->getPosition());
	getGUI().displayLineShape(PROPERTY_CONNECTOR_LINE_SHAPE,points,0);

}


//! ---|> Component
void Connector::addConnectorPoint(){
	auto p=new ConnectorPoint(getGUI());
	_addChild(p);
}

//! ---|> Component
bool Connector::coversLocalPos(const Geometry::Vec2 & pos){
	if(!getLocalRect().changeSizeCentered(10,10).contains(pos))
		return false;
	Geometry::Vec2 p1;
	Geometry::Vec2 p2;

	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext()){
		p1=p2;
		p2=c->getPosition();
		if(c==getFirstChild())
			continue;
		float length=p1.distance(p2);
		if(length==0)
			continue;
		float u= (	(pos.x() - p1.x()) * (p2.x()-p1.x()) +
					(pos.y() - p1.y()) * (p2.y()-p1.y())) / (length*length);
		if(u<0.0f || u >1.0f ){
			continue;
		}
		float dist=pos.distance(p1 + (p2-p1)*u);

		if(dist<=5){
			return true;
		}
	}
	return false;
}

//! ---|> Component
void Connector::doLayout(){

	Geometry::Vec2 parentsAbsPos=hasParent()?getParent()->getAbsPosition():Geometry::Vec2();

	Geometry::Rect r;
	r.invalidate();
//	std::cout << r.getX();

	if( getFirstComponent() )
		r.include( getFirstComponent()->getAbsRect().getCenter()-parentsAbsPos );
//	std::cout << (getFirstComponent()->getAbsRect().getCenter()-parentsAbsPos).getX();

	if( getSecondComponent() )
		r.include( getSecondComponent()->getAbsRect().getCenter()-parentsAbsPos );

	if(getContentsCount()>2){
		// TODO!!!!!
	}
	setPosition(Geometry::Vec2(r.getX(),r.getY()));
	setSize(r.getWidth(),r.getHeight());
	Geometry::Vec2 myAbsPos=getAbsPosition();

	if(getFirstChild() && getFirstComponent()){
		getFirstChild()->setPosition( getFirstComponent()->getAbsRect().getCenter()-myAbsPos );
	}
	if(getLastChild() && getSecondComponent()){
		getLastChild()->setPosition( getSecondComponent()->getAbsRect().getCenter()-myAbsPos );
//		getLastChild()->setPosition(Vec2(r.getWidth(),r.getHeight()));
	}

	if(getContentsCount()>2){
		// TODO!!!!!
	}
}

float Connector::getLength()const{
	float l=0.0f;
	Geometry::Vec2 p1;
	Geometry::Vec2 p2;

	for(Component * c=getFirstChild();c!=nullptr;c=c->getNext()){
		p1=p2;
		p2=c->getPosition();
		if(c==getFirstChild())
			continue;
		l+=p1.distance(p2);
	}
	return l;
}
// ----------------------------------

Connector::ConnectorPoint::ConnectorPoint(GUI_Manager & _gui,flag_t _flags):Container(_gui,_flags){
}

Connector::ConnectorPoint::ConnectorPoint(const ConnectorPoint & c):Container(c){
}

Connector::ConnectorPoint::~ConnectorPoint(){
}

}
