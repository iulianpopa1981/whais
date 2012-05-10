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
#include <vector>

#include "dbs_values.h"
#include "dbs_mgr.h"
#include "random.h"

#include "ps_textstrategy.h"
#include "ps_arraystrategy.h"
#include "ps_valintep.h"

using namespace pastra;
using namespace std;

static const D_UINT MAX_VALUE_RAW_STORAGE = 0x20;

static bool
is_leap_year (const D_INT year)
{
  if ((year % 4) == 0)
    {
      if ((year % 100) == 0)
        {
          if ((year % 400) == 0)
            return true;
          else
            return false;
        }
      else
        return true;
    }
  else
    return false;
}

static bool
is_valid_date (D_INT year, D_UINT month, D_UINT day)
{
  static D_UINT monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (--month > 11)
    return false;
  else if (day == 0)
    return false;

  if ((month != 1) && (day > monthDays [month]))
    return false;
  else if (month == 1)
    {
      if (is_leap_year (year) && (day > (monthDays[1] + 1)))
        return false;
      else if (day > monthDays[1])
        return false;
    }

  return true;
}

static bool
is_valid_datetime (D_INT year, D_UINT month, D_UINT day, D_UINT hour, D_UINT min, D_UINT sec)
{
  if (is_valid_date (year, month, day) == false)
    return false;
  else
    if ((hour > 23) || (min > 59) || (sec > 59))
      return false;

  return true;
}

static bool
is_valid_hiresdate (D_INT year,
                    D_UINT month,
                    D_UINT day,
                    D_UINT hour,
                    D_UINT min,
                    D_UINT sec,
                    D_UINT microsec)
{
  if (is_valid_datetime (year, month, day, hour, min, sec) == false)
    return false;
  else
    if (microsec > 999999999)
      return false;

  return true;
}

DBSDate::DBSDate (D_INT32 year, D_UINT8 month, D_UINT8 day) :
    m_Year (year),
    m_Month (month),
    m_Day (day),
    m_IsNull (false)
{
  if ((m_IsNull == false) && (is_valid_date (year, month, day) == false))
    throw DBSException (NULL, _EXTRA (DBSException::INVALID_DATE));
}

DBSDateTime::DBSDateTime (D_INT32 year,
                          D_UINT8 month,
                          D_UINT8 day,
                          D_UINT8 hour,
                          D_UINT8 minutes,
                          D_UINT8 seconds) :
    m_Year (year),
    m_Month (month),
    m_Day (day),
    m_Hour (hour),
    m_Minutes (minutes),
    m_Seconds (seconds),
    m_IsNull (false)
{
  if ((m_IsNull == false) &&
      (is_valid_datetime (year, month, day, hour, minutes, seconds) == false))
    throw DBSException (NULL, _EXTRA (DBSException::INVALID_DATETIME));
}

DBSHiresTime::DBSHiresTime (D_INT32 year,
                            D_UINT8 month,
                            D_UINT8 day,
                            D_UINT8 hour,
                            D_UINT8 minutes,
                            D_UINT8 seconds,
                            D_UINT32 microsec) :
    m_Microsec (microsec),
    m_Year (year),
    m_Month (month),
    m_Day (day),
    m_Hour (hour),
    m_Minutes (minutes),
    m_Seconds (seconds),
    m_IsNull (false)
{
  if ((m_IsNull == false) &&
      (is_valid_hiresdate (year, month, day, hour, minutes, seconds, microsec) == false))
    throw DBSException (NULL, _EXTRA (DBSException::INVALID_DATETIME));
}

DBSText::DBSText (const D_UINT8 *pUtf8String) :
    m_pText (& NullText::GetSingletoneInstace())
{
  if ((pUtf8String != NULL) && (*pUtf8String != 0))
    {
      std::auto_ptr <I_TextStrategy> apText (new TemporalText (pUtf8String));
      apText.get ()->IncreaseReferenceCount();

      m_pText = apText.release ();
    }

}

DBSText::DBSText (I_TextStrategy &text) :
    m_pText (& NullText::GetSingletoneInstace())
{
  text.IncreaseReferenceCount ();
  m_pText = &text;
}

DBSText::DBSText (const DBSText &sourceText) :
    m_pText (& NullText::GetSingletoneInstace())
{
  sourceText.m_pText->IncreaseReferenceCount();

  m_pText = sourceText.m_pText;
}

DBSText &
DBSText::operator= (const DBSText &sourceText)
{
  if (this == &sourceText)
    return *this;

  I_TextStrategy *const pText = m_pText;

  sourceText.m_pText->IncreaseReferenceCount();
  m_pText = sourceText.m_pText;

  pText->DecreaseReferenceCount ();

  return *this;
}

