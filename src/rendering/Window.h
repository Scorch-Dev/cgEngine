//
//  window.h
//  sentinel
//
//  Created by Austin Kee on 1/16/18.
//  Copyright © 2018 Austin Kee. All rights reserved.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <SDL.h>

namespace sentinel
{

class window
{
public:
	window();

	int start_window(const char* window_name, const int window_w, const int window_h);
	int draw();
	int window_watcher();
	int fullscreen_mode(int i);
	int destroy_window();

	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_img;
};

}//namespace sentinel

#endif /* WINDOW_H */