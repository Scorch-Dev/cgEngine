//
//  InputManager.h
//  sentinel
//
//  Created by Austin Kee on 2/2/18.
//  Copyright © 2018 Austin Kee. All rights reserved.
//

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <iostream>
#include <thread>

#include <SDL.h>
#include <SDL_keycode.h>

namespace sentinel
{

class InputManager
{
public:
	InputManager();
	~InputManager();

	void startUp();
	void shutDown();

	//TODO: function not implemented yet
	//int watch_mevent();

private:
	int watchKeyEvent();

	std::thread m_input_thread;
	bool m_quit;

};

}//namespace sentinel

#endif /* INPUT_MANAGER_H */