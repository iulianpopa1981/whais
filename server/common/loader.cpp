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

#include <string>
#include <sstream>
#include <vector>

#include "dbs/dbs_mgr.h"
#include "interpreter/interpreter.h"
#include "compiler//compiledunit.h"
#include "utils/logger.h"
#include "loader.h"


using namespace std;
using namespace whais;


static const char CLEAR_LOG_STREAM[] = "";

bool
LoadDatabase(FileLogger& log, DBSDescriptors& inoutDesc)
{
  uint64_t      temp;

  ostringstream logEntry;
  logEntry << "Loading database: " << inoutDesc.mDbsName;
  log.Log(LT_INFO, logEntry.str());
  logEntry.str(CLEAR_LOG_STREAM);

  inoutDesc.mDbs = &DBSRetrieveDatabase(inoutDesc.mDbsName.c_str(),
                                        inoutDesc.mDbsDirectory.c_str());
  std::unique_ptr<Logger> dbsLogger = unique_make(FileLogger, inoutDesc.mDbsLogFile.c_str(), true);

  logEntry << "Sync interval is set at " << inoutDesc.mSyncInterval << " milliseconds";
  dbsLogger->Log(LT_INFO, logEntry.str());
  log.Log(LT_INFO, logEntry.str());
  logEntry.str(CLEAR_LOG_STREAM);

  logEntry << "Request timeout interval is set at " << inoutDesc.mWaitReqTmo << " milliseconds.";
  dbsLogger->Log(LT_INFO, logEntry.str());
  log.Log(LT_INFO, logEntry.str());
  logEntry.str(CLEAR_LOG_STREAM);

  if (inoutDesc.mDbsName != GlobalContextDatabase())
    inoutDesc.mSession = &GetInstance(inoutDesc.mDbsName.c_str(), dbsLogger.get());

  else
    inoutDesc.mSession = &GetInstance(nullptr, dbsLogger.get());

  temp = inoutDesc.mStackCount;
  if ( ! inoutDesc.mSession->NotifyEvent(ISession::MAX_STACK_COUNT, &temp))
  {
    logEntry << "Failed to set the maximum stack count limitation for session '"
             << inoutDesc.mDbsName << "' at " << inoutDesc.mStackCount << " elements.";

    log.Log(LT_ERROR, logEntry.str());
    logEntry.str(CLEAR_LOG_STREAM);
    logEntry.str(CLEAR_LOG_STREAM);
  }

  for (const auto& lib : inoutDesc.mNativeLibs)
  {
    logEntry << "... Loading dynamic native library '" << lib << "'.";
    dbsLogger->Log(LT_INFO, logEntry.str());
    log.Log(LT_INFO, logEntry.str());
    logEntry.str(CLEAR_LOG_STREAM);

    WH_SHLIB shl = wh_shl_load(lib.c_str());
    if (shl == INVALID_SHL || !inoutDesc.mSession->LoadSharedLib(shl))
      log.Log(LT_ERROR, "Failed to load the dynamic library.");
  }

  for (const auto& obj : inoutDesc.mObjectLibs)
  {
    logEntry << "... Loading compiled object unit '" << obj << "'.";
    dbsLogger->Log(LT_INFO, logEntry.str());
    log.Log(LT_INFO, logEntry.str());
    logEntry.str(CLEAR_LOG_STREAM);

    CompiledFileUnit unit(obj.c_str());
    inoutDesc.mSession->LoadCompiledUnit(unit);
  }

  inoutDesc.mLogger = dbsLogger.release();

  return true;
}
