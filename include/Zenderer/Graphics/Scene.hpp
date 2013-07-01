#include "Zenderer/CoreGraphics/VertexArray.hpp"
#include "Zenderer/CoreGraphics/Renderer.hpp"
#include "Zenderer/CoreGraphics/Sorter.hpp"

#include "Window.hpp"
#include "Quad.hpp"
#include "Effect.hpp"
#include "Light.hpp"

namespace zen
{
namespace gfx
{    
    class ZEN_API CScene
    {
    public:
        CScene();
        ~CScene();
        
        /**
         * Initializes internal graphical components.
         **/
        bool Init();
        
        //CEntity& AddEntity();

        /**
         * Adds a managed primitive to the scene.
         *  No initialization or setting of parameters is done on the
         *  returned instance, it's merely added to the internal list of
         *  primitives in order to clean them up appropriately and draw
         *  them together in the rendering sequence, and `Create()` is
         *  called, since it takes no parameters and is a default.
         *
         *  We return the value of `Create()` to avoid having the user work
         *  with pointers.
         *
         * @return  A `CDrawable&` instance of whatever type `T` you specified.
         **/
        template<typename T>
        T& AddPrimitive()
        {
            T* pNew = new T;
            m_allPrimitives.push_back(pNew);
            return m_allPrimitives.back()->Create();
        }
        
        /** 
         * Inserts a primitive at a point in the draw queue.
         *  Sometimes, you need to have something drawn in a different order
         *  than you had originally planned. Thus this method allows you to 
         *  insert primitives at any point in the draw queue. This operation
         *  is `O(1)` thanks to `std::list`, so performance worries are
         *  non-existant.
         *
         * @param   index   The index to insert a primitive at
         *
         * @return  A `CDrawable&` instance of whatever type `T` you specified.
         *
         * @warning The index MUST be in the range of the current list, or
         *          an exception will (likely) be thrown. The range cannot be
         *          checked because this method only returns a valid reference.
         *          Use `IsValidPrimitiveIndex()` to check for a good index.
         *
         * @see     GetPrimitiveIndex()
         * @see     IsValidPrimitiveIndex()
         **/
        template<typename T>
        T& InsertPrimitive(const uint32_t index)
        {
            T* pNew = new T;
            T& Ret = pNew->Create();
            m_allPrimitives.insert(pNew, index);
            return Ret;
        }
        
        int32_t GetPrimitiveIndex(const gfxcore::Drawable& D)
        {
            auto i = m_allPrimitives.begin(),
                 j = m_allPrimitives.end();
            
            int32_t index = -1;
            for( ; i != j; ++i, ++index)
            {
                if((*i) == &D) return index;
            }
            
            return index;
        }
        
        bool IsValidPrimitiveIndex(int32_t i)
        {
            return (i > 0 && i < m_allPrimitives.size());
        }
        
    private:
        gfxcore::CVertexArray   m_Geometry;
        gfx::CRenderTarget      m_FBO1, m_FBO2;
        
        // Lists of things that will be rendered.
        std::list<CLight*>      m_allLights;
        std::list<CEffect*>     m_allEffects;        
        std::list<CDrawable*>   m_allPrimitives;
        //std::list<CEntity*> m_allEntities;
    };
}
}
