/**
 * @file
 *  Zenderer/Graphics/Triangulate.hpp - A utility to triangulate drawable
 *  polygon primitives.
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
 *
 * @see http://abitwise.blogspot.com/2013/09/triangulating-concave-and-convex.html
 * @see http://gist.github.com/Ruskiy69/6526805
 **/

#ifndef ZENDERER__GRAPHICS__TRIANGULATION_HPP
#define ZENDERER__GRAPHICS__TRIANGULATION_HPP

#include "Zenderer/Math/Shapes.hpp"
#include "Polygon.hpp"

namespace zen
{
namespace gfx
{
    /// @return `true` if clockwise, `false` if counter-clockwise.
    bool orientation(const std::vector<math::vector_t>& Polygon)
    {
        ZEN_ASSERTM(Polygon.size() >= 3, "not a polygon");
        
        uint16_t count = 0;
        for(uint16_t i = 0; i < Polygon.size(); ++i)
        {
            uint16_t j = (i + 1 < Polygon.size()) ? i + 1 : 0;
            uint16_t k = (i + 2 < Polygon.size()) ? i + 2 : 1;
            real_t z = (Polygon[j].x - Polygon[i].x) * (Polygon[k].y - Polygon[j].y)
                     - (Polygon[j].y - Polygon[i].y) * (Polygon[k].x - Polygon[j].x);
            
            count += (z > 0) ? 1 : -1;
        }
        
        ZEN_ASSERTM(count != 0, "not a simple polygon");
        return (count > 0);
    }
    
    bool orientation(const math::tri_t& Triangle)
    {
        return (Triangle[1].x - Triangle[0].x) * (Triangle[2].y - Triangle[0].y) -
               (Triangle[2].x - Triangle[0].x) * (Triangle[1].y - Triangle[0].y);
    }
    
    bool triangle_test(const vector_t& V, const math::tri_t& T)
    {
        real_t denom = (T[1].y - T[2].y) * (T[0].x - T[2].x)
                     + (T[2].x - T[1].x) * (T[0].y - T[2].y);

        // Avoid division by zero.
        if(math::compf(denom, 0.0)) return true;
        denom = 1.0 / denom;
        
        real_t alpha = denom * ((T[1].y - T[2].y) * (V.x - T[2].x) +
                                (T[2].x - T[1].x) * (V.y - T[2].y));
        if(alpha < 0) return false;
     
        real_t beta  = denom * ((T[2].y - T[0].y) * (V.x - T[2].x) +
                                (T[0].x - T[2].x) * (V.y - T[2].y));

        return (beta > 0 || alpha + beta >= 1);
    }
    
    std::vector<math::vector_t> triangulate(std::vector<math::vector_t> Polygon)
    {
        std::vector<uint16_t> reflex;
        std::vector<vector_t> triangles;    
        
        // Determine entire polygon orientation
        bool ort = orientation(Polygon);
        
        // We know there will be vertex_count - 2 triangles made.
        triangles.reserve(Polygon.size() - 2);
     
        if(Polygon.size() == 3) return Polygon;
        while(Polygon.size() >= 3)
        {
            reflex.clear();
            int16_t eartip = -1, index = -1;
            math::tri_t tri;

            for(auto& i : Polygon)
            {
                ++index;
                if(eartip >= 0) break;
                
                uint16_t p = (index > 0) ? index - 1 : Polygon.size() - 1;
                uint16_t n = (index < Polygon.size()) ? index + 1 : 0;
                
                tri[0] = Polygon[p]; tri[1] = i; tri[2] = Polygon[n];
                
                if(orientation(tri) != ort)
                {
                    reflex.emplace_back(index);
                    continue;
                }
                
                bool ear = true;
                for(auto& j : reflex)
                {
                    if(j == p || j == n) continue;
                    if(triangle_test(Polygon[j], tri))
                    {
                        ear = false;
                        break;
                    }
                }
                
                if(ear)
                {
                    auto j = Polygon.begin() + index + 1,
                         k = Polygon.end();
                     
                    for( ; j != k; ++j)
                    {
                        auto& v = *j;
                        if(&v == &Polygon[p] || &v == &Polygon[n]) continue;
                        if(in_triangle(v, tri))
                        {
                            ear = false;
                            break;
                        }
                    }
                }
                
                if(ear) eartip = index;
            }
            
            if(eartip < 0) break;
            
            // Create the triangulated piece.
            for(const auto& i : tri) triangles.push_back(i);
            
            // Clip the ear from the polygon.
            Polygon.erase(std::find(Polygon.begin(), Polygon.end(), tri[1]));
        }
        
        return triangles;
    }
    
}   // namespace gfx
}   // namespace zen

#endif // ZENDERER__GRAPHICS__TRIANGULATION_HPP

/**
 * @fn  zen::gfx::triangulate
 * @details
 *  Triangles are a low-level drawing primitive provided for convenience
 *  to users of @a Zenderer. They are not actually used in any part of the
 *  engine other than the triangulation module and collision detection.
 **/

/** @} **/