bool
DBSText::operator== (const DBSText &text)
{
  if (IsNull() != text.IsNull())
    return false;

  if (IsNull() == true)
    return true;

  D_UINT64 textSize = GetRawUtf8Count();

  assert (textSize != 0);
  assert (text.GetRawUtf8Count() != 0);

  if (textSize != text.GetRawUtf8Count())
    return false;

  D_UINT64 offset = 0;
  D_UINT8  first[64];
  D_UINT8  second[64];

  while (textSize > 0)
  {
    D_UINT chunkSize = MIN (sizeof first, textSize);

    GetRawUtf8 (offset, chunkSize, first);
    text.GetRawUtf8 (offset, chunkSize, second);

    if (memcmp (first, second, chunkSize) != 0)
      return false;

    offset += chunkSize, textSize -= chunkSize;
  }

  return true;
}

DBSText::~DBSText ()
{
  m_pText->DecreaseReferenceCount();
}

bool
DBSText::IsNull () const
{
  return (m_pText->GetBytesCount() == 0);
}

D_UINT64
DBSText::GetCharactersCount () const
{
  return m_pText->GetCharactersCount();
}

D_UINT64
DBSText::GetRawUtf8Count () const
{
  return m_pText->GetBytesCount();
}

void
DBSText::GetRawUtf8 (D_UINT64 offset, D_UINT64 count, D_UINT8 *const pBuffer) const
{
  if (IsNull())
    return;

  count = min (count, m_pText->GetBytesCount());
  m_pText->RawReadUtf8Data (offset, count, pBuffer);
}

void
DBSText::Append (const DBSChar& character)
{
  if ((character.m_IsNull) || (character.m_Value == 0))
    return ;

  if (m_pText->GetReferenceCount() > 1)
    {
      auto_ptr<I_TextStrategy> newText (new TemporalText(NULL));
      newText->IncreaseReferenceCount();
      newText.get()->Duplicate (*m_pText);

      m_pText->DecreaseReferenceCount();
      m_pText = newText.release ();
    }

  assert (m_pText->GetReferenceCount() != 0);

  m_pText->Append (character.m_Value);
}

void
DBSText::Append (const DBSText& text)
{
  if (text.IsNull())
    return;

  if (m_pText->GetReferenceCount() > 1)
    {
      auto_ptr<I_TextStrategy> newText (new TemporalText(NULL));
      newText->IncreaseReferenceCount();
      newText.get()->Duplicate (*m_pText);

      m_pText->DecreaseReferenceCount();
      m_pText = newText.release ();
    }

  m_pText->Append (*text.m_pText);
}

DBSChar
DBSText::GetCharAtIndex(const D_UINT64 index) const
{
  return m_pText->GetCharacterAtIndex (index);
}

void
DBSText::SetCharAtIndex (const DBSChar& rCharacter, const D_UINT64 index)
{
  const D_UINT64 charsCount = m_pText->GetCharactersCount();

  if (charsCount <= index)
    throw DBSException (NULL, _EXTRA (DBSException::STRING_INDEX_TOO_BIG));

  auto_ptr<I_TextStrategy> newText (new TemporalText(NULL));
  newText->IncreaseReferenceCount();

  for (D_UINT64 it = 0; it < charsCount; ++it)
    if (index != it)
      newText->Append (m_pText->GetCharacterAtIndex (it).m_Value);
    else
      {
        if ((rCharacter.m_IsNull) || (rCharacter.m_Value == 0))
          break ;

        newText->Append (rCharacter.m_Value);
      }

  m_pText->DecreaseReferenceCount();
  m_pText = newText.release ();
}


template <class T> void
init_array (const T* array, D_UINT64 count, I_ArrayStrategy*& prOutStrategy)
{
  if (count == 0)
    {
      assert (array == NULL);
      prOutStrategy = & NullArray::GetSingletoneInstace (array[0]);

      return; // We finished here!
    }

  if (array == NULL)
    throw DBSException (NULL, _EXTRA(DBSException::INVALID_PARAMETERS));

  auto_ptr <TemporalArray> apArray (new TemporalArray (array[0]));
  prOutStrategy = apArray.get ();

  D_UINT64 currentOffset = 0;
  D_UINT   valueIncrement = 0;

  while (valueIncrement < _SC(D_UINT,  PSValInterp::GetSize (array[0], false)))
    valueIncrement += PSValInterp::GetAlignment (array[0], false);

  for (D_UINT64 index = 0; index < count; ++index)
    {
      if (array[index].IsNull ())
        continue;

      D_UINT8 rawStorage [MAX_VALUE_RAW_STORAGE];

      assert (valueIncrement <= (sizeof rawStorage));

      PSValInterp::Store (array[index], rawStorage);
      prOutStrategy->WriteRawData(currentOffset, valueIncrement, rawStorage);
      prOutStrategy->IncrementElementsCount();
      currentOffset += valueIncrement;
    }

  prOutStrategy->IncrementReferenceCount();
  apArray.release ();
}

