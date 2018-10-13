
# In a word: CMake

There are a lot of build systems out, but the tried and true solution is currently, and has been for several years, CMake. CMake isn't perfect, but when it comes to cross-platform build systems, not many rival its robust features and massive community. If used properly, CMake can be an elegant build system with all the trimmings.

Before you get started though, it's important to know that CMake is BIG as a system. Similar to C++, you get the most utility out of actually *restricting* the CMake operations we are performing. CMake is a sea of macros, functions, definitions, etc. and not all of them are particularly useful. A lot of them exist for compatibility reasons (these are often identified directly in the CMake documentation), and a lot of it exists to serve niche applications that probably don't apply to us. When you strip away all of that fluff, you're left with the core of CMake that we and most others should be using.

I won't go into much here, only on the general CMake principle, how to add source, new executables/libraries, and adding new folders. If you want to actually understand what's happening rather than just following my instructions, you might want to try this [blog post](https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/) which explains the `target_` syntax used heavily in our project. As well, there's always the [CMake documentation](https://cmake.org/documentation/), which is invaluable.

## The Idea:

CMake works by inserting small text documents (usually called CMakeLists.txt) in each folder which contains source files. These files are usually rather small at each level of your project tree, and most only do some small jobs like including sources and identifying any subdirectories which contain `CMakeLists.txt` files. Though occasionally they do other things like ensuring packages and functions exist on a platform, using some sort of conditional logic based on platform or otherwise, or setting compile flags available in the editor. We can't cover everything here, but I'll cover the stuff you'll need to get yourself up and running.

## Adding Sources:

Adding sources is a pretty straight-forward process. I won't go over the entire system, and there's a few ways to do this, but the newest, and, arguably, cleanest solution is to use the `target_sources` command, and the `target_` family of functions. For the purposes of this project, if you have a folder in which you want to add a source, there are two things you need to know before-hand. First, what is the name of the executable or library you are including the source in. The second is if, if you're building into a library, which files need to be publicly exposed, and which are ok to keep internal to your library.

For example, say I want to add two files `MyArray.h` and `MyArray.cpp` to the utility library, which is linked against at compile time. I would first know that the utility library is called s_util in the build system (short for sentinel_utility to avoid name clashing). I would go into `src/util` and it might look like this after I add my two files:

	|-CMakeLists.txt
	|-src/
	|--CMakeLists.txt
	|--util/
	|--|-CMakeLists.txt
	|--|-MyArray.h
	|--|-MyArray.cpp
	|--|-<other sources>

I would then go into the CMakeLists.txt file located in the same folder I added my sources (in this case `/src/util`), and add my new files to the compile list. It might look like this:

	target_sources( s_util
		PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/MyArray.cpp"    <--- I need to add this line
		<some other source files>
		PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/MyArray.h"      <--- And this line too
		<some other source files>
	)

	target_include_directories(s_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})

I won't explain what the `PUBLIC`, `PRIVATE`, and other keywords mean, but if you follow those steps, you can safely use your source files in the project which you are now compiling it into (in this case the s_util library) with minimal effort. Note, the library s_util wasn't declared upfront in this directory, because it could have been created by another top-level `CMakeLists.txt` earlier (it was).

## Adding Folders

If you want to add a folder, you'll need to create your own `CMakeLists.txt` file from scratch. Here's a quick recipe for doing that. Given a similar structure to before:

	|-CMakeLists.txt
	|-src/
	|--CMakeLists.txt
	|--util/

Now say you want to create a new subfolder in `src` called `hid` (for human interface devices). You'll have to create your folder `hid` first, and create yourself a `CMakeLists.txt` file. Then, knowing what target you're building for, in this case we'll assume your building into the final sentinel game engine executable `sentinel`. In this case, you can then pretty much copy the format I outlined above (starting from the line `target_sources` to the line `target_include_directories`), into a `CMakeLists.txt` file you create in your folder `hid`.

Though there's one final step now, which is to go into the `CMakeLists.txt` file in the directory immediately above our new direcotry `hid`. So if we place `hid` in `/src/hid` then we need to go to the file `src/CMakeLists.txt` and edit it. Opening the file we need to append the following line:

    include (${CMAKE_CURRENT_LIST_DIR}/hid/CMakeLists.txt)

This will, in effect, perform an operation analagous to the C/C++ `#include` and allow our changes to propogate to the rest of the project.

## Adding an executable or library

Last thing we'll cover is executables and libraries. If you need to add a new executable or library, say a test or something, then you can folloow this procedure.

Simply open up the `CMakeLists.txt` file in the same directory as your "main" function will be located, and insert the following line before any `include` statements.

    add_executable(<output exe name> ${CMAKE_CURRENT_LIST_DIR}/<my-main>.cpp)

	#any include statements you might need go here

	target_link_libraries(<output exe name> <libs to link separated by spaces>)

The only difference would be for a library is that the line `add_executable` becomes:

    add_library(<output lib name> "")
	 #note the quotes are empty, as our subdir CMakeLists.txt files will append to this when included


## In Conclusion

This is just a quick-and-dirty intro if you really don't want to know the details and want get started coding as fast as possible. If you have any questsions, you can look at some of the exsting files and structures in the project. That said, would highly encourage you to read through the article I linked above as well as some of the CMake tutorials to get a real understanding.
