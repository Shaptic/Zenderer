/**
 * @file
 *  Zenderer/Scripting/Lua.hpp - A wrapper around core Lua API interactions
 *  and embedding.
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
 * @addtogroup Scripting
 *  This module contains convenience functions and wrappers for embedding and
 *  communicating with Lua through the C API.
 *
 * @{
 **/

#ifndef ZENDERER__SCRIPTING__LUA_HPP
#define ZENDERER__SCRIPTING__LUA_HPP

#include "lua/lua.hpp"

#ifdef _WIN32
  #pragma comment(lib, "lua52.lib")
#endif

namespace zen
{

/// An abstraction layer over raw embedding of the Lua scripting environment.
namespace lua
{
    /// A shortcut for easily defining the `name -> loader` set.
    typedef std::map<string_t, lua_CFunction> luaLibs_t;
    
    /// Wrapper for the core Lua interpreter.
    class ZEN_API CLuaAPI : public CSubsystem
    {
    public:
        /**
         * Constructs a Lua interpreter instance.
         *  This will set up the configuration for the private Lua
         *  interpreter instance, storing the necessary libraries
         *  locally.
         *
         * @param   libraries   Libraries to use
         **/
        CLuaAPI(const luaLibs_t& libraries) : 
            m_Libs(libraries), m_Lua(nullptr) {}
            
        /// Cleans up the interpreter instance.
        ~CLuaAPI()
        {
            this->Destroy();
        }
        
        /// Implicitly covert to `lua_State*` for `lua_` API calls.
        inline operator lua_State*() { return m_Lua; }
        
        /**
         * Loads the Lua interpreter with the given libraries.
         * @return  `true` if loaded successfully,
         *          `false` if already initialized.
         *
         * @see     IsInit()
         **/
        bool Init()
        {
            if(!m_init)
            {
                m_Lua = luaL_newstate();
                ZEN_ASSERT(m_Lua != nullptr);
                
                for(auto& i : m_Libs)
                {
                    luaL_requiref(m_Lua, i.first, i.second, 1);
                    lua_settop(m_Lua, 0);
                }
                
                return (m_init = true);
            }
            
            return false;
        }
        
        /// Cleans up the Lua interpreter instance.
        bool Destroy()
        {
            if(!m_init) return false;
            
            lua_close(m_Lua);
            m_Lua = nullptr;
            return true;
        }
        
        /**
         * Loads and executes a Lua script.
         *  This will execute the Lua script just as if you ran it 
         *  from the command line using `lua filename`. Any function
         *  calls and other interactions can only be done after this
         *  has been run, but the setting of any variables must be
         *  done beforehand.
         *
         * @param   filename    Lua script path / filename
         *
         * @return  `true`  if the script ran without errors,
         *          `false` otherwise.
         *
         * @see     GetError()
         **/
        bool LoadFile(const string_t& filename)
        {
            return lua_loadfile(m_Lua, filename.c_str()) == 0 && 
                   lua_pcall(m_Lua, 0, LUA_MULTRET, 0) == 0;
        }
        
        /// Empties the Lua virtual stack 
        inline bool ResetStack()
        {
            if(m_init) lua_settop(m_Lua, 0);
            return m_init;
        }
        
        /**
         * Retrieves the latest Lua error, if any.
         *
         * @return  The error string if it exists, a custom error
         *          if the interpreter has not been loaded, and
         *          a blank string if there is no error.
         *
         * @warning This should only be called if you are absolutely 
         *          positive that the interpreter has generated an
         *          error, otherwise this may mess up the stack or
         *          conversion may fail.
         **/
        inline string_t GetError()
        {
            if(!m_init) return string_t("Lua interpreter not loaded");
            return string_t(lua_tostring(m_Lua, -1));
        }
        
    private:
        const luaLibs_t& m_Libraries;
        lua_State* m_Lua;
    };
}
}

#endif // ZENDERER__SCRIPTING__LUA_HPP

/** @} **/

