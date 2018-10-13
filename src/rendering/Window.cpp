//
//  window.cpp
//	sentinel
//
//  Created by Austin Kee on 1/16/18.
//  Copyright © 2018 Austin Kee. All rights reserved.
//

#include "Window.h"

namespace sentinel
{

window::window() :
	m_window(nullptr),
	m_renderer(nullptr),
	m_img(nullptr)
{}

int window::start_window(const char* window_name, const int window_w, const int window_h)
{
	SDL_Init(SDL_INIT_VIDEO);

	m_window = SDL_CreateWindow(
		window_name,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		window_w,
		window_h,
		SDL_WINDOW_OPENGL
	);

	SDL_SetWindowResizable(m_window, SDL_TRUE);

	if (m_window == nullptr)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	return 0;
}

int window::draw()
{
	return 0;
}

int window::window_watcher()
{

	return 0;
}

int window::fullscreen_mode(int i)
{
	if (i <= 2 && i >= 0)
	{
		if (i == 0)
		{
			SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
		}
		else if (i == 1)
		{
			SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else
		{
			SDL_SetWindowFullscreen(m_window, 0);
		}
	}
	else
	{
		std::cout << "Warning, window_mode flag is outside of the valid range (0-2)." << std::endl;
	}
	return 0;
}

int window::destroy_window()
{
	SDL_DestroyWindow(m_window);

	SDL_Quit();

	return 0;
}

}//namespace sentinel