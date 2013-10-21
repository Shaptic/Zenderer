/**
 * @file
 *  Zenderer/CoreGraphics/ConcavePolygon.hpp - A vertex-agnostic concave
 *  primitive.
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
 * @addtogroup Graphics
 * @{
 **/

#ifndef ZENDERER__GRAPHICS__CONCAVE_POLYGON_HPP
#define ZENDERER__GRAPHICS__CONCAVE_POLYGON_HPP

#include "Polygon.hpp"

namespace zen
{
namespace gfx
{
    /// An arbitrary n-vertex concave polygon.
    class ZEN_API zConcavePolygon : public zPolygon
    {
    public:
        zConcavePolygon(asset::zAssetManager& Assets, const size_t preload=0);
        ~zConcavePolygon();

        virtual zPolygon& Create(const bool do_triangulation = false);

        /// For setting things implicitly.
        friend class ZEN_API obj::zEntity;
    };
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__CONCAVE_POLYGON_HPP

/**
 * @class zen::gfx::zConcavePolygon
 * @details
 *  This is a class representing an arbitrary concave polygon that can
 *  accept any number of vertices. The polygon uses triangulation to determine
 *  appropriate indices for rendering the vertices in the intended shape.
 **/

/** @} **/
