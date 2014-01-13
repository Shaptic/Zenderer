/**
 * @file
 *  Zenderer/Assets/AssetManager.hpp - A container for all in-engine assets
 *  like shaders, textures, and audio files.
 *
 * @author      George (@_Shaptic)
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
 * @addtogroup Assets
 * @{
 **/

#ifndef ZENDERER__ASSETS__ASSET_MANAGER_HPP
#define ZENDERER__ASSETS__ASSET_MANAGER_HPP

#include <list>

#include "Zenderer/Core/Subsystem.hpp"
#include "Asset.hpp"

namespace zen
{
namespace asset
{
    /// Controls creation and proper memory management of assets.
    class ZEN_API zAssetManager : public zSubsystem
    {
    public:
        zAssetManager();
        virtual ~zAssetManager();

        /// Does nothing.
        bool Init();

        /// Destroys and deletes all assets connected to this manager.
        bool Destroy();

        /**
         * Creates an asset from a filename.
         *  Assets are set up on the zen::asset::zAsset base class, which
         *  guarantees the ability to load from a file. That's what this
         *  method will do: load an asset from a file. Since assets may be
         *  identical but claimed by different parts of the engine, such as
         *  vertex buffer and mesh data for unique scenes, it is possible
         *  to specify an "owner" for the asset. This is entirely optional,
         *  and the asset will default to having no unique owner.
         *  Assets created in different instances of this class are also
         *  considered to be completely separate.
         *
         * @tparam  T           An `asset::zAsset` inheriting class type
         * @param   filename    Filename to load asset from
         * @param   owner       Address of asset owner      (optional=`nullptr`)
         * @param   settings    Asset settings (if any)     (optional=`nullptr`)
         *
         * @return  A dynamically created asset if it loaded successfully, and
         *          `nullptr` otherwise.
         **/
        template<typename T>
        T* Create(const string_t& filename, const void* const owner = nullptr,
                  const void* const settings = nullptr);

        /// @overload
        template<typename T>
        T* Create(const char* const filename, const void* const owner = nullptr,
                  const void* const settings = nullptr);

        /**
         * Creates a raw managed asset instance (unloaded).
         *
         * @tparam  T       An `asset::zAsset` inheriting class type
         * @param   owner   The asset's "owner"             (optional=`nullptr`)
         * @param   settings    Asset settings (if any)     (optional=`nullptr`)
         *
         * @return  A reference to newly created asset of type `T`.
         **/
        template<typename T>
        T& Create(const void* const owner       = nullptr,
                  const void* const settings    = nullptr);

        /**
         * Creates a copy of an existing asset.
         *  Since assets are set up such that there is only one existing
         *  copy at a time, calling
         *
         *  @code
         *      Create<gfxcore::zTexture>("test.tga");
         *  @endcode
         *
         *  three times will give the same return value every time.
         *  Sometimes, though, it is desirable to have multiple copies of
         *  an asset, and this method will do so.
         *
         * @tparam  T           A child class of asset::zAsset
         * @param   Copier      Assets to copy (filename or object)
         * @param   owner       Address of asset owner      (optional=`nullptr`)
         * @param   settings    Asset settings (if any)     (optional=`nullptr`)
         *
         * @return  A dynamically created asset if it copies successfully, and
         *          `nullptr` otherwise.
         *
         * @warning For the object `Recreate()` method, this relies on the `T`
         *          object to have implemented `zAsset::LoadFromExisting()`,
         *          which is not guaranteed.
         **/
        template<typename T>
        T* Recreate(const T* const Copier,
                    const void* const owner    = nullptr,
                    const void* const settings = nullptr);

        template<typename T>
        T* Recreate(const string_t& Copier,
                    const void* const owner    = nullptr,
                    const void* const settings = nullptr);  ///< @overload

        template<typename T>
        T* Recreate(const char* Copier,
                    const void* const owner    = nullptr,
                    const void* const settings = nullptr);  ///< @overload