/**
 * @class zen::lua::CLuaAPI
 *
 * @details
 *  This class provides a simple convenience wrapper around the `lua_State`
 *  variable required for the Lua intepreter. It automatically takes care
 *  of resource allocation and freeing of the interpreter instance.
 *  Scripting is merely a feature of the engine, and is not actually used
 *  in its core (yet). Thus, the file is not included by default in
 *  @ref Zenderer.hpp. If you wish to include it, be sure to add the 
 *  appropriate linker commands to your compilation routine.
 *
 * @note    By default, the Visual Studio 11 library is linked when `_WIN32`
 *          is defined.
 *
 * @see     http://www.lua.org/manual/5.2/manual.html
 * @see     http://blog.acamara.es/2012/08/19/calling-c-functions-from-lua-5-2/
 * @see     http://csl.name/lua/
 *
 * @example Scripting
 * 
 * @section usage       Lua Usage Examples
 * @subsection creation Instantiating a Lua wrapper
 *  This will create a Lua wrapper with some very barebones Lua libraries.
 *
 *  @code
 *  using namespace zen;
 *
 *  lua::luaLibs_t libs;
 *  libs["io"]  = lua_loadio;
 *  libs["base"]= lua_loadbase;
 *
 *  lua::CLua API(libs);
 *  @endcode
 *
 * @subsection obj Passing objects to Lua
 *  There comes a time where it's necessary to pass objects around to Lua,
 *  especially if Lua is referring back to a C function in your code. This
 *  will demonstrate a simple, flexible way to pass around the `this` pointer
 *  to the Lua interpreter, and back again.
 *
 *  Here is our example class.
 *  @code
 *  #include <iostream>
 *
 *  class Dummy
 *  {
 *      int m_f;
 *
 *  public:
 *      explicit Dummy(const int factor) : m_f(factor) {}
 *      inline int GetF() const
 *      {
 *          return m_f;
 *      }
 *
 *      inline void Talk()
 *      {
 *          std::cout << "Dummies can't talk!\n";
 *      }
 *  };
 *  @endcode
 *
 *  Here is our C function that will interact with the object. Let's assume
 *  that Lua gives us 2 numbers to add, and we return a value scaled with 
 *  the `m_f` variable in the given object, and the 2 numbers added together.
 *
 *  @code
 *  int dummy_function(lua_State* Lua)
 *  {
 *      // Verify the proper amount of args.
 *      int argc = lua_gettop(Lua);
 *      if (argc < 3) 
 *      {
 *          lua_pushstring(Lua, "Invalid argument count.");
 *          lua_pushnumber(Lua, 0);
 *          return 2;
 *      }
 *
 *      int a = lua_tonumber(Lua, 1);
 *      int b = lua_tonumber(Lua, 2);
 *      Dummy* pObject = static_cast<Dummy*>(lua_touserdata(Lua, 3));
 *
 *      lua_pushnumber(a * b * pObject->GetF());
 *      lua_pushnumber(a + b);
 *      return 2;
 *  }
 *  @endcode
 *
 *  Here is our Lua code, which just passes some arbitrary values to the
 *  C function (bound later).
 *
 *  @code{.lua}
 *  -- Example.lua
 *  local number1 = 55
 *  local number2 = 69
 *
 *  a, b = dummy_function()
 *  f, s = dummy_function(number1, number2, this)
 *
 *  print('Factors: ' .. a .. ', ' .. f)
 *  print('Sum: ' .. b .. ', ' .. s)
 *  @endcode
 *
 *  And finally, here is the host-side C++ code that will bind the Lua 
 *  data appropriately.
 *
 *  @code
 *  #include "Zenderer/Core/Types.hpp"
 *  #include "Zenderer/Scripting/Lua.hpp"
 *
 *  using namespace zen;
 *
 *  luaLibs_t libs;
 *  libs["base"] = luaopen_base;
 *
 *  lua::CLuaAPI Lua(libs);
 *  Lua.Init();
 *
 *  Dummy Object(24);
 *
 *  lua_pushuserdata(Lua, static_cast<void*>(&Dummy));
 *  lua_setglobal(Lua, "this");
 *  lua_register(Lua, "dummy_function", dummy_function);
 *
 *  Lua.LoadFile("Sample.lua");
 *  @endcode
 *
 *  If all is well, the output of the program would be:
 *  @code{.txt}
 *  Invalid argument count., 91080
 *  0, 124
 *  @endcode
 **/
