== Zenderer -- v0.1 pre-release ==

=== About ===
*Zenderer* grew out of increasing frustration with *IronClad*, an 
OpenGL rendering engine I wrote from scratch in order to learn the 
basics of OpenGL. As it starting growing into a game I dubbed 
*Praecursor*, it became more and more unwieldy and totally inflexible
in most contexts. Thus grew the need for a simpler, more
user-friendly engine that I could use without worrying about
low-level details to further pursue development of @a Praecursor,
or to start a new project as I see fit.
I know now a ton more than I did when I first started creating 
*IronClad*, so I can guarantee that I will not make any of the
same mistakes again.

=== Installation ===

Installing *Zenderer* is quite trivial. All you need are the header 
files found in `Zenderer/include`, and the `Zenderer.lib` file you can
find in the `Binaries` directory of the project root. There is
currently only support for Windows (*gasp*), due to the fact that I use
Visual Studio and the Microsoft compiler for 95% of my C++ development.
Thus, I can't guarantee any sort of cross-platform compatibility. This
is obviously a potential `TODO` for when everything else has been taken
care of.
 
There are various dependencies for this engine. Luckily, they are all
cross platform and are relatively easy to build from source. They are
as follows:
    -   GLEW            (OpenGL Extension Wrangler Library)
    -   GLFW            (OpenGL FrameWork)
    -   Freetype 2      (TrueType Font Library)
    -   OpenAL          (Open Audio Library)
    -   libvorbis       (OGG/Vorbis Audio Compression Algorithm)
    
=== More Info ===

*Zenderer* comes with a very well-documented API, which you can easily find
in the `docs` folder in this repository. It was generated using doxygen, 
meaning the identical commentary can be found within the source code itself.

Feel free to check out my blog [here](http://zenpandainteractive.blogspot.com).
You can also follow Praecursor's progress on Twitter
[here](https://www.twitter.com/PraecursorGame), though it will 
probably be slow going for a while, with my job taking up most of
my development energy and this engine taking everything else.