        /**
         * Permanently deletes and removes an asset.
         * @param   pAsset  Asset to remove
         * @return  `true` if it was removed successfully, `false` otherwise.
         **/
        bool Delete(zAsset* pAsset);

        /// @overload Delete(zAsset* const pAsset);
        bool Delete(const uint32_t index);

        /// Finds an asset by filename, if it exists.
        zAsset* Find(const string_t& filename,
                     const void* const owner = nullptr) const;

        /// Finds an asset by ID, if it exists.
        /// @overload
        zAsset* Find(const assetid_t id) const;

        /// Returns the amount of assets this manager has.
        inline uint32_t GetAssetCount() const;

        /// Returns an iterator to the first asset.
        inline std::list<zAsset*>::const_iterator cbegin();

        /// Returns an iterator to the end of the asset list.
        inline std::list<zAsset*>::const_iterator cend();

        /// Returns the amount of total assets @a Zenderer is aware of.
        static uint32_t GetGlobalAssetCount();

    private:
        /// Adds to internal lists and logs data appropriately.
        template<typename T>
        T* FinalizeAsset(const bool flag, T* pAsset);

        static std::list<zAsset*> sp_allAssets;

        util::zLog& m_Log;
        std::list<zAsset*> mp_managerAssets;
    };

    #include "AssetManager.inl"
}   // namespace asset
}   // namespace zen

#endif // ZENDERER__ASSETS__ASSET_MANAGER_HPP

/**
 * @class zen::asset::zAssetManager
 * @details
 *  To be an asset instance supported by this manager, the
 *  object must inherit from zen::asset::zAsset.
 **/

/**
 * @fn zen::asset::zAssetManager::Find()
 * @details
 *  Technical details ahead:    \n
 *
 *  @warning    The benchmark info is out-dated and inaccurate. Range-based
 *              `for()` loops outperform all other methods now and are used
 *              throughout @a Zenderer where applicable. The metrics are kept
 *              purely for historical measurement purposes.
 *
 *  @section search     Search
 *  @subsection perf    Performance Comparison
 *
 *  Finding assets may occur during runtime and must thus be optimized for
 *  speed as much as possible. During a thorough test of STL containers and
 *  their varied methods of access, I've come to the conclusion that
 *  hand-rolled iterator loops outperforms everything else.
 *
 *  In a test of 100,000,000 (100 million) iterations over an `std::vector<char>`,
 *  benchmarks indicated the following:
 *
 *  @code
 *  // Two vectors to prevent any issues with caching.
 *  const size_t BIGNUM = 100000000;
 *  std::vector<char> A, B;
 *
 *  for(size_t i = 0; i < BIGNUM; ++i)
 *  {
 *    A.push_back(i);
 *    B.push_back(i);
 *  }
 *  @endcode
 *
 *   Code                                                                                                   |  Access Type              | Speed
 *  :-------------------------------------------------------------------------------------------------------| :-----------------------: | :---:
 *  <pre>for(size_t i = 0; i < BIGNUM; ++i) ++A[i];</pre>                                                   | `operator[]`              | 191ms
 *  <pre>for(auto& i : B)  ++i</pre>                                                                        | Range-Based `for()` loop  | 139ms
 *  <pre>auto i = A.begin(), j = A.end();<br>for( ; i != j; ++i) ++(*i)</pre>                               | Optimized `iterator`      | 107ms
 *  <pre>for(auto i = B.begin(); i != B.end(); ++i) ++(*i)</pre>                                            | Standard `iterator`       | 112ms
 *  <pre>for(size_t i = 0; i < BIGNUM; ++i) ++A.at(i) </pre>                                                | `std::vector<T>::at()`    | 309ms
 *  <pre>for(auto i = B.begin(); i != B.end(); i++) ++(*i); </pre>                                          | `iterator++`              | 416ms
 *  <pre>char* end = &A[A.size() - 1];<br>for(char* i = &A[0]; i != end; i += sizeof(char)) ++(*c); </pre>  | Pointer arithmetic        | 108ms
 **/

/** @} **/
