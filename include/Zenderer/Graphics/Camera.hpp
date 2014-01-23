/**
 * @file
 *  Zenderer/Graphics/Camera.hpp - Handles viewing, object scaling (projection),
 *  and camera panning integration.
 *
 * @author      George (@_Shaptic)
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
 * @addtogroup Graphics
 * @{
 **/

#ifndef ZENDERER__GRAPHICS__CAMERA_HPP
#define ZENDERER__GRAPHICS__CAMERA_HPP

#include "Zenderer/Math/Math.hpp"

namespace zen
{
namespace gfx
{
    class zCamera
    {
    public:
        void SetProjectionMatrix(const math::mat4_t& proj)
        {
            m_proj = proj;
        }

        void SetViewMatrix(const math::mat4_t& view)
        {
            m_view = view;
        }

        void Pan(const math::vector_t& rate)
        {
            m_view = glm::translate(m_view, rate);
        }

        void Pan(const real_t x, const real_t y)
        {
            this->Pan(math::vector_t(x, y));
        }

        const math::mat4_t& GetProjectionMatrix()  const { return m_proj;    }
        const math::mat4_t& GetViewMatrix()        const { return m_view;    }
        const math::vector_t& GetPanning()         const
        {
            return math::vector_t(m_view[3].x, m_view[3].y, m_view[3].z);
        }

    private:
        math::mat4_t m_proj, m_view;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__CAMERA_HPP

/** @} **/
