/**
 * @file
 *  Zenderer/Zenderer.hpp - Includes the user-exposed portions of the engine.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").         \n
 *  You may not use this file except in compliance with the License.        \n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0                              \n
 *  Unless required by applicable law or agreed to in writing, software     \n
 *  distributed under the License is distributed on an "AS IS" BASIS,       \n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and     \n
 *  limitations under the License.
 *
 * @addtogroup Engine
 *  A generic group for the misfits of other groups.
 * @{
 **/

#ifndef ZENDERER__ZENDERER_HPP
#define ZENDERER__ZENDERER_HPP

// Link graphics libraries.
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLu32.lib")
#pragma comment(lib, "GLEW32.lib")

#ifdef _DEBUG
  #pragma comment(lib, "GLFW3_DBG.lib")
#else
  #pragma comment(lib, "GLFW3.lib")
#endif // _DEBUG

// Link audio libraries
#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "alut.lib")
#pragma comment(lib, "libvorbisfile.lib")
#pragma comment(lib, "libvorbis.lib")
#pragma comment(lib, "libogg.lib")

// For random-ness
#include <cstdlib>

// Core API
#include "Core/Types.hpp"
#include "Math/Math.hpp"
#include "Utilities/Utilities.hpp"

// Custom memory allocator.
//#include "Core/Allocator.hpp"

// Graphics API
#include "Graphics/Window.hpp"
#include "Graphics/Quad.hpp"
#include "Graphics/Light.hpp"
#include "Graphics/Scene.hpp"

// Assets
#include "Assets/AssetManager.hpp"

// Audio API
#include "Audio/AudioManager.hpp"
#include "Audio/Music2D.hpp"
#include "Audio/Sound2D.hpp"

namespace zen
{
    /// Initializes the subsystems of @a Zenderer.
    ZEN_API bool Init();

    /// Destroys all subsystems of @a Zenderer.
    ZEN_API void Quit();
}

#endif // ZENDERER__ZENDERER_HPP

/** @} **/

