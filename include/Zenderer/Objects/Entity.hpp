/**
 * @file
 *  Zenderer/Objects/Entity.hpp - A generic entity class wrapping drawable
 *  primitives, materials, and easy sprite-like functionality together.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     2.0
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
 *  A group containing renderable objects with a variety of controllable 
 *  options and features like animation and collision detection.
 *  These classes are a step above the simple primitive rendering scheme
 *  set up by the architecture in zen::gfxcore::CDrawable, and are the most
 *  likely to face direct user interaction through the application.
 *
 * @{
 **/

#ifndef ZENDERER__OBJECTS__ENTITY_HPP
#define ZENDERER__OBJECTS__ENTITY_HPP

#include "Zenderer/Math/Math.hpp"
#include "Zenderer/CoreGraphics/Drawable.hpp"
#include "Zenderer/Graphics/Material.hpp"
#include "Zenderer/Graphics/Quad.hpp"
#include "Zenderer/Utilities/INIParser.hpp"

namespace zen
{
namespace gfx { class ZEN_API CScene; }

/// A group of sprite-like objects that operate at a high level of abstraction.
namespace obj
{
    /// A base class for all "in-game" objects.
    class ZEN_API CEntity
    {
    protected:
        // Internal error types.
        enum class ErrorType : uint16_t
        {
            BAD_PAIR,
            BAD_POSITION,
            NO_TEXTURE
        };
        
    public:
        CEntity();
        virtual ~CEntity();

        /// @todo   Move semantics on `util::split()`
        bool LoadFromFile(const string_t& filename)
        {
            ZEN_ASSERT(!filename.empty());
            
            util::CINIParser Parser;
            std::ifstream file(filename);
            std::string line;
            uint32_t line_no = 0;
            
            if(!file) return false;
            
            gfx::CQuad* pPrim = nullptr;
            
            while(std::getline(file, line))
            {
                ++line_no;
                if(line.empty() || line[0] == '/') continue;
                
                if(line.find("position") != std::string::npos)
                {
                    std::vector<string_t> pair = util::split(line, '=');
                    if(pair.size() != 2) return this->FileError(filename, line, line_no);
                    
                    pair = util::split(line, ',');
                    if(pair.size() < 2)
                        return this->FileError(filename, line, line_no, ErrorType::BAD_POSITION);
                    
                    this->Move(std::stod(pairs[0]), std::stod(pairs[1]);
                    
                    // Depth is optional
                    if(pair.size() == 3) m_Position.z = std::stod(pairs[2]);
                }
                
                else if(line.find("<prim>") != std::string::npos)
                {
                    if(pPrim != nullptr) mp_allPrims.push_back(pPrim);
                    
                    pPrim = new gfx::CQuad(0, 0);
                    std::streampos start = file.tellg();
                    
                    // Find end of primitive block.
                    while(std::getline(file, line) &&
                          line.find("</prim>") == std::string::npos);
                    
                    std::streampos end = file.tellg();
                    
                    Parser.LoadFromStream(file, start, end, filename.c_str());
                    
                    // We have loaded key=value pairs for a primitive instance.
                    if(!Parser.Exists("texture"))
                        return this->FileError(filename, line, line_no, ErrorType::NO_TEXTURE);
                        
                    gfx::CMaterial* pMat = new gfx::CMaterial(m_Assets);
                    pMat->LoadFromStream(file, start, end);
                    
                    if(Parser.Exists("width") && Parser.Exists("height"))
                        pPrim->Resize(Parser.GetValuei("width"), Parser.GetValuei("height"));
                    else
                        pPrim->Resize(pMat->GetTexture()->GetWidth(),
                                      pMat->GetTexture()->GetHeight());

                    if(Parser.Exists("invert")) pPrim->SetInvertible(Parser.GetValueb("invert"));
                    if(Parser.Exists("repeat")) pPrim->SetRepeatable(Parser.GetValueb("repeat"));
                }
            }
            
            m_filename = filename;
            return true;
        }
        
        bool LoadFromTexture(const string_t& filename);
        bool AddPrimitive(const CQuad& Prim);
        bool Create();
        bool Draw(bool is_bound = false);
        
    protected:        
        bool FileError(const string_t& filename, 
                       const string_t& line, const uint32_t line_no,
                       const ErrorType& Err = ErrorType::BAD_PAIR)
        {
            m_Log << m_Log.SetMode(LogMode::ZEN_ERROR) << m_Log.SetSystem("Entity")
                  << "Error while parsing '" << filename << "' on line " << line_no
                  << ": " << line << "(";

            switch(Err)
            {
            case ErrorType::BAD_PAIR:
                m_Log << "bad key=value pair";
                break;
                
            case ErrorType::BAD_POSITION:
                m_Log << "position must at least contain x,y coordinates";
                break;
                
            case ErrorType::NO_TEXTURE:
                m_Log << "no texture specified for primitive";
                break;
                
            default:
                m_Log << "parsing error";
                break;
            }
            
            m_Log << ")." << CLog::endl;
            return false;
        }
        
        string_t m_filename;
        std::vector<CQuad*> mp_allPrims;
    };

}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__OBJECTS__ENTITY_HPP

/** @} **/

/** 
 * @class zen::obj::CEntity
 * @description
 *  These differ from generic primitives in the sense that they have a lot more
 *  functionality abstracted away, as well as additional functions like
 *  animation and physics reactions (in inheriting classes). They do use the
 *  zen::gfx::CQuad class at their core, but function at a much higher level.
 *
 *  They can also be loaded from files (see the spec) and can 
 *  contain multiple primitive instances.
 *
 *  When creating an entity from multiple primitives, a copy of the vertex/index
 *  data will be stored internally in order to prevent a bad reference later.
 *
 *  The call to `Optimize()` is not necessary, but is recommended to merge
 *  identical primitives together if they use the same material.
 *
 * @see specs.html#ZEnt
 **/