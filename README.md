# Sentinel

The Sentinel Engine is a game engine being written in C++ on top of the SDL2 framework as a part of Project Sanctuary. It is currently not complete, but there are a number of tests available as well as the source code to peruse.

## Building From Source

*DISCLAIMER:* We are currently targeting OSX, Linux, and Windows, so, if you're not on one of those platforms, we can't promise that the project will compile or run for you at all.

The first step is to make sure you have CMake installed. You can download and install CMake by following the instructions on their [website](https://cmake.org/). Alternatively, Visual Studio has the option to install CMake support directly in the IDE if this fits your workflow better.

#### For Linux and Mac:

For Linux and Mac you now need to make sure SDL2 is installed. On most linux distros, you can use your favorite package manager to install the SDL development libraries (note, this is not the same as the source libraries). This might look something like:

    apt-get install libsdl2-dev

On Mac you can alternatively use the .dmg to install SDL2 from their [website](https://www.libsdl.org/download-2.0.php).
It's preferable to place the contents under ~/Library/Frameworks.

At this point, you can just run the following commands:

    cd <your>/<path>/<to>/Sentinel
	cmake ./
	make

And congrats you've done, Sentinel has now compiled. Alternatively, if you want to keep your root directory clean, you can instead run this set of commands:

    cd <your>/<path>/<to>/Sentinel
	mkdir build
	cd build
	cmake ../
	make

#### For Windows:

As with most things in development though, if you're on windows, life gets a bit harder. SDL pretty much just gives you a zip file and tells you to figure out, so you need to follow the following steps for CMake to find SDL properly.

For SDL you want to download the SDL2 development libraries (not the source code) from the [website](https://www.libsdl.org/download-2.0.php). From there, you should extract from inside the zip archive a folder called `SDL2-<version>` to the directory `Sentinel/lib` directory (if it does not exist, go ahead and create it now. Note that the word `lib` is case-sensitive.). The <version> part is whatever version of SDL you happened to download (though the minimum required version is SDL2-2.0.0) You should now rename this folder from `SDL2-<version>` to `SDL2`.

From there, you have to go into the folder `Sentinel/extras` and copy the file `sdl2-config.cmake` into the `SDL2` directory we just made.

Finally, to make sure that your program can link at runtime to the SDL2 library, if you are targeting a 64-bit architecture, you need to go into the folder `Sentinel/SDL2/lib/x64` and copy the file `SDL2.dll` into `C:\Windows\System32` directory to finish the install. Alternatively, if you are compiling to target 32-bit architecture instead of the 64-bit version, you should instead go into the folder `Sentinel/SDL2/lib/x86` and copy the file `SDL2.dll` from that folder into the folder `C:\Windows\SysWOW64`. (I know it seems counter-intuitive to put the 64-bit version in `System32` and the 32-bit version makes it into `SysWOW64`. That's not a typo, it's just how Windows is... *sigh*).

Now you're safe to run the CMake build from the root Sentinel Directory, and start hacking away. If you're using VisualStudio, you can reload CMake by hitting `Ctrl+s` in the root `CMakeLists.txt` file.
