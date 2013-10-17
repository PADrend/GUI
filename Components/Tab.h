/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef GUI_TAB_H
#define GUI_TAB_H

#include "Container.h"
#include "Label.h"

namespace GUI {
/***
 **     TabbedPanel ---|> Container ---|> Component
 **                   0..1 ------------> *
 **/
class TabbedPanel : public Container   {
		PROVIDES_TYPE_NAME(TabbedPanel)
	public:

		/***
		 **     Tab ---|> Container ---|> Component
		 **                   0..1 ------------> *
		 **/
		class Tab : public Container {
				PROVIDES_TYPE_NAME(Tab)
			public:
				Tab(GUI_Manager & gui,const std::string & title="",Container * clientArea=nullptr);
				Tab(GUI_Manager & gui,const Geometry::Rect & r,const std::string & title="",Container * clientArea=nullptr);
				virtual ~Tab();

				Container * clientArea()const 				{	return clientAreaPanel;	}
				void setTitle(const std::string & title);
				std::string getTitle()const;
				TabbedPanel * getTabbedPanel()const;
				bool isActiveTab()const;
				void makeActiveTab();

				float getTabTitlePos() 						{	return titlePanel->getPosition().x();	}
				float getTabTitleWidth() 					{	return titlePanel->getWidth();	}
				void setTabTitlePos(float tabTitlePos) 		{	titlePanel->setPosition(Geometry::Vec2(tabTitlePos,titlePanel->getPosition().y()));	}

				Container *getTitlePanel() 					{	return titlePanel;	}

				// ---|> Container
				virtual void addContent(const Ref & child)		{	clientAreaPanel->addContent(child);	}
				virtual void removeContent(const Ref & child)	{	clientAreaPanel->removeContent(child);	}
				virtual size_t getContentsCount()const			{	return clientAreaPanel->getContentsCount();	}

				// ---|> Component
				virtual Geometry::Rect getInnerRect()const 		{	return clientAreaPanel->getLocalRect();	}
				virtual void doLayout();

				virtual bool hasTooltip()const                  {   return titlePanel->hasTooltip();  }
				virtual std::string getTooltip()const           {   return titlePanel->getTooltip();  }
				virtual void setTooltip(const std::string & s)  {   titlePanel->setTooltip(s); }
				virtual void removeTooltip()                    {   titlePanel->removeTooltip();   }
			private:
				// ---|> Component
				virtual void doDisplay(const Geometry::Rect & region);

			protected:
				void init();

				Container * clientAreaPanel;
				Container * titlePanel;
				Label * titleTextLabel;
		};
		// ---------------------------------------------------
	public:
		TabbedPanel(GUI_Manager & gui,flag_t flags=0);
		virtual ~TabbedPanel(){}

		Tab * createTab(const std::string  & title,Container * clientArea=nullptr);

		void setActiveTab(Tab * tab);
		Tab * getActiveTab()const			{	return activeTab;	}

		void setActiveTabIndex(int nr);
		int getActiveTabIndex()const;

		void recalculateTabTitlePositions();

		// ---|> Container
		virtual void addContent(const Ref & child);
		virtual void removeContent(const Ref & child);
		virtual void bringChildToFront(Component * c);

		// ---|> Component
		virtual void doLayout();
	private:
		Tab * activeTab;
};
}
#endif // GUI_TAB_H
