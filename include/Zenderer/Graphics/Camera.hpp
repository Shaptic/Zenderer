/**
 * @file
 *  Zenderer/Graphics/Camera.hpp - Declarations for the `zCamera` class, which
 *  handles adjusting views and projections of the game renderings.
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

#include "Zenderer/Math/Math.hpp";

namespace zen
{
namespace gfx
{
    class zCamera
    {
    public:
        zCamera();
        ~zCamera();

        void SetProjectionMatrix(const glm::mat4& m)
        {
            m_proj = m;
            m_comb = m_proj * m_view;
        }

        void SetViewMatrix(const glm::mat4& m)
        {
            m_view = m;
            m_comb = m_proj * m_view;
        }

        void Pan(const glm::vec2& rate)
        {
            m_pan += rate;
        }

        // Getters.

        inline const glm::vec2& GetOffset()             const { return m_pan;  }
        inline const glm::mat4& GetProjectionMatrix()   const { return m_proj; }
        inline const glm::mat4& GetViewMatrix()         const { return m_view; }
        inline const glm::mat4& GetCombinedMatrix()     const { return m_comb; }

        friend class zWindow;

    private:
        glm::mat4 m_proj, m_view, m_comb;
        glm::vec2 m_pan;
    };
}   // namespace gfx
}   // namespace zen

/** @} **/

#endif // ZENDERER__GRAPHICS__CAMERA_HPP
