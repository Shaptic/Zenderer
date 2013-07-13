# Zenderer #
v0.1.240-dev  
Copyright &copy; George Kudrayvtsev 2013

#### About ####

*Zenderer* (rhymes with renderer) grew out of increasing frustration with
[IronClad](https://github.com/Ruskiy69/IronClad), an OpenGL rendering engine
I wrote from scratch in order to learn the basics of OpenGL. As it starting
growing into a game I dubbed *Praecursor*, it became more and more unwieldy
and totally inflexible in most contexts. Thus grew the need for a simpler,
more user-friendly engine that I could use without worrying about low-level
details to further pursue development of 
[Praecursor](https://github.com/Ruskiy69/Praecursor), or to start a new
project as I see fit. I know now a ton more than I did when I first started
creating *IronClad*, so I can guarantee that I will not make any of the
same mistakes again.

#### Installation ####

Installing *Zenderer* is quite trivial. All you need are the header
files found in `Zenderer/include`, and all of the dependency libraries
you can find in the `lib` directory of the project root. There is
currently only support for Windows (*gasp*), due to the fact that I use
Visual Studio and the Microsoft compiler for 95% of my C++ development.
Thus, I can't guarantee any sort of cross-platform compatibility. But,
this is obviously a potential `todo` for when everything else has been taken
care of.

There are various binary dependencies for this engine. Luckily, they are all
cross platform and are relatively easy to build from source. They are
as follows:
 - GLEW         (OpenGL Extension Wrangler Library)
 - GLFW         (OpenGL FrameWork)
 - Freetype 2   (TrueType Font Library)
 - OpenAL       (Open Audio Library)
 - libvorbis    (OGG/Vorbis Audio Compression Algorithm)
 - Lua 5.2      (Lua Scripting Language)

All engine assets are stored in the `data` folder of the repository. If you 
are using Visual Studio in junction with *Zenderer* for your project, the 
included project file will automatically transfer the contents of the `data`
folder to the `Zenderer` folder of your expected output directory.
You can see the various `ZENDERER_*_PATH` `#define`s sprinkled throughout the
code to get an idea of where the engine will search for certain resources.
For example, shader files are expected to be in `Zenderer/shaders/*`.
In `Release` mode, the project will attempt to generate documentation, but
this will likely fail on your system, which is totally okay. I use this for
myself internally to update the Doxygen docs.
    
#### More Info ####

*Zenderer* comes with a very well-documented API, which you can easily find
in the `docs` folder in this repository. It was generated using Doxygen, 
meaning the identical commentary can be found within the source code itself.

Feel free to check out my blog [here](http://zenpandainteractive.blogspot.com).
You can also follow *Praecursor*'s progress on Twitter
[here](https://www.twitter.com/PraecursorGame), though it will 
probably be slow going for a while, with my job taking up most of
my development energy and this engine taking everything else.
