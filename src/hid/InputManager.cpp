//
//  InputManager.cpp
//  sentinel
//
//  Created by Austin Kee on 2/2/18.
//  Copyright © 2018 Austin Kee. All rights reserved.
//

#include "InputManager.h"

namespace sentinel
{

/*ctor*/
InputManager::InputManager() :
	m_quit(false)
{
	//do nothing!
}

/*dtor*/
InputManager::~InputManager()
{
	//do nothing!
}

/**
 * spin up an io thread
 */
void InputManager::startUp()
{
	m_input_thread = std::thread([this]() {
		watchKeyEvent();
	});
}

/**
 * spins down our io thread
 */
void InputManager::shutDown()
{
	m_quit = true;
	if (m_input_thread.joinable())
		m_input_thread.join();
}

/**
 * NOTE: to Austin:
 * If this runs in its own thread,
 * probably best to just have it set
 * some internal state every time a key is
 * pressed, and just reset the key presses
 * every frame. Then we can just call
 * some sort of GET on the InputManager
 * to get currently pressed keys for this frame.
 * just an idea
 *
 * @returns an int (of some sort? My b, just fill in the blanks Austin :) )
 */
int InputManager::watchKeyEvent()
{
	SDL_Event e;
	while (!m_quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				//quit signal
			}
			else if (e.type == SDL_KEYDOWN)
			{
				switch (e.type)
				{
				case SDL_MOUSEMOTION:
					int mousex, mousey;
					mousex = e.motion.x;

					mousey = e.motion.y;
					break;
				}
				switch (e.key.keysym.sym)
				{
				case SDLK_0:
					//pass 0 to UI
					break;
				case SDLK_1:
					//pass 1 to UI
					break;
				case SDLK_2:
					//pass 2 to UI
					break;
				case SDLK_3:
					//pass 3 to UI
					break;
				case SDLK_4:
					//pass 4 to UI
					break;
				case SDLK_5:
					//pass 5 to UI
					break;
				case SDLK_6:
					//pass 6 to UI
					break;
				case SDLK_7:
					//pass 7 to UI
					break;
				case SDLK_8:
					//pass 8 to UI
					break;
				case SDLK_9:
					//pass 9 to UI
					break;
				case SDLK_MINUS:
					//pass '-' to UI
					break;
				case SDLK_PLUS:
					//pass '+' to UI
					break;
				case SDLK_ESCAPE:
					//pass 'ESC' to UI
					break;
				default:
					break;
				}
			}
		}
	}
	return 0;
}

} //namespace sentinel