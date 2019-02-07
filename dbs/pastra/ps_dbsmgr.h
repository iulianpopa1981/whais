/******************************************************************************
WHAIS - An advanced database system
Copyright(C) 2014-2018  Iulian Popa

Address: Str Olimp nr. 6
         Pantelimon Ilfov,
         Romania
Phone:   +40721939650
e-mail:  popaiulian@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef PS_DBSMGR_H_
#define PS_DBSMGR_H_

#include <tuple>
#include <map>
#include <string.h>

#include "utils/wthread.h"
#include "utils/tokenizer.h"
#include "dbs/dbs_mgr.h"
#include "dbs/dbs_types.h"


namespace whais {
namespace pastra {


//Forward declarations
class PersistentTable;
class DbsHandler;
struct DbsManager;


class DbsHandler : public IDBSHandler
{
public:
  DbsHandler(const DBSSettings& settings, const std::string& directory, const std::string& name);
  DbsHandler(DbsHandler&& source);

  virtual ~DbsHandler() override;

  virtual TABLE_INDEX PersistentTablesCount() override;
  virtual ITable& RetrievePersistentTable(const TABLE_INDEX index) override;
  virtual ITable& RetrievePersistentTable(const char* const name) override;
  virtual void ReleaseTable(ITable&) override;

  virtual void AddTable(const char* const          name,
                       const FIELD_INDEX           fieldsCount,
                       DBSFieldDescriptor* const   inoutFields) override;
  virtual void DeleteTable(const char* const name) override;
  virtual void SyncAllTablesContent() override;
  virtual void SyncTableContent(const TABLE_INDEX index) override;
  virtual bool NotifyDatabaseUpdate(const bool tryDbLock) override;

  virtual ITable& CreateTempTable(const FIELD_INDEX fieldsCount, DBSFieldDescriptor* inoutFields) override;
  virtual const char* TableName(const TABLE_INDEX index) override;

  void Discard();
  void RemoveFromStorage();

  const std::string& WorkingDir() const { return mDbsLocationDir; }
  const std::string& TemporalDir() const { return mGlbSettings.mTempDir; }
  uint64_t MaxFileSize() const { return mGlbSettings.mMaxFileSize; }
  const DBSSettings& Settings() const { return mGlbSettings; }

  bool HasUnreleasedTables();
  void RegisterTableSpawn();


private:
  using TABLE_DATA = std::tuple<PersistentTable*,uint32_t>;
  using TABLES = std::map<std::string, TABLE_DATA>;

  void SyncToFile();

  const DBSSettings&   mGlbSettings;
  Lock                 mSync;
  const std::string    mDbsLocationDir;
  const std::string    mFileName;
  File                 mFile;
  TABLES               mTables;
  int                  mCreatedTemporalTables;
  bool                 mNeedsSync;
};


struct DbsElement
{
  DbsElement(DbsHandler&& dbs)
    : mRefCount(0),
      mDbs(std::move(dbs))
  {}

  uint64_t   mRefCount;
  DbsHandler mDbs;
};


struct DbsManager
{
  using DATABASES_MAP = std::map<std::string, DbsElement>;

  DbsManager(const DBSSettings& settings)
    : mDBSSettings(settings)
  {
    if (mDBSSettings.mWorkDir.length() == 0
        || mDBSSettings.mTempDir.length() == 0
        || mDBSSettings.mTableCacheBlkSize == 0
        || mDBSSettings.mTableCacheBlkCount == 0
        || mDBSSettings.mVLStoreCacheBlkSize == 0
        || mDBSSettings.mVLStoreCacheBlkCount == 0
        || mDBSSettings.mVLValueCacheSize == 0)
    {
      throw DBSException(_EXTRA(DBSException::BAD_PARAMETERS),
          "Cannot create a database manager with the specified parameters.");
    }
    NormalizeFilePath(mDBSSettings.mWorkDir, true);
    NormalizeFilePath(mDBSSettings.mTempDir, true);
  }

  Lock            mSync;
  DBSSettings     mDBSSettings;
  DATABASES_MAP   mDatabases;
};


} //namespace pastra
} //namespace whais


#endif                                /* PS_DBSMGR_H_ */
