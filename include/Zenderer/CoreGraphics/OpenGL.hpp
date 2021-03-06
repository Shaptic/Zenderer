/**
 * @file
 *  Zenderer/CoreGraphics/OpenGL.hpp - Includes necessary OpenGL-related
 *  headers and defines convenience functions.
 *
 * @author      george (halcyon)
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
 * @addtogroup CoreGraphics
 *  This group is a low-level abstraction layer over various API-specific
 *  functionality. Currently, @a Zenderer only supports OpenGL, but with a
 *  good level of abstraction, it should be possible to adapt it to use
 *  DirectX for rendering, as well.
 * @{
 **/

#ifndef ZENDERER__CORE_GRAPHICS__OPENGL_HPP
#define ZENDERER__CORE_GRAPHICS__OPENGL_HPP

#include <vector>

// I statically link GLEW when using MinGW.
#ifdef __GNUC__
  #define GLEW_STATIC
#endif // __GNUC__

#include "GL/glew.h"
#include "GL/glfw3.h"

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Vector.hpp"
#include "Zenderer/Utilities/Assert.hpp"

#ifdef GL
  #undef GL
#endif // GL

#ifdef ZEN_DEBUG_BUILD
  /// Error-checking OpenGL function call (debug builds only).
  #define GL(f) f; zen::gfxcore::glCheck(#f, __LINE__, __FILE__);
#else
  #define GL(f) f; if(glGetError() != GL_NO_ERROR) return false;
#endif // ZEN_DEBUG_BUILD

namespace zen
{
    void ZEN_API Quit();
    namespace gfx { class ZEN_API zWindow; }

/**
 * Low level graphics layer.
 *  Here we encompass a low-level graphics layer used internally by
 *  @a Zenderer for rendering operations. Most of these are abstracted
 *  away by higher level APIs in `zen::gfx`.
 **/
namespace gfxcore
{
    /**
     * Checks the validity of an OpenGL call.
     *
     * @param   expr    OpenGL call expression
     * @param   line    Line number of function call
     * @param   file    File name of function call
     *
     * @return  `true`  if there was no error, and
     *          `false` otherwise, though the assertion will exit the program.
     **/
    inline ZEN_API bool glCheck(const char* expr,
                                const uint32_t line,
                                const char* file)
    {
        GLenum error_code = glGetError();

        if(error_code == GL_NO_ERROR) return true;

        // So it's not recreated on the stack every time.
        static std::stringstream ss;

        // Clear existing data (since static).
        ss.str(std::string());

        // Format: CODE (STRING).
        ss << error_code << " (" << gluErrorString(error_code) << ").";

        // Express our discontent.
        util::runtime_assert(false, expr, line, file, ss.str().c_str());

        return false;
    }

    struct ZEN_API vertex_t
    {
        math::vector_t  position;   ///< Vertex position
        math::vector_t  tc;         ///< Vertex texture coordinates
        color4f_t       color;      ///< Vertex color
    };

    /// A wrapper for OpenGL objects with proper cleanup.
    class ZEN_API zGLSubsystem
    {
    public:
        zGLSubsystem(const string_t name = "OpenGL");
        virtual ~zGLSubsystem();

        virtual bool Init()     = 0;
        virtual bool Destroy()  = 0;

        virtual bool Bind() const   = 0;
        virtual bool Unbind() const = 0;

        virtual GLuint GetObjectHandle() const = 0;

        inline bool IsInit() { return m_init; }

        virtual inline const string_t& GetName() const
        { return m_name; }

        friend ZEN_API void zen::Quit();
        friend gfx::zWindow;

    protected:
        bool m_init;

    private:
        static std::vector<zGLSubsystem*> sp_allGLSystems;

        string_t m_name;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__OPENGL_HPP

/**
 * @fn zen::gfxcore::glCheck()
 * @details
 *  Mostly used internally for debugging, this will check valid
 *  execution of an OpenGL call. It should be used in congruence with
 *  the macro defined at the top of this file (GL(f)).
 *  This function won't really work well without the macro, as too
 *  many things have to be passed as parameters for accuracy; it'd be
 *  unwieldy to pass the string of the OpenGL function call by hand
 *  every time.
 **/

/**
 * @class zen::gfxcore::zGLSubsystem
 * @details
 *  Similar to zen::zSubsystem, this acts as a managed wrapper around
 *  various OpenGL object instances. Anything registered with this as
 *  a base class will automatically be `Destroy()`-ed when the engine
 *  is quit.
 **/

/** @} **/
