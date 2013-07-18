/**
 * @file
 *  Zenderer/Objects/Physics.hpp - Defines the core-level physics structures
 *  that will be used for basic simulation of rigid body objects.
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
 * @addtogroup Objects
 * @{
 **/

#ifndef ZENDERER__OBJECTS__PHYSICS_HPP
#define ZENDERER__OBJECTS__PHYSICS_HPP

namespace zen
{
namespace obj
{
    /**
     * An axis-aligned bounding box (AABB) representation.
     *	This is used for basic quad collision detection throughout @a Zenderer
     *	using the separation of axis theorem. The two components are defined
     *	by the top-left and the bottom-right points of a standard, unrotated
     *	quad.
     *
     * @see http://gamedev.tutsplus.com/tutorials/implementation/collision-detection-with-the-separating-axis-theorem
     * @see http://gafferongames.com/game-physics/integration-basics/
     **/
    struct ZEN_API bbox_t
    {
	math::vector_t m_Min;
	math::vector_t m_Max;
    };

    /**
     * A representation of a circle to provide an alternative collision
     * in @a Zenderer for objects or sprites that don't play nicely with
     * quads.
     **/
    struct ZEN_API circle_t
    {
	real_t		m_radius;
	math::vector_t	m_Position;
    };

    struct material_t
    {
	real_t m_inv_mass;
    };	

    /// Checks for collision between two AABB boxes.
    /// @note	This algorithm doesn't work well with rotated boxes. 
    bool collides(const bbox_t& a, const bbox_t& b)
    {
	if(!compf(a.m_Max.z, b.m_Max.z)) return false;
	if(a.m_Max.x < b.m_Min.x || a.m_Min.x > b.m_Max.x) return false;
	if(a.m_Max.y < b.m_Min.y || a.m_Min.y > b.m_Max.y) return false;

	return true;
    }

    /// @overload
    bool collides(const circle_t& a, const circle_t& b)
    {
	real_t r = a.m_radius + b.m_radius;
	r *= r;
	return r < a.m_Position.distance(a, b, false);
    }
}
}

#endif // ZENDERER__OBJECTS__PHYSICS_HPP

