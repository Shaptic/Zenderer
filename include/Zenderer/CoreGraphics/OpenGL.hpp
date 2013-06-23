/**
 * @file
 *  Zenderer/CoreGraphics/OpenGL.hpp - Includes necessary OpenGL-related
 *  headers and defines convenience functions.
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

#include "GL/glew.h"
#include "GL/glfw.h"

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Vector.hpp"
#include "Zenderer/Utilities/Assert.hpp"

#ifdef GL
  #undef GL
#endif // GL

#ifdef _DEBUG
  /// Error-checking OpenGL function call (debug builds only).
  #define GL(f) f; zen::gfxcore::glCheck(#f, __LINE__, __FILE__);
#else
  #define GL(f) f; if(glGetError() != GL_NO_ERROR) return false;
#endif // _DEBUG

namespace zen
{
    void ZEN_API Quit();

namespace gfxcore
{
    /**
     * Checks the validity of an OpenGL call.
     *  Mostly used internally for debugging, this will check valid
     *  execution of an OpenGL call. It should be used in congruence with
     *  the macro defined at the top of this file (GL(f)).
     *  This function won't really work well without the macro, as too
     *  many things have to be passed as parameters for accuracy; it'd be
     *  unwieldy to pass the string of the OpenGL function call by hand
     *  every time.
     *
     * @param   char*       OpenGL call expression
     * @param   uint32_t    Line number of function call
     * @param   char*       File name of function call
     *
     * @return  `true`      if there was no error, and
     *          `false`     otherwise, though the assertion will exit.
     **/
    static inline ZEN_API bool glCheck(const char* expr,
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
    class ZEN_API CGLSubsystem
    {
    public:
        CGLSubsystem(string_t name = "Renderer");
        virtual ~CGLSubsystem(){}

        virtual bool Init()     = 0;
        virtual bool Destroy()  = 0;

        virtual bool Bind()     = 0;
        virtual bool Unbind()   = 0;

        virtual GLuint GetObjectHandle() = 0;

        virtual inline const string_t& GetName() const
        { return m_name; }

        friend void zen::Quit();

    protected:
        bool m_init;

    private:
        static std::vector<CGLSubsystem*> sp_allGLSystems;

        string_t m_name;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__OPENGL_HPP

/** @} **/
