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


#ifndef ALTER_TABLE_H_
#define ALTER_TABLE_h_

#include <string>
#include <vector>

#include "whais.h"

#include "utils/wtypes.h"
#include "utils/range.h"
#include "dbs/dbs_mgr.h"

namespace whais {

class TableAlterRules
{
public:
  TableAlterRules(whais::IDBSHandler&             dbs,
                   const std::string&              table,
                   const std::vector<std::string>& fields);
  ~TableAlterRules();

  void DropField(const std::string& field);
  void RetypeField(const std::string&   name,
                   const DBS_FIELD_TYPE type,
                   const bool           isArray);
  void RenameField(const std::string& oldName, const std::string& newName);
  void AddField(const std::string&   name,
                const DBS_FIELD_TYPE type,
                const bool           isArray);

  void CommitToTable(const std::string&         newTableName,
                     const DArray&              selectedRows,
                     whais::IDBSHandler* const  otherDbs = nullptr);
  void CommitToTable(const std::string&             newTableName,
                     const whais::Range<ROW_INDEX>& selectedRows,
                     whais::IDBSHandler* const      otherDbs = nullptr);
  void Commit();

private:
  TableAlterRules(whais::IDBSHandler&             dbs,
                  whais::ITable&                  table);

  void CommitToTable(whais::ITable&                 table,
                     const whais::Range<ROW_INDEX>& selectedRows);

  struct FieldConnection
  {
    FIELD_INDEX       mSrc;
    FIELD_INDEX       mDst;

    void              (*mCopyValue) (whais::ITable&         src,
                                     whais::ITable&         dst,
                                     const ROW_INDEX        row);
  };

  whais::IDBSHandler&               mDbs;
  const std::string                 mTableName;
  whais::ITable*                    mTable;
  std::vector<DBSFieldDescriptor>   mSrcFields;
  std::vector<DBSFieldDescriptor>   mDstFields;
};

} //namespace whais


#endif /* ALTER_TABLE_H_ */
