#
# This file is part of the GUI library.
# Copyright (C) 2013 Benjamin Eikel <benjamin@eikel.org>
#
# This library is subject to the terms of the Mozilla Public License, v. 2.0.
# You should have received a copy of the MPL along with this library; see the 
# file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
#
option(GUI_BUILD_EXAMPLES "Defines if examples for the GUI library are built.")
if(GUI_BUILD_EXAMPLES)
	add_subdirectory(TextfieldAndButton)
endif()