/**
 * @mainpage
 *
 * @section intro   Introduction
 *  @subsection about_prj   About the Project
 *      @a Zenderer grew out of increasing frustration with @a IronClad, an
 *      OpenGL rendering engine I wrote from scratch in order to learn the
 *      basics of OpenGL. As it starting growing into a game I dubbed @a
 *      Praecursor, it became more and more unwieldy and totally inflexible
 *      in most contexts. Thus grew the need for a simpler, more
 *      user-friendly engine that I could use without worrying about
 *      low-level details to further pursue development of @a Praecursor,
 *      or to start a new project as I see fit.
 *      I know now a ton more than I did when I first started creating
 *      @a IronClad, so I can guarantee that I will not make any of the
 *      same mistakes again.
 *
 * @subsection about_me     About Me
 *      My name is George Kudrayvtsev and I have been programming for a
 *      hobby for around 5 years now. I started out with C++, dropped it
 *      for Python, then came back around. I've been developing games
 *      with C++ for about 2 years, and have grown from using the SDL API
 *      to creating a full-fledged 2D rendering engine as you can see here.
 *      I also develop software professionally, currently interning at a
 *      medical education company doing back and front-end website
 *      development using PHP, XML/XSLT, PostgreSQL, Javacript/jQuery, and
 *      other web development tools.\n
 *      My current pet project is @a Praecursor, an awesome concept game in
 *      the form of a 2D platformer. The main game mechanic revolves around
 *      time manipulation to get through levels and past enemies. For now,
 *      though, that's on hold while I recover my bearings and put some
 *      time into this rendering engine to (hopefully) vastly improve what
 *      was brought about in @a IronClad.
 *      Feel free to check out my blog
 *      [here](http://zenpandainteractive.blogspot.com) or my GitHub
 *      [here](https://www.github.com/Ruskiy69). You can also follow
 *      @a Praecursor's progress on Twitter
 *      [here](https://www.twitter.com/PraecursorGame), though it will
 *      probably be slow going for a while, with my job taking up most of
 *      my development energy and this engine taking everything else.
 *
 * @section install Installation
 *  Installing @a Zenderer is quite trivial. All you need are the header
 *  files found in `Zenderer/include`, and all of the dependency libraries
 *  you can find in the `lib` directory of the project root. There is
 *  currently only support for Windows (*gasp*), due to the fact that I use
 *  Visual Studio and the Microsoft compiler for 95% of my C++ development.
 *  Thus, I can't guarantee any sort of cross-platform compatibility. But,
 *  this is obviously a potential TODO for when everything else has been taken
 *  care of.
 *
 *  There are various dependencies for this engine. Luckily, they are all
 *  cross platform and are relatively easy to build from source. They are
 *  as follows:
 *      -   GLEW            (OpenGL Extension Wrangler Library)
 *      -   GLFW            (OpenGL FrameWork)
 *      -   Freetype 2      (TrueType Font Library)
 *      -   OpenAL          (Open Audio Library)
 *      -   libvorbis       (OGG/Vorbis Audio Compression Algorithm)
 *
 * @note    All file version numbers are meaningless until a stable
 *          1.0.0-alpha release.
 *
 * @todo    Test cross-platform compatibility.
 *
 * @section arch    Architecture
 *  The @a Zenderer engine is broken up into multiple components so that
 *  the user can use whichever parts are necessary for them. Despite that,
 *  almost all of the components interact with each other a lot, so by
 *  using one you are most likely using multiple others.
 *  At the root of these various components is often times a **subsystem**.
 *  In @a Zenderer, a subsystem is defined as an object with startup and
 *  shutdown methods that clean up completely after themselves and depend
 *  on nothing. This basic idea was put into place after numerous crashes
 *  occurred in @a IronClad due to the graphics subsystem attempting
 *  drawing or the asset subsystem attempting cleanup after there was no
 *  more OpenGL context existing, thus invalidating any handles that these
 *  systems had. Hopefully as development progresses, this fundamental
 *  subsystem architecture will prevent a similar problem. \n
 *
 *  Below are detailed explanations of each of the components.
 *
 *  @subsection comp_util   Utilities
 *      These are general-purpose utilities that act as 'helper' objects
 *      and functions throughout the engine. You can find a very nice (IMO)
 *      logging utility in Zenderer/Utilities/Log.hpp that is fairly
 *      flexible and easy to use. There is also a file parser that works
 *      extremely well for files that utilize `key=value` pairs, such as
 *      `.ini` files. This `key=value` relationship is used extensively
 *      throughout @a Zenderer for level loading and other components. Also
 *      included are some file-loading functions, to load image files
 *      for sprite sheets or generic textures for use in the engine. It has 
 *      only been tested internally with 24-bit and 32-bit `.png` files in 
 *      RGB and RGBA format, respectively.
 *
 *      You can find some basic usage examples of these utilities
 *      in the [Examples](examples.html) tab above.
 *
 *  @subsection comp_math   Math
 *      Math lies at the very core of computer graphics and game
 *      development. The math component does not include a subsystem object
 *      because it is primarily a bunch of loosely related objects and
 *      functions that do not need any sort of initialization. Almost all
 *      the other components rely on this component to function. It's at
 *      the core of the scene manager and basically the entire graphics
 *      component, since almost everything needs at least a position on
 *      the screen.
 *
 *  @subsection comp_gfx    Graphics
 *      This is the bread and butter of the engine; this is what everyone
 *      really cares about. The graphics component of @a Zenderer is set up
 *      on a scene basis. A scene is an object that contains a vertex
 *      buffer for geometry, a render target to draw to, lists of
 *      post-processing effects (see zen::gfx::CEffect for more), lists of
 *      lights (zen::gfx::CLight), and of course lists of drawable objects
 *      to render to the screen. Though viewing the raw source may be more
 *      beneficial than a textual explanation, the rendering process is
 *      outlined here.
 *
 *      First, we render all of the scene geometry to an off-screen
 *      frame-buffer. This is done efficiently by sorting the geometry
 *      based on the material it uses (a material is a combination of a
 *      shader and a texture) and then batching draw calls with each type.
 *
 *      Then we make a pass for each light and use additive
 *      blending to achieve realistic lighting effects. Finally, in order
 *      to properly post-process, a "ping-pong" technique is used on the
 *      rendered scene to overlay effects on top of one another.
 *      This is then drawn directly to the scene's render target (the
 *      screen by default) using a full-screen quad stored in a globally
 *      accessible vertex buffer.
 *
 *  @subsection comp_sfx    Audio
 *      The audio subsystem uses OpenAL and libvorbis for its underlying
 *      sound playing. Files are primarily split into two parts: sound
 *      effects and music files. Sound effects are typically short,
 *      WAV files loaded completely into memory and played many times
 *      repetitively. Music files, on the other hand, are usually very
 *      long and played continuously. They don't have to be music, of course;
 *      dialogue can be another use. Music files cannot be rewound and
 *      can only be repeated from the very beginning due to their streamed
 *      nature.
 *
 *      File Type | Extension | Purpose
 *      :-------- | :-------: | :--------------------------------
 *      Ogg-Vorbis| .ogg      | Lengthy, continuous audio files
 *      WAV       | .wav      | Short, uncompressed sound effects
 *
 * @page specs  File Specifications
 *  Formatting specifications for various file types used in @a Zenderer.
 *
 * @section zlvl    ZenLevel
 *  @subsection zlvlspec    Specification
 *      Levels in @a Zenderer combine the @ref entity specification 
 *
 *
 *  @subsubsection zlvl_light  Light Options
 *      All floating point values can have a maximum of three decimal
 *      places of precision in them.
 *      fp -- floating-point
 *
 *      Ambient Lights:
 *          * type          -- 0
 *          * color         -- 3 fp values
 *          * brightness    -- 1 fp value
 *
 *      Point Lights:
 *          * type          -- 1
 *          * color         -- 3 fp values
 *          * brightness    -- 1 fp value
 *          * attenuation   -- 3 fp values
 *
 *      Spot Lights:
 *          * type          -- 1
 *          * color         -- 3 fp values
 *          * brightness    -- 1 fp value
 *          * attenuation   -- 3 fp values
 *          * maxangle      -- 1 fp value
 *          * minangle      -- 1 fp value
 *
 *  @subsubsection example     Example File
 *
 *  @code{.ini}
 *  filename=sample.zlv
 *  title=Level 1 -- Tutorial
 *
 *  // Place entity from existing texture file
 *  <entity type="texture">
 *    position=100,200,0
 *    filename=textures/grass.png
 *  </entity>
 *
 *  // Place entity from existing entity file
 *  <entity type="file">
 *    position=100,100,0
 *    filename=assets/grass.znt
 *  </entity>
 *
 *  // Place entity using in-line entity mesh creation.
 *  // This is a tiled grass entity (2x2 tiles) 
 *  // with a slight blur and vertex inversion for skewing,
 *  // together with a dirt entity.
 *  <entity type="inline">
 *    position=0,0,0
 *    <mesh>
 *      width=64
 *      height=64
 *      repeat=true
 *      invert=true
 *      texture=textures/grass.png
 *      shader=shaders/default.vs,shaders/GaussianBlurH.fs
 *      params=radius:0.005
 *    </mesh>
 *    <mesh>
 *      texture=textures/dirt.png
 *    </mesh>
 *  </entity>
 *
 *  <light>
 *    // 2 --> spotlight
 *    type=2
 *    color=0.050,0.762,0.556
 *    attenuation=0.001,0.005,0.000
 *    position=100,200
 *    minangle=45.0
 *    maxangle=225.0
 *  </light>
 *  @endcode
 *
 *  The in-line entity multi-mesh creation featured in the example file
 *  would be equivalent to the following API code:
 *
 *  @code
 *  CQuad Q1, Q2;
 *  CMaterial M1, M2;
 *
 *  // Set up the first primitive.
 *  M1.LoadEffect(EffectType::GAUSSIAN_BLUR_H);
 *  M1.GetEffect()->Enable();
 *  M1.GetEffect()->SetParameter("radius", 0.005);
 *  M1.GetEffect()->Disable();
 *  M1.LoadTextureFromFile("textures/grass.png");
 *
 *  Q1.Resize(64, 64);
 *  Q1.SetRepeating(true);
 *  Q1.SetInverted(true);
 *  Q1.AttachMaterial(M1);
 *  Q1.Create();
 *
 *  // Set up the second primitive.
 *  M2.LoadTextureFromFile("textures/dirt.png");
 *  Q2.AttachMaterial(M2);
 *
 *  CEntity E;
 *  E.AddPrimitiveMesh(Q1);
 *  E.AddPrimitiveMesh(Q2);
 *  E.Create();
 *
 *  // Later...
 *  E.Draw();
 *  @endcode
 * 
 * @section  ZMat           ZenFX
 *  @subsection ZMatSpec    Specification
 *  Extension: `.zfx`
 *
 *  This is a simple INI-like file that contains filenames
 *  to create custom materials from a combination of shaders
 *  and textures. The filenames can be paths, but they must
 *  be complete relative paths that do not rely on the internal
 *  short-cut macros, such as `ZENDERER_SHADER_PATH`.
 *
 *  Expected key-value pairs:
 *  | Key       |        Description       | Required |
 *  | --------- | ------------------------ | :------: |
 *  | `vshader` | Vertex shader filename   |    x     |
 *  | `fshader` | Fragment shader filename |    x     |
 *  | `texture` | Texture filename         |    x     |
 *  | `params`  | Shader parameters        |          |
 *
 *  Shader parameters are options found in the shader files that
 *  customize how it looks. They are optional, but can be included
 *  if you desire a default effect.
 *  They should all be on one line, in the form
 *  `name1:val1,val2,val3;name2:val1`
 *  Thus a shader wanting a `radius` of (10, 6) and a `time` of 0.34
 *  would be specified like so:
 *
 *      params=radius:10,6;time:0.34
 *
 *
 *  @subsection ZMatEx      Example File
 *  This is an example ZenFX file using the default shaders and a fake
 *  sample texture.
 *
 *  @code{.ini}
 *  // Example.zfx
 *  vshader=Zenderer/shaders/Default.vs
 *  fshader=Zenderer/shaders/Default.fs
 *  texture=Assets/textures/Sample.png
 *  @endcode
 *
 * @section ZEnt            ZenTity
 *  @subsection ZEntSpec    Specification
 *  Extension: `.znt`
 *
 *  In @a Zenderer, entities are essentially extremely versatile
 *  primitive instances. The most important distinction is
 *  multi-primitive rendering. So if you had, for example, a 
 *  table made of two leg textures and a top texture, you could
 *  create a single entity from it. 
 *
 *  The file specification features a lot of optional values and 
 *  valid/invalid combinations, all of which are detailed below.
 *
 *  The following keys are part of the raw entity specification, not the
 *  individual primitive specification. 
 *
 *  |   Key      |     Description    | Required |           Notes           |
 *  | :--------: | ------------------ | :------: | :------------------------ |
 *  | `position` | Position on screen |    x     | In the form x,y,z         |
 *  | `primcount`| # of primitives    |          | Allows for pre-allocation | 
 *
 *  The `z` on the `position` key is optional, but specifying depth is required 
 *  for proper shadow rendering. It will default to 0.
 *
 *  The following are individual primitive options.
 *  These are placed within a `<prim></prim>` block, which can
 *  be repeated as many times as necessary.
 *
 *  |   Key     |     Description    | Required |               Notes               |
 *  | :-------: | ------------------ | :------: | :-------------------------------- |
 *  | `invert`  | Invert vertices?   |          | Defaults to `false`               |
 *  | `repeat`  | Repeat texture?    |          | Defaults to `false`               | 
 *  | `width`   | Primitive width    |          | Defaults to texture width         |
 *  | `height`  | Primitive height   |          | Defaults to texture height        |
 *  | `vshader` | Vertex shader file |          |                                   |
 *  | `fshader` | Pixel shader file  |          | Pixel / Fragment                  |
 *  | `texture` | Texture file       |    x     | This or material file             |
 *  | `material`| Material file      |    x     | This or texture file              |
 *  | `params`  | Shader parameters  |          | Format: `name:val1,val2;name:val1`|
 *
 *  If overlapping values are used for `material` and the texture file, 
 *  the file keys will be preferred over the material file. Thus if you specify
 *  `vshader`, `fshader`, `texture`, *and* `material`, the local files will be
 *  loaded, as opposed to the material file. 
 *
 *  If the shader files are left off, the default will be used (like for
 *  primitives). If the size is specified (`width`/`height`), *both* must be
 *  specified. Otherwise, the default values are used for both of them. 
 *
 *  The `primcount` option will allow for a tiny speed up in loading, but will
 *  only make a real difference if the entity contains dozens of primitives.
 *
 *  @see    zen::gfx::CQuad::SetInverted()
 *  @see    zen::gfx::CQuad::SetRepeating()
 *
 *  @subsubsection  ZEntEx  Example File
 *  @code{.ini}
 *  // This is an example ZenTity file with two
 *  // side-by-side quads; one has a texture and
 *  // the other is mapped with a texture and a
 *  // pair of shaders.
 *
 *  // Metadata, ignored.
 *  filename=sample.znt
 *  author=George Kudrayvtsev
 *  version=1.0
 *
 *  // Position on-screen
 *  position=100,100,5
 *  primcount=2
 *
 *  // Tiled twice in X, twice in Y
 *  // Slight horizontal blur, ready to be skewed along
 *  // the top vertices.
 *  <prim>
 *    texture=textures/grass.png
 *
 *    vshader=shaders/Default.vs
 *    fshader=shaders/GaussianBlurH.fs
 *    params=radius:0.005
 *
 *    width=64
 *    height=64
 *
 *    repeat=true
 *    invert=true
 *  </prim>
 *
 *  // Nothing special about this one
 *  <prim>
 *    texture=textures/dirt.png
 *  </prim>
 *
 * @endcode
 *
 * @section zanim   ZenImation
 *  @subsection zanimspec   Specification
 *
 *  @subsubsection example     Example File
 *
 **/