DBSArray::DBSArray (const DBSBool* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSChar* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSDate* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSDateTime* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSHiresTime* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSUInt8* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSUInt16* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSUInt32* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSUInt64* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSReal* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSRichReal* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSInt8* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSInt16* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSInt32* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (const DBSInt64* array, D_UINT64 count) :
    m_pArray (NULL)
{
  init_array (array, count, m_pArray);
}

DBSArray::DBSArray (I_ArrayStrategy& strategy) :
    m_pArray (NULL)
{
  strategy.IncrementReferenceCount();
  m_pArray = &strategy;
}

DBSArray::~DBSArray ()
{
  m_pArray->DecrementReferenceCount ();
}

DBSArray::DBSArray (const DBSArray& rSource) :
    m_pArray (rSource.m_pArray)
{
  m_pArray->IncrementReferenceCount ();
}


DBSArray &
DBSArray::operator= (const DBSArray& rSource)
{
  if (&rSource == this)
    return *this;

  if (rSource.GetElementsType() != GetElementsType())
    throw DBSException (NULL, _EXTRA(DBSException::INVALID_ARRAY_TYPE));

  I_ArrayStrategy* pTemp = m_pArray;
  rSource.m_pArray->IncrementReferenceCount();
  m_pArray = rSource.m_pArray;

  pTemp->DecrementReferenceCount();

  return *this;
}

D_UINT64
DBSArray::GetElementsCount () const
{
  return m_pArray->GetElementsCount();
}

DBS_FIELD_TYPE
DBSArray::GetElementsType () const
{
  return m_pArray->GetElementsType();
}

static D_INT
get_aligned_elem_size (DBS_FIELD_TYPE type)
{
  D_INT result = 0;
  while (result < PSValInterp::GetSize(type, false))
    result += PSValInterp::GetAlignment (type, false);

  return result;
}

static void
prepare_array_strategy (I_ArrayStrategy*& pArrayStrategy)
{
  assert (pArrayStrategy->GetReferenceCount() > 0);

  if (pArrayStrategy->GetReferenceCount() == 1)
    return ; //Do not change anything

  auto_ptr <I_ArrayStrategy> newStrategy (new TemporalArray (pArrayStrategy->GetElementsType()));

  newStrategy->Clone (*pArrayStrategy);
  pArrayStrategy->DecrementReferenceCount();
  pArrayStrategy = newStrategy.release();
  pArrayStrategy->IncrementReferenceCount();
}

template <class T> inline D_UINT64
add_array_element (const T& element, I_ArrayStrategy*& pArrayStrategy)
{
  if (pArrayStrategy->GetElementsType() != element)
    throw DBSException (NULL, _EXTRA(DBSException::INVALID_ARRAY_TYPE));
  else if (element.IsNull())
    throw DBSException (NULL, _EXTRA (DBSException::NULL_ARRAY_ELEMENT));

  static const D_UINT storageSize = get_aligned_elem_size (element);

  prepare_array_strategy (pArrayStrategy);

  D_UINT8 rawElement[MAX_VALUE_RAW_STORAGE];

  assert (storageSize <= sizeof rawElement);
  PSValInterp::Store (element, rawElement);
  pArrayStrategy->WriteRawData (pArrayStrategy->GetRawDataSize(), storageSize, rawElement);

  assert ((pArrayStrategy->GetRawDataSize() % storageSize) == 0);

  pArrayStrategy->IncrementElementsCount();

  return (pArrayStrategy->GetElementsCount() - 1);
}

D_UINT64
DBSArray::AddElement (const DBSBool& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSChar& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSDate& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSDateTime& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSHiresTime& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSUInt8& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSUInt16& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSUInt32& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSUInt64& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSReal& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSRichReal& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSInt8& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSInt16& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSInt32& value)
{
  return add_array_element (value, m_pArray);
}

D_UINT64
DBSArray::AddElement (const DBSInt64& value)
{
  return add_array_element (value, m_pArray);
}

template <class T> void
get_array_element (T& outElement, I_ArrayStrategy* const pArrayStrategy, const D_UINT64 index)
{
  if (pArrayStrategy->GetElementsCount() <= index)
    throw DBSException (NULL, _EXTRA (DBSException::ARRAY_INDEX_TOO_BIG));
  else if (pArrayStrategy->GetElementsType() != outElement)
    throw DBSException (NULL, _EXTRA(DBSException::INVALID_ARRAY_TYPE));

  static const D_UINT storageSize = get_aligned_elem_size (outElement);

  D_UINT8 rawElement[MAX_VALUE_RAW_STORAGE];

  assert (sizeof rawElement >= storageSize);

  pArrayStrategy->ReadRawData (index * storageSize, storageSize, rawElement);
  PSValInterp::Retrieve (&outElement, rawElement);
}

void
DBSArray::GetElement (DBSBool& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSChar& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSDate& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSDateTime& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSHiresTime& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSUInt8& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSUInt16& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSUInt32& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSUInt64& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSReal& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSRichReal& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSInt8& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSInt16& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSInt32& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}

void
DBSArray::GetElement (DBSInt64& outValue, const D_UINT64 index) const
{
  get_array_element (outValue, m_pArray, index);
}


template <class T>  inline void
set_array_element (I_ArrayStrategy*& pArrayStrategy, const T& value, const D_UINT64 index)
{
  if (index >= pArrayStrategy->GetElementsCount())
    throw DBSException(NULL, _EXTRA(DBSException::ARRAY_INDEX_TOO_BIG));

  prepare_array_strategy (pArrayStrategy);
  static const D_UINT storageSize = get_aligned_elem_size (pArrayStrategy->GetElementsType());

  if (value.IsNull())
    {
      pArrayStrategy->CollapseRawData (index * storageSize, storageSize);
      pArrayStrategy->DecrementElementsCount ();
    }
  else
    {
      D_UINT8 rawElement[MAX_VALUE_RAW_STORAGE];
      PSValInterp::Store (value, rawElement);
      pArrayStrategy->WriteRawData(storageSize * index, storageSize, rawElement);
    }
}

void
DBSArray::SetElement (const DBSBool& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSChar& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSDate& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSDateTime& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSHiresTime& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement ( const DBSUInt8& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSUInt16& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSUInt32& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSUInt64& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSReal& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSRichReal& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}


void
DBSArray::SetElement (const DBSInt8& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSInt16& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSInt32& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}

void
DBSArray::SetElement (const DBSInt64& newValue, const D_UINT64 index)
{
  set_array_element (m_pArray, newValue, index);
}


void
DBSArray::RemoveElement (const D_UINT64 index)
{
  if (index >= m_pArray->GetElementsCount())
    throw DBSException(NULL, _EXTRA(DBSException::ARRAY_INDEX_TOO_BIG));

  prepare_array_strategy (m_pArray);

  const D_UINT storageSize = get_aligned_elem_size (m_pArray->GetElementsType());

  m_pArray->CollapseRawData (index*storageSize, storageSize);
  m_pArray->DecrementElementsCount();
}


template <class DBS_T> D_INT64
partition (DBSArray& array, D_INT64 from, D_INT64 to, bool& alreadySorted)
{
  assert (from < to);
  assert (to < _SC(D_INT64, array.GetElementsCount()));

  const D_INT64 pivotIndex = (from + to) / 2;
  DBS_T         leftEl;
  DBS_T         pivot;
  DBS_T         rightEl;
  DBS_T         temp;

  array.GetElement (pivot, pivotIndex);
  array.GetElement (leftEl, from);

  alreadySorted = false;
  if ((leftEl < pivot) || (leftEl == pivot))
    {
      temp = leftEl;
      while (from < to)
        {
          array.GetElement (leftEl, from + 1);
          if (((leftEl < pivot) || (leftEl == pivot)) && ((temp < leftEl) || (temp == leftEl)))
            {
              ++from;
              temp = leftEl;
            }
          else
            break;
        }
      if (from == to)
        alreadySorted = true;
    }

  while (from < to)
    {
      while (from <= to)
        {
          array.GetElement (leftEl, from);
          if (leftEl < pivot)
            ++from;
          else
            break;
        }

      if (leftEl == pivot)
        {
          while (from < to)
            {
              array.GetElement (temp, from + 1);
              if (temp == pivot)
                ++from;
              else
                break;
            }
        }

      assert ((from < to) || (leftEl == pivot));

      while (from <= to)
        {
          array.GetElement (rightEl, to);
          if (pivot < rightEl)
            --to;
          else
            break;
        }

      assert ((from < to) || (rightEl == pivot));

      if (from < to)
        {
          if (leftEl == rightEl)
            {
              assert (leftEl == pivot);
              ++from;
            }
          else
            {
              array.SetElement (leftEl, to);
              array.SetElement (rightEl, from);
            }
        }
    }

  return from;
}

template <class DBS_T> D_INT64
partition_reverse (DBSArray& array, D_INT64 from, D_INT64 to, bool& alreadySorted)
{
  assert (from < to);
  assert (to < _SC(D_INT64, array.GetElementsCount()));

  const D_INT64 pivotIndex = (from + to) / 2;
  DBS_T         leftEl;
  DBS_T         pivot;
  DBS_T         rightEl;
  DBS_T         temp;

  array.GetElement (pivot, pivotIndex);
  array.GetElement (leftEl, from);

  alreadySorted = false;
  if ((leftEl > pivot) || (leftEl == pivot))
    {
      temp = leftEl;
      while (from < to)
        {
          array.GetElement (leftEl, from + 1);
          if (((leftEl > pivot) || (leftEl == pivot)) && ((temp > leftEl) || (temp == leftEl)))
            {
              ++from;
              temp = leftEl;
            }
          else
            break;
        }
      if (from == to)
        alreadySorted = true;
    }

  while (from < to)
    {
      while (from <= to)
        {
          array.GetElement (leftEl, from);
          if (pivot < leftEl)
            ++from;
          else
            break;
        }

      if (leftEl == pivot)
        {
          while (from < to)
            {
              array.GetElement (temp, from + 1);
              if (temp == pivot)
                ++from;
              else
                break;
            }
        }

      assert ((from < to) || (leftEl == pivot));

      while (from <= to)
        {
          array.GetElement (rightEl, to);
          if (rightEl < pivot)
            --to;
          else
            break;
        }

      assert ((from < to) || (rightEl == pivot));

      if (from < to)
        {
          if (leftEl == rightEl)
            {
              assert (leftEl == pivot);
              ++from;
            }
          else
            {
              array.SetElement (leftEl, to);
              array.SetElement (rightEl, from);
            }
        }
    }

  return from;
}

struct _partition_t
{
  _partition_t (D_UINT64 from, D_UINT64 to) : m_From (from), m_To (to) {}
  D_INT64 m_From;
  D_INT64 m_To;
};

template <class DBS_T> void
quick_sort (DBSArray&  array, D_INT64 from, D_INT64 to, const bool reverse)
{
  assert (from <= to);

  std::vector <_partition_t> partStack;
  partStack.push_back ( _partition_t (from, to));

  do
    {
      _partition_t current = partStack [partStack.size() - 1];
      partStack.pop_back();

      if (current.m_From >= current.m_To)
        continue;

      D_INT64 pivot;
      bool    alreadySorted;

      if (reverse)
        pivot = partition_reverse<DBS_T> (array, current.m_From, current.m_To, alreadySorted);
      else
        pivot = partition<DBS_T> (array, current.m_From, current.m_To, alreadySorted);

      if (alreadySorted == false)
        {
          if ((pivot + 1) < current.m_To)
            partStack.push_back (_partition_t (pivot + 1, current.m_To));

          if (current.m_From < (pivot - 1))
            partStack.push_back (_partition_t (current.m_From, pivot - 1));
        }
    } while (partStack.size() > 0);
}

void
DBSArray::Sort (bool reverse)
{
  switch (GetElementsType ())
  {
  case T_BOOL:
    quick_sort<DBSBool> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_CHAR:
    quick_sort<DBSChar> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_DATE:
    quick_sort<DBSDate> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_DATETIME:
    quick_sort<DBSDateTime> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_HIRESTIME:
    quick_sort<DBSHiresTime> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_UINT8:
    quick_sort<DBSUInt8> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_UINT16:
    quick_sort<DBSUInt16> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_UINT32:
    quick_sort<DBSUInt32> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_UINT64:
    quick_sort<DBSUInt64> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_REAL:
    quick_sort<DBSReal> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_RICHREAL:
    quick_sort<DBSRichReal> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_INT8:
    quick_sort<DBSInt8> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_INT16:
    quick_sort<DBSInt16> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_INT32:
    quick_sort<DBSInt32> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  case T_INT64:
    quick_sort<DBSInt64> (*this, 0, GetElementsCount () - 1, reverse);
    break;
  default:
    assert (false);
  }
}