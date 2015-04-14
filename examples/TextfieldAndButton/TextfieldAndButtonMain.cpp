/*
	This file is part of the GUI library.
	Copyright (C) 2013 Benjamin Eikel <benjamin@eikel.org>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <GUI/Components/Button.h>
#include <GUI/Components/Textfield.h>
#include <GUI/Components/Window.h>
#include <GUI/GUI_Manager.h>
#include <Util/UI/EventContext.h>
#include <Util/UI/UI.h>
#include <Util/References.h>
#include <Util/StringIdentifier.h>
#include <Util/Util.h>
#include <cstdlib>
#include <iostream>

/**
 * @file
 * @brief Simple usage example for GUI
 * 
 * The example uses a GUI::GUI_Manager to create a GUI::Window, a
 * GUI::Textfield, and a GUI::Button. Then, it registers a GUI::ActionListener
 * to wait for a click onto the button. When the button is clicked, the text
 * field is cleared.
 */

int main(int /*argc*/, char */*argv*/[]) {
	Util::init();

	Util::UI::Window::Properties properties;
	properties.positioned = true;
	properties.posX = 100;
	properties.posY = 100;
	properties.clientAreaWidth = 1024;
	properties.clientAreaHeight = 768;
	properties.title = "GUI Textfield and Buttons";
	properties.compatibilityProfile = true;
	auto window = Util::UI::createWindow(properties);

	Util::UI::EventContext eventContext;
	eventContext.getEventQueue().registerEventGenerator(std::bind(&Util::UI::Window::fetchEvents, window.get()));
	
	GUI::GUI_Manager guiManager(&eventContext);
	guiManager.setWindow(window.get());

	Util::Reference<GUI::Window> guiWin = guiManager.createWindow(Geometry::Rect_f(10, 10, 200, 200), "Window");

	Util::Reference<GUI::Textfield> guiText = guiManager.createTextfield("Text");
	guiText->setRect(Geometry::Rect_f(0, 0, 40, 20));
	guiWin->addContent(guiText.get());

	Util::Reference<GUI::Button> guiButton = guiManager.createButton("Clear");
	guiButton->setActionListener(	[&guiText](GUI::Component *, const Util::StringIdentifier &) {
										guiText->setText("");
										return true;
									});
	guiButton->setRect(Geometry::Rect_f(0, 25, 40, 20));
	guiWin->addContent(guiButton.get());

	bool done = false;
	while(!done) {
		eventContext.getEventQueue().process();
		while(eventContext.getEventQueue().getNumEventsAvailable() > 0) {
			auto event = eventContext.getEventQueue().popEvent();
			if(event.type == Util::UI::EVENT_QUIT ||
						(event.type == Util::UI::EVENT_KEYBOARD &&
						 event.keyboard.pressed &&
						 event.keyboard.key == Util::UI::KEY_ESCAPE)) {
				done = true;
			} else {
				guiManager.handleEvent(event);
			}
		}
		guiManager.display();
		window->swapBuffers();
	}
	return EXIT_SUCCESS;
}
