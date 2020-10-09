/*
	This file is part of the GUI library.
	Copyright (C) 2008-2013 Benjamin Eikel <benjamin@eikel.org>
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
class TabbedPanel : public Container {
		PROVIDES_TYPE_NAME(TabbedPanel)
	public:

		/***
		 **     Tab ---|> Container ---|> Component
		 **                   0..1 ------------> *
		 **/
		class Tab : public Container {
				PROVIDES_TYPE_NAME(Tab)
			public:
				GUIAPI static const Util::StringIdentifier ACTION_Tab_close;
				GUIAPI static const Util::StringIdentifier ACTION_Tab_open;

				GUIAPI Tab(GUI_Manager & gui,const std::string & title="",Container * clientArea=nullptr);
				GUIAPI virtual ~Tab();

				Container * clientArea()const 				{	return clientAreaPanel;	}
				GUIAPI void setTitle(const std::string & title);
				GUIAPI std::string getTitle()const;
				GUIAPI TabbedPanel * getTabbedPanel()const;
				GUIAPI bool isActiveTab()const;
				GUIAPI void makeActiveTab();

				float getTabTitlePos() 						{	return titlePanel->getPosition().x();	}
				float getTabTitleWidth() 					{	return titlePanel->getWidth();	}
				void setTabTitlePos(float tabTitlePos) 		{	titlePanel->setPosition(Geometry::Vec2(tabTitlePos,titlePanel->getPosition().y()));	}

				Container *getTitlePanel() 					{	return titlePanel;	}

				// ---|> Container
				virtual void addContent(const Ref & child) override		{	clientAreaPanel->addContent(child);	}
				virtual void removeContent(const Ref & child) override	{	clientAreaPanel->removeContent(child);	}
				virtual size_t getContentsCount()const override			{	return clientAreaPanel->getContentsCount();	}

				// ---|> Component
				virtual Geometry::Rect getInnerRect()const override 		{	return clientAreaPanel->getLocalRect();	}
				GUIAPI virtual void doLayout() override;

				virtual bool hasTooltip()const override                  {   return titlePanel->hasTooltip();  }
				virtual std::string getTooltip()const override           {   return titlePanel->getTooltip();  }
				virtual void setTooltip(const std::string & s) override  {   titlePanel->setTooltip(s); }
				virtual void removeTooltip() override                    {   titlePanel->removeTooltip();   }
			private:
				// ---|> Component
				GUIAPI virtual void doDisplay(const Geometry::Rect & region) override;

		protected:
				Container * clientAreaPanel;
				Container * titlePanel;
				Label * titleTextLabel;
		};
		// ---------------------------------------------------
	public:
		GUIAPI TabbedPanel(GUI_Manager & gui,flag_t flags=0);
		GUIAPI virtual ~TabbedPanel(){}

		GUIAPI Tab * createTab(const std::string & title,Container * clientArea=nullptr);

		GUIAPI void setActiveTab(Tab * tab);
		Tab * getActiveTab()const			{	return activeTab;	}

		GUIAPI void setActiveTabIndex(int nr);
		GUIAPI int getActiveTabIndex()const;

		GUIAPI void recalculateTabTitlePositions();

		// ---|> Container
		GUIAPI virtual void addContent(const Ref & child) override;
		GUIAPI virtual void removeContent(const Ref & child) override;
		GUIAPI virtual void bringChildToFront(Component * c) override;

		// ---|> Component
		GUIAPI virtual void doLayout() override;
	private:
		Tab * activeTab;
};
}
#endif // GUI_TAB_H
