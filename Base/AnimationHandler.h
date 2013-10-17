/*
	This file is part of the GUI library.
	Copyright (C) 2008-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2008-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2008-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef ANIMATIONHANDLER_H
#define ANIMATIONHANDLER_H

#include <Util/References.h>

namespace GUI{
class Component;

/*! An AnimationHandler is responsible for one animation of a component.
	To start an animation, an appropriate AnimationHandler has to be created
	and registered at the GUI_Manager via addAnimationHandler.
	The GUI_Manager deletes the AnimationHandler Object when the animation has
	finished. */
class AnimationHandler{
	public:

		/** Default constructor */
		AnimationHandler(Component * c,float _duration):
					myComponent(c),lastTime(0),myStartTime(0),
					duration(_duration){}

		/** Default destructor */
		virtual ~AnimationHandler()	{}

		void updateLastTime(float t){	lastTime=t;	}
		float getLastTime()const	{	return lastTime;	}

		void setDuration(float t)	{	duration=t;	}
		float getDuration()const	{	return duration;	}

		void setStartTime(float t)	{	myStartTime=t;	}
		float getStartTime()const	{	return myStartTime;	}

		float getEndTime() const {
			return myStartTime + duration;
		}

		Component * getComponent()const	{	return myComponent.get();	}

		// ---o
		/*! Called by the GUI_Manager in every frame with the current
			time in seconds.
			If the animation has finished, finish() should be called and
			false should be returned (true otherwise). Then this object
			is removed by the GUI_Manager.	*/
		virtual bool animate(float currentTime){
			if(currentTime > getEndTime()) {
				finish();
				return false;
			}
			return true;
		}

		// ---o
		/*! Sets the component to its final stage of the animation.
			Called when the animation finished by animate, or by the
			GUI_Manager if the animation should be finished prematurely.*/
		virtual void finish()	{}

	private:
		Util::Reference<Component> myComponent;
		float lastTime;
		float myStartTime;
		float duration;
};

}

#endif // ANIMATIONHANDLER_H
