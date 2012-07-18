/******************************************************************************
 PASTRA - A light database one file system and more.
 Copyright (C) 2008  Iulian Popa

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
 *****************************************************************************/

#include <assert.h>

#include "dbs_mgr.h"
#include "dbs_types.h"
#include "dbs_exception.h"

#include "ps_arraystrategy.h"
#include "ps_valintep.h"

using namespace pastra;
using namespace std;

static D_UINT ARRAY_MEMORY_RESERVE = 4096; //4KB

I_ArrayStrategy::I_ArrayStrategy (const DBS_FIELD_TYPE elemsType) :
    m_ElementsCount (0),
    m_ReferenceCount (0),
    m_ShareCount (1),
    m_ElementsType (elemsType)
{
}

I_ArrayStrategy::~I_ArrayStrategy()
{
  assert (m_ReferenceCount == 0);
}

D_UINT
I_ArrayStrategy::ReferenceCount() const
{
  return m_ReferenceCount;
}

void
I_ArrayStrategy::IncrementReferenceCount ()
{
  ++ m_ReferenceCount;
}

void
I_ArrayStrategy::DecrementReferenceCount ()
{
  assert (m_ReferenceCount > 0);

  -- m_ReferenceCount;
  if (m_ReferenceCount == 0)
    delete this;
}

D_UINT
I_ArrayStrategy::ShareCount () const
{
  return m_ShareCount;
}

void
I_ArrayStrategy::IncrementShareCount ()
{
  ++ m_ShareCount;
}

void
I_ArrayStrategy::DecrementShareCount ()
{
  -- m_ShareCount;
  assert (m_ShareCount > 0);
}


void
I_ArrayStrategy::Clone (I_ArrayStrategy& strategy)
{
  assert (strategy.Type() == m_ElementsType);
  assert (this != &strategy);

  D_UINT64 currentPosition = 0;
  D_UINT64 cloneSize = strategy.RawSize();
  while (cloneSize > 0)
    {
      D_UINT8   cloneBuff [128];
      D_UINT64  toClone = MIN (sizeof cloneBuff, cloneSize);

      strategy.ReadRaw (currentPosition, toClone, cloneBuff);
      WriteRaw (currentPosition, toClone, cloneBuff);

      cloneSize -= toClone, currentPosition += toClone;
    }

  m_ElementsCount = strategy.m_ElementsCount;
}

bool
I_ArrayStrategy::IsRowValue() const
{
  return false;
}

TemporalArray&
I_ArrayStrategy::GetTemporal()
{
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));

  return * _RC(TemporalArray *, this);
}

RowFieldArray&
I_ArrayStrategy::GetRowValue()
{
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));

  return * _RC(RowFieldArray *, this);
}


NullArray::NullArray (const DBS_FIELD_TYPE elemsType) :
    I_ArrayStrategy (elemsType)
{
}

NullArray::~NullArray ()
{
}

D_UINT
NullArray::ReferenceCount () const
{
  return ~0; //All 1s (e.g. but not 0 to force new allocation when is modified).
}

void
NullArray::IncrementReferenceCount ()
{
  return ; //Do nothing as we are a singletone that lifes for ever.
}

void
NullArray::DecrementReferenceCount ()
{
  return ; //Do nothing as we are a singletone that lifes for ever.
}

void
NullArray::ReadRaw (const D_UINT64 offset, const D_UINT64 length, D_UINT8 *const pData)
{
  //Some does not know what is doing!
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));
}
void
NullArray::WriteRaw (const D_UINT64 offset, const D_UINT64 length, const D_UINT8 *const pData)
{
  //Some does not know what is doing!
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));
}

void
NullArray::CollapseRaw (const D_UINT64 offset, const D_UINT64 count)
{
  //Some does not know what is doing!
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));
}

D_UINT64
NullArray::RawSize () const
{
  return 0;
}

