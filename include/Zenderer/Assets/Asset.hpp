/**
 * @file
 *  Zenderer/Assets/Asset.hpp - An abstract base class used by all managed
 *  assets in @a Zenderer.
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
 * @addtogroup Assets
 *  A subsystem for creating, re-using, finding, and removing managed assets
 *  in an optimized way, keeping creation to a minimum.
 *
 * @{
 **/

#ifndef ZENDERER__ASSETS__ASSET_HPP
#define ZENDERER__ASSETS__ASSET_HPP

#include "Zenderer/Core/Types.hpp"

#include "Zenderer/Utilities/Assert.hpp"
#include "Zenderer/Utilities/Log.hpp"
#include "Zenderer/Utilities/Helpers.hpp"

namespace zen
{

/// A namespace encompassing asset-related objects and managers.
namespace asset
{
    /// For unique asset ID.
    typedef uint32_t assetid_t;

    /// Used throughout @a Zenderer to act as a managed asset.
    class ZEN_API CAsset
    {
    public:
        /// Logs some information.
        virtual ~CAsset();

        /**
         * Loads an asset from the disk.
         * @param   filename    Filename to load asset from
         * @return `true` if loaded successfully, `false` otherwise.
         **/
        virtual bool LoadFromFile(const string_t& filename) = 0;

        /**
         * Copies an asset from another.
         *  By default, this just copies all of the asset metadata over,
         *  excluding any implementation-specific data.
         *  It's recommended to overload this method, and then just call
         *  CAsset::LoadFromExisting() again internally to ensure all
         *  metadata is copied (if that's desired).
         *
         * @param   pCopy   Asset to create a copy from
         *
         * @return `true` if copied successfully, `false` otherwise.
         **/
        virtual bool LoadFromExisting(const CAsset* const pCopy);

        /// Returns the hashed filename for quicker comparison.
        inline uint32_t GetFilenameHash() const;

        /// Returns the unique asset ID.
        inline assetid_t GetID() const;

        /// Returns the filename used to load the asset.
        inline const string_t& GetFilename() const;

        /// Returns raw asset data (usually for reloading).
        virtual const void* const GetData() const = 0;

        /// Returns address of asset owner.
        inline const void* const GetOwner() const;

        /// Returns the last error string.
        inline const string_t& GetError() const;

        /// Has LoadFromFile() been called successfully?
        inline bool IsLoaded() const;

        /// Sets the asset owner.
        inline void SetOwner(const void* const owner);

        /// Overrides the asset filename.
        inline void SetFilename(const string_t& filename);

        /// Only asset::CAssetManager can create CAsset instances.
        friend class ZEN_API CAssetManager;

    protected:
        /// Only asset::CAssetManager can create CAsset instances.
        CAsset(const void* const owner = nullptr);

        /// No copy or assignment of assets.
        CAsset(const CAsset& Disabled);             /* = delete; */
        CAsset& operator=(const CAsset& Disabled); /* = delete; */

        virtual bool Destroy() = 0;

        static uint32_t s_seed;

        util::CLog& m_Log;

        string_t    m_filename;
        string_t    m_error_str;
        uint32_t    m_filename_hash;
        bool        m_loaded;

    private:
        uint32_t m_id;
        const void* mp_owner;
    };

    #include "Asset.inl"
}   // namespace asset
}   // namespace zen

#endif // ZENDERER__ASSETS__ASSET_HPP

/**
 * @class zen::asset::CAsset
 * @details
 *  Assets can include audio files, shaders, or any other data that can
 *  be loaded from disk with a filename.
 *
 *  Inheriting classes are required to be able to successfully reload
 *  themselves if they have been given a filename originally. Thus it is
 *  recommended to store the filename given to CAsset::LoadFromFile()
 *  internally. In addition, comparisons between assets to check for
 *  equivalence is typically done by comparing the filename hash and the
 *  owner address. Thus, CAsset::LoadFromFile() should be sure to call
 *  util::string_hash and store the value internally in
 *  CAsset::m_filename_hash. By default, if this value is not stored the
 *  base class will hash it on-the-fly, which obviously can be
 *  undesirable with successive calls.
 *
 *  Another requirement for all inheriting classes is the ability to
 *  load a copy of an asset from an existing asset using only existing
 *  virtual methods found in the base class. These will likely include
 *  CAsset::GetFilename() and CAsset::GetData().
 *
 *  All assets are assigned a (hopefully) unique ID at instantiation
 *  based on their address in memory. This ID can be used to find the
 *  asset using the asset manager, assuming the ID is known but the
 *  asset is not. The ID is also used in comparison.
 *
 *  Any class inheriting this base class does not necessarily need to
 *  place itself within the zen::asset namespace.
 *
 *  Inheriting class requirements:
 *      - Load from file / path
 *      - Load from another instance
 *      - Load from a filestream (optional but recommended)
 *      - The ability to retrieve copyable data via `GetData()`
 *      - Sets filename and filename hash
 *      - Sets `m_error_str` when encountering errors.
 *      - Sets `m_loaded` on a successful load.
 *
 * @see util::hash()
 * @see util::string_hash()
 **/

/** @} **/
