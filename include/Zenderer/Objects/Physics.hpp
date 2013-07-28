/**
 * @file
 *  Zenderer/Objects/Physics.hpp - Integrates Box2D.
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

#include <Box2D/Box2D.h>

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Core/Subsystem.hpp"
#include "Entity.hpp"

#ifdef ZEN_DEBUG_BUILD
  #pragma comment(lib, "Box2D_DBG.lib")
#else
  #pragma comment(lib, "Box2D.lib")
#endif

namespace zen
{
namespace obj
{
    enum class BodyType : uint16_t
    {
        STATIC_BODY,
        DYNAMIC_BODY,
        KINEMATIC_BODY
    };

    class ZEN_API CPhysicsWorld : public CSubsystem
    {
    public:
        CPhysicsWorld(
            const math::vector_t& gravity = math::vector_t(0.0, 9.81),
            const uint16_t frame_rate = 60) :
                CSubsystem("Phyzx"), m_World(nullptr),
                m_Gravity(gravity.x, gravity.y),
                m_ticks(1.0 / frame_rate) {}

        ~CPhysicsWorld()
        {
            this->Destroy();
        }

        bool Init()
        {
            if(m_init) this->Destroy();

            m_World = new b2World(m_Gravity);
            m_init = true;
        }

        bool Destroy()
        {
            if(!m_init) return false;

            delete m_World;
            m_Gravity.x = 0.0;
            m_Gravity.y = 0.0;

            return !(m_init = false);
        }

        void Update()
        {
            m_World->Step(m_ticks, 4, 6);
        }

        void SetGravity(const math::vector_t& Gravity)
        {
            m_Gravity = b2Vec2(Gravity.x, Gravity.y);
        }

        b2World& GetWorld() const
        {
            return *m_World;
        }

        b2BodyDef& GetBody()
        {
            return m_Body;
        }

    private:
        b2World*    m_World;
        b2Vec2      m_Gravity;
        b2BodyDef   m_Body;
        real_t      m_ticks;
    };

    class ZEN_API CBody : public CEntity
    {
        CPhysicsWorld&  m_Phyz;
        b2World&        m_World;
        b2BodyDef*      mp_BodyDef;
        b2Body*         mp_Body;

    public:
        CBody(CPhysicsWorld& World, asset::CAssetManager& Assets) :
            CEntity(Assets), m_Phyz(World),
            m_World(World.GetWorld()),
            mp_BodyDef(nullptr) {}

        void SetType(const BodyType& Type)
        {
            b2BodyDef& Def = m_Phyz.GetBody();

            switch(Type)
            {
            case BodyType::STATIC_BODY:
                Def.type = b2_staticBody;
                break;

            case BodyType::DYNAMIC_BODY:
                Def.type = b2_dynamicBody;
                break;

            case BodyType::KINEMATIC_BODY:
                Def.type = b2_kinematicBody;
                break;
            }

            mp_BodyDef = &Def;
        }

        bool Create()
        {
            if(mp_BodyDef == nullptr) return false;
            mp_Body = m_World.CreateBody(mp_BodyDef);
        }
    };
}   // namespace obj
}   // namespace zen

#endif // ZENDERER__OBJECTS__PHYSICS_HPP