NullArray&
NullArray::GetSingletoneInstace (const DBS_FIELD_TYPE type)
{
  static NullArray _boolInstance (T_BOOL);
  static NullArray _charInstance (T_CHAR);
  static NullArray _dateInstance (T_DATE);
  static NullArray _datetimeInstance (T_DATETIME);
  static NullArray _hirestimeInstance (T_HIRESTIME);
  static NullArray _uint8Instance (T_UINT8);
  static NullArray _uint16Instance (T_UINT16);
  static NullArray _uint32Instance (T_UINT32);
  static NullArray _uint64Instance (T_UINT64);
  static NullArray _realInstance (T_REAL);
  static NullArray _richrealInstance (T_RICHREAL);
  static NullArray _int8Instance (T_INT8);
  static NullArray _int16Instance (T_INT16);
  static NullArray _int32Instance (T_INT32);
  static NullArray _int64Instance (T_INT64);
  static NullArray _genericInstance (T_UNDETERMINED);

  switch (type)
  {
  case T_UNDETERMINED:
    return _genericInstance;
  case T_BOOL:
    return _boolInstance;
  case T_CHAR:
    return _charInstance;
  case T_DATE:
    return _dateInstance;
  case T_DATETIME:
    return _datetimeInstance;
  case T_HIRESTIME:
    return _hirestimeInstance;
  case T_UINT8:
    return _uint8Instance;
  case T_UINT16:
    return _uint16Instance;
  case T_UINT32:
    return _uint32Instance;
  case T_UINT64:
    return _uint64Instance;
  case T_REAL:
    return _realInstance;
  case T_RICHREAL:
    return _richrealInstance;
  case T_INT8:
    return _int8Instance;
  case T_INT16:
    return _int16Instance;
  case T_INT32:
    return _int32Instance;
  case T_INT64:
    return _int64Instance;
  case T_TEXT:
    throw DBSException (NULL, _EXTRA (DBSException::INVALID_ARRAY_TYPE));
  default:
    assert (0);
    throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));
  }
}



TemporalArray::TemporalArray (DBS_FIELD_TYPE type) :
    I_ArrayStrategy (type),
    m_Storage (DBSGetTempDir (), ARRAY_MEMORY_RESERVE)
{
}

TemporalArray::~TemporalArray ()
{
  assert (m_ReferenceCount == 0);
}

void
TemporalArray::ReadRaw (const D_UINT64 offset, const D_UINT64 length, D_UINT8 *const pData)
{
  m_Storage.Read (offset, length, pData);
}
void
TemporalArray::WriteRaw (const D_UINT64 offset, const D_UINT64 length, const D_UINT8 *const pData)
{
  m_Storage.Write (offset, length, pData);
}

D_UINT64
TemporalArray::RawSize () const
{
  return m_Storage.Size() ;
}

TemporalArray&
TemporalArray::GetTemporal ()
{
  return *this;
}

void
TemporalArray::CollapseRaw (const D_UINT64 offset, const D_UINT64 count)
{
  m_Storage.Colapse (offset, offset + count);
}


RowFieldArray::RowFieldArray (VLVarsStore& storage, D_UINT64 firstRecordEntry, DBS_FIELD_TYPE type) :
    I_ArrayStrategy (type),
    m_FirstRecordEntry (firstRecordEntry),
    m_Storage (storage)
{
  assert (m_FirstRecordEntry > 0);

  m_Storage.IncrementRecordRef (m_FirstRecordEntry);
  m_Storage.GetRecord (firstRecordEntry, 0, sizeof (m_ElementsCount), _RC(D_UINT8*, &m_ElementsCount));
}

RowFieldArray::~RowFieldArray ()
{
  assert (m_ReferenceCount == 0);

  m_Storage.DecrementRecordRef (m_FirstRecordEntry);
}

D_UINT
RowFieldArray::ReferenceCount () const
{
  return ~0; //All 1s (e.g. but not 1 force new allocation when is modified).
}

bool
RowFieldArray::IsRowValue () const
{
  return true;
}

RowFieldArray&
RowFieldArray::GetRowValue()
{
  return *this;
}

void
RowFieldArray::ReadRaw (const D_UINT64 offset, const D_UINT64 length, D_UINT8 *const pData)
{
  m_Storage.GetRecord(m_FirstRecordEntry, offset + sizeof (D_UINT64), length, pData);
}
void
RowFieldArray::WriteRaw (const D_UINT64 offset, const D_UINT64 length, const D_UINT8 *const pData)
{
  //We are not allowed to modify the row entry directly
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));
}

D_UINT64
RowFieldArray::RawSize () const
{
  D_INT64 storageSize = PSValInterp::Alignment (m_ElementsType, false);

  while (storageSize < PSValInterp::Size(m_ElementsType, false))
    storageSize += PSValInterp::Alignment (m_ElementsType, false);

  return m_ElementsCount * storageSize;
}

void
RowFieldArray::CollapseRaw (const D_UINT64 offset, const D_UINT64 count)
{
  //We are not allowed to modify the row entry directly
  throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));
}
