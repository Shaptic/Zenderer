/**
 * @class       zen::gfx::CSceneManager
 *
 * @description
 * @section     sort    Optimized Sorting Algorithm
 * @subsection  expl    Explanation
 * 
 * In order to maximize rendering efficiency, it is important to
 * make as little state changes as possible to the underlying 
 * rendering API. For this, @a Zenderer implements a high-speed
 * sorting technique that operates at maximum performance. 
 * 
 * Essentially, each entity internally stores a 32-bit integer
 * consisted of various state flags describing material usage, 
 * depth, transparency, and other data.
 * 
 * This state flag is used for comparing the scene graph prior
 * to rendering in order to, for example, group all entities with
 * identical materials together in order to minimize texture swaps.
 *
 * A material is defined as follows:
 *
 *     material = texture + vshader + fshader
 *
 * And a thourough explanation of the state bit definitions are ahead.
 *
 * We expect to have no more than 4096 materials in a single game. This
 * can, of course, be easily expanded, but 4096 is more than enough for
 * a standard 2D game. For this, we allocate 12 bits in the flag for
 * storing a unique material ID (2<sup>12</sup> = 4096).
 *
 * Thus: 32 - 12 = 20 bits remaining.
 *
 * For depth information, we allocate 8 bits, giving us 256 different
 * levels of depth for any entity in the game. Again this is perfectly
 * acceptable for a 2D game.
 *
 * Thus: 20 - 8 = 12 bits remaining.
 * 
 * For transparency, we only need a single bit. to compare if a 
 * 
 * Thus: 12 - 1 = 11 bits remaining.
 *
 * The remaining bits are reserved for any future sorting requirements.
 * 11 bits gives 2048 different values, surely large enough to accomodate
 * any future requirements.
 *
 * @subsection  algo    Sorting
 * The actual sorting process works as follows:
 *
 * The scene stores a list of entities for rendering. It also contains 
 * an internal dictionary associating the various sorting parameters
 * (material, depth) with a list of entities that use that parameter.
 * 
 * On addition of an entity from the scene, it's inserted into the 
 * appropriate list based on its internal flags.
 **/
 
/*
 
using namespace zen;
using namespace gfxcore;

void CSceneManager::SortByMaterial(
    const std::vector<ent::CEntity*>& allObjects,
    std::map<uint32_t, std::list<ent::CEntity*>>& groupedObjects)
{
    auto start = groupedObjects.begin(), end = groupedObjects.end();
    
    // Iterate over each material and check that the entities for
    // this material haven't changed. If they have, insert them into
    // the list for their proper material.
    for( ; start != end; ++start)
    {
        auto i = start->second.begin(), j = start->second.end();
        for( ; i != j; )
        {
            auto id = CSorter::ExtractMaterialID(*i);
            
            // We are already in the right place.
            if(start->first == id)
            {
                ++i;
                continue;
            }
            
            auto finder = groupedObjects.find(id);            
            if(finder == groupedObjects.end())
            {
                groupedObjects[id] = std::list<ent::CEntity*>>(1, *i);
            }
            else
            {
                finder->second.push_back(*i);
            }
            
            i = start->second.erase(i);
        }
    }
}

void CSceneManager::SortByDepth(
    const std::vector<ent::CEntity*>& allObjects,
    std::map<uint32_t, std::list<ent::CEntity*>>& groupedObjects)
{
    auto start = groupedObjects.begin(), end = groupedObjects.end();
    
    // Iterate over each material and check that the entities for
    // this material haven't changed. If they have, insert them into
    // the list for their proper material.
    for( ; start != end; ++start)
    {
        auto i = start->second.begin(), j = start->second.end();
        for( ; i != j; )
        {
            material_t::id_t id = CSorter::ExtractDepth(*i);
            
            // We are already in the right place.
            if(start->first == id)
            {
                ++i;
                continue;
            }
            
            // Does such a place exist?
            auto finder = groupedObjects.find(id);
            
            // It likely will...
            if(finder != groupedObjects.end())
            {
                finder->second.push_back(*i);
            }
            // But if not, make it so.
            else
            {
                groupedObjects[id] = std::list<ent::CEntity*>>(1, *i);
            }
            
            // Remove from the current list.
            i = start->second.erase(i);
        }
    }
}

void CScene::Render()
{
    std::map<uint32_t,  std::vector<ent::CEntity*>> m_materialObjects;
    std::map<int16_t,   std::vector<ent::CEntity*>> m_depthObjects;
    std::map<bool,      std::vector<ent::CEntity*>> m_alphaObjects;
    
    this->Manager.SortByMaterial(mp_sceneObjects, m_materialObjects);
    this->Manager.SortByAlpha(mp_sceneObjects, m_alphaObjects);
    
    if(m_alphaObjects[true].size() > 0)
    {
        this->Manager.SortByDepth(m_alphaObjects[true]);
        this->Manager.PrepareAlphaRenderState();
            
        auto k = m_alphaObjects[true].begin(),
             l = m_alphaObjects[true].end();
        for( ; k != l; ++k)
        {
            this->RenderEntity(*k);
        }
        
        this->Manager.PrepareRenderState();
        
        k = m_alphaObjects[false].begin();
        l = m_alphaObjects[false].end();
        for( ; k != l; ++k)
        {
            this->RenderEntity(*k);
        }
    }
    
    auto i = m_materialObjects.begin(), j = m_materialObjects.end();
    for( ; i != j; ++i)
    {
        i->first->Prepare();
        auto k = i->second.begin(), l = i->second.end();
        for( ; k != l; ++k)
        {
            this->RenderEntity(*k);
        }
    }
    
    this->Manager.ResetRenderState();
}