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

#include <assert.h>
#include <cstdio>
#include <cstring>

#include "utils/wutf.h"
#include "dbs_valtranslator.h"


using namespace whais;


static const uint_t   MAX_INTEGER_STR_SIZE  = 32;
static const uint64_t MAX_SIGNED_ABS        = 0x8000000000000000ull;
static const int64_t  USECS_PRECISION       = 1000000;
static const uint_t   MAX_UTF8_CHAR_SIZE    = 6;


static int64_t
read_real(const uint8_t* from,
          int64_t* const outIntPart,
          int64_t* const outFracPart,
          int64_t* const outPrecision)
{
  int64_t size = 0;
  bool negative = false;

  if ( *from == '-')
  {
    from++;
    if (( *from < '0') || ( *from > '9'))
      return 0;

    size++;
    negative = true;
  }

  uint64_t intPart = 0, fracPart = 0, precision = 1;
  while (('0' <= *from) && ( *from <= '9'))
  {
    if ((intPart * 10 + ( *from - '0')) > MAX_SIGNED_ABS)
      return 0; // Overflow

    intPart *= 10;
    intPart += *from - '0';

    from++, size++;
  }

  if ( *from == '.')
    from++, size++;

  while (('0' <= *from) && ( *from <= '9'))
  {
    if ((fracPart > (fracPart * 10 + ( *from - '0'))) || (precision > precision * 10))
    {
      return 0; // Overflow!
    }

    fracPart *= 10;
    fracPart += *from - '0';
    precision *= 10;

    from++, size++;
  }

  if (negative)
  {
    if (intPart > 0)
    {
      intPart -= 1;
      intPart = ~intPart;
    }

    if (fracPart > 0)
    {
      fracPart -= 1;
      fracPart = ~fracPart;
    }
  }

  *outIntPart = intPart;
  *outFracPart = fracPart;
  *outPrecision = precision;

  return size;
}


static uint64_t
read_integer(const uint8_t*    from,
             uint_t* const     outCount,
             const bool        isSigned)
{
  uint64_t result = 0;
  bool isNegative = false;

  *outCount = 0;
  if ( *from == '-')
  {
    ++from;
    if ( !isSigned || ( *from < '0') || ( *from > '9'))
      return 0;

    isNegative = true;
    *outCount += 1;
  }

  while ( *from >= '0' && *from <= '9')
  {
    if ((isSigned && ((result * 10 + ( *from - '0')) > MAX_SIGNED_ABS))
        || ( !isSigned && (result > (result * 10 + ( *from - '0')))))
    {
      *outCount = 0;
      return 0; // Overflow condition
    }

    result *= 10;
    result += *from - '0';

    ++from, *outCount += 1;
  }

  if (isNegative && (result > 0))
  {
    result -= 1;
    result = ~result;
  }

  return result;
}


int
Utf8Translator::Read(const uint8_t* const  utf8Src,
                     const uint_t          srcSize,
                     DBool* const          outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DBool();
    return 0;
  }
  else
  {
    if (( *utf8Src != '1') && ( *utf8Src != '0')
        && ( *utf8Src != 't') && ( *utf8Src != 'f')
        && ( *utf8Src != 'T') && ( *utf8Src != 'F'))
    {
      return -1;
    }

    *outValue = DBool(( *utf8Src == '1') || ( *utf8Src == 't') || ( *utf8Src == 'T'));
  }

  return 1;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     const bool           checkSpecial,
                     DChar* const         outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DChar();
    return 0;
  }

  if (( *utf8Src == '\\') && checkSpecial)
  {
    if (srcSize == 1)
      return false;

    switch (utf8Src[1])
    {
    case 'a':
      *outValue = DChar('\a');
      return 2;

    case 'b':
      *outValue = DChar('\b');
      return 2;

    case 'f':
      *outValue = DChar('\f');
      return 2;

    case 'n':
      *outValue = DChar('\n');
      return 2;

    case 'r':
      *outValue = DChar('\r');
      return 2;

    case 't':
      *outValue = DChar('\t');
      return 2;

    case 'u':
    {
      uint_t temp = 0;
      const int64_t code = read_integer(utf8Src + 2, &temp, true);

      if ((temp == 0) || (0xFFFFFFFF < code))
        return -1;

      *outValue = DChar(code);
      temp += 2;

      if (utf8Src[temp++] != '&')
        return -1;

      return temp;
    }

    default:
      *outValue = DChar(utf8Src[1]);
      return 2;
    }

  }

  const uint_t result = wh_utf8_cu_count( *utf8Src);
  if ((result == 0) || (result + 1 > srcSize))
    return -1;

  else
  {
    uint32_t ch;

    if ((wh_load_utf8_cp(utf8Src, &ch) != result) || (ch == 0))
    {
      return -1;
    }

    *outValue = DChar(ch);
  }

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DChar* const         outValue)
{
  return Utf8Translator::Read(utf8Src, srcSize, false, outValue);
}

int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DDate* const         outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DDate();
    return 0;
  }

  uint_t temp = 0;
  uint_t result = 0;

  const int64_t year = read_integer(utf8Src, &temp, true);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != '/') || (result > srcSize))
    return -1;

  const uint64_t month = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != '/') || (result > srcSize))
    return -1;

  const uint64_t day = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (result > srcSize))
    return -1;

  *outValue = DDate(year, month, day);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DDateTime* const     outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DDateTime();
    return 0;
  }

  uint_t temp = 0;
  uint_t result = 0;

  const int64_t year = read_integer(utf8Src, &temp, true);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != '/') || (result > srcSize))
    return -1;

  const uint64_t month = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != '/') || (result > srcSize))
    return -1;

  const uint64_t day = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != ' ') || (result > srcSize))
    return -1;

  const uint64_t hour = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != ':') || (result > srcSize))
    return -1;

  const uint64_t min = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != ':') || (result > srcSize))
    return -1;

  const uint64_t secs = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (result > srcSize))
    return -1;

  *outValue = DDateTime(year, month, day, hour, min, secs);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DHiresTime* const    outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DHiresTime();
    return 0;
  }

  uint_t temp = 0;
  uint_t result = 0;

  const int64_t year = read_integer(utf8Src, &temp, true);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != '/') || (result > srcSize))
    return -1;

  const uint64_t month = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != '/') || (result > srcSize))
    return -1;

  const uint64_t day = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != ' ') || (result > srcSize))
    return -1;

  const uint64_t hour = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != ':') || (result > srcSize))
    return -1;

  const uint64_t min = read_integer(utf8Src + result, &temp, false);
  result += temp;
  if ((temp == 0) || (utf8Src[result++] != ':') || (result > srcSize))
    return -1;

  int64_t secs, usecs, usecPrec;
  temp = read_real(utf8Src + result, &secs, &usecs, &usecPrec);
  result += temp;
  if ((temp == 0) || (secs < 0) || (usecs < 0) || (result > srcSize))
  {
    return -1;
  }

  if (usecPrec > USECS_PRECISION)
    usecs /= (usecPrec / USECS_PRECISION);

  else
    usecs *= USECS_PRECISION / usecPrec;

  *outValue = DHiresTime(year, month, day, hour, min, secs, usecs);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DInt8* const         outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DInt8();
    return 0;
  }

  uint_t result = 0;
  const int64_t value = read_integer(utf8Src, &result, true);
  if ((result == 0) || (result > srcSize) || (value < DInt8::Min().mValue)
      || (DInt8::Max().mValue < value))
  {
    return -1;
  }

  *outValue = DInt8(value);
  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DInt16* const        outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DInt16();
    return 0;
  }

  uint_t result = 0;
  const int64_t value = read_integer(utf8Src, &result, true);
  if ((result == 0)
      || (result > srcSize)
      || (value < DInt16::Min().mValue)
      || (DInt16::Max().mValue < value))
  {
    return -1;
  }

  *outValue = DInt16(value);

  return result;
}

int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DInt32* const        outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DInt32();
    return 0;
  }

  uint_t result = 0;
  const int64_t value = read_integer(utf8Src, &result, true);
  if ((result == 0)
      || (result > srcSize)
      || (value < DInt32::Min().mValue)
      || (DInt32::Max().mValue < value))
  {
    return -1;
  }

  *outValue = DInt32(value);

  return result;
}

int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DInt64* const        outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DInt64();
    return 0;
  }

  uint_t result = 0;
  const int64_t value = read_integer(utf8Src, &result, true);
  if ((result == 0) || (result > srcSize))
    return -1;

  *outValue = DInt64(value);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src, const uint_t srcSize, DUInt8* const outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DUInt8();
    return 0;
  }

  uint_t result = 0;
  const uint64_t value = read_integer(utf8Src, &result, false);
  if ((result == 0)
      || (result > srcSize)
      || (value < DUInt8::Min().mValue)
      || (DUInt8::Max().mValue < value))
  {
    return -1;
  }
  *outValue = DUInt8(value);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DUInt16* const       outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DUInt16();
    return 0;
  }

  uint_t result = 0;
  const uint64_t value = read_integer(utf8Src, &result, false);
  if ((result == 0) || (result > srcSize) || (value < DUInt16::Min().mValue)
      || (DUInt16::Max().mValue < value))
  {
    return -1;
  }

  *outValue = DUInt16(value);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t         srcSize,
                     DUInt32* const       outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DUInt32();
    return 0;
  }

  uint_t result = 0;
  const uint64_t value = read_integer(utf8Src, &result, false);
  if ((result == 0) || (result > srcSize) || (value < DUInt32::Min().mValue)
      || (DUInt32::Max().mValue < value))
  {
    return -1;
  }

  *outValue = DUInt32(value);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t srcSize,
                     DUInt64* const outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DUInt64();
    return 0;
  }

  uint_t result = 0;
  const uint64_t value = read_integer(utf8Src, &result, false);
  if ((result == 0) || (result > srcSize))
    return -1;

  *outValue = DUInt64(value);

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src, const uint_t srcSize, DReal* const outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DReal();
    return 0;
  }

  int64_t intPart, fracPart, precision;
  uint_t result = read_real(utf8Src, &intPart, &fracPart, &precision);
  if ((result == 0) || (result > srcSize))
    return -1;

  assert((precision == 1) || (precision % 10 == 0));

  *outValue = DReal(DBS_REAL_T(intPart, fracPart, precision));

  return result;
}


int
Utf8Translator::Read(const uint8_t* const utf8Src,
                     const uint_t srcSize,
                     DRichReal* const outValue)
{
  if (srcSize == 0)
    return -1;

  else if ( *utf8Src == 0)
  {
    *outValue = DRichReal();
    return 0;
  }

  int64_t intPart, fracPart, precision;
  uint_t result = read_real(utf8Src, &intPart, &fracPart, &precision);
  if ((result == 0) || (result > srcSize))
    return -1;

  assert((precision == 1) || (precision % 10 == 0));

  *outValue = DRichReal(DBS_RICHREAL_T(intPart, fracPart, precision));

  return result;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DBool& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }
  else if (maxSize < 2)
    return 0;

  else if (value.mValue)
    utf8Dest[0] = '1';

  else
    utf8Dest[0] = '0';

  utf8Dest[1] = 0;

  return 2;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest,
                      const uint_t maxSize,
                      const bool checkSpecial,
                      const DChar& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }
  else if (maxSize <= MAX_UTF8_CHAR_SIZE)
    return 0;

  if (checkSpecial)
  {
    utf8Dest[0] = '\\';
    utf8Dest[2] = 0;

    switch (value.mValue)
    {
    case '\a':
      utf8Dest[1] = 'a';
      return 3;

    case '\b':
      utf8Dest[1] = 'b';
      return 3;

    case '\f':
      utf8Dest[1] = 'f';
      return 3;

    case '\n':
      utf8Dest[1] = 'n';
      return 3;

    case '\r':
      utf8Dest[1] = 'r';
      return 3;

    case '\t':
      utf8Dest[1] = 't';
      return 3;

    case '\\':
      utf8Dest[1] = '\\';
      return 3;
    }
  }

  uint_t result = wh_store_utf8_cp(value.mValue, utf8Dest);
  utf8Dest[result++] = 0;

  return result;
}

uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DChar& value)
{
  return Utf8Translator::Write(utf8Dest, maxSize, false, value);
}

uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DDate& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%d/%u/%u",
                        value.mYear,
                        value.mMonth,
                        value.mDay);

  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}

uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DDateTime& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%d/%u/%u %u:%u:%u",
                        value.mYear,
                        value.mMonth,
                        value.mDay,
                        value.mHour,
                        value.mMinutes,
                        value.mSeconds);

  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DHiresTime& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%d/%u/%u %u:%u:%u.",
                        value.mYear,
                        value.mMonth,
                        value.mDay,
                        value.mHour,
                        value.mMinutes,
                        value.mSeconds);

  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  const int64_t usecsRes = result;

  result = snprintf(_RC(char*, utf8Dest + result), maxSize - result, "%06u", value.mMicrosec);

  if ((result < 0) || (result + usecsRes >= maxSize))
    return 0;

  return result + usecsRes + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DInt8& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest), maxSize, "%lld", _SC(long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DInt16& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest), maxSize, "%lld", _SC(long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DInt32& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest), maxSize, "%lld", _SC(long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DInt64& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest), maxSize, "%lld", _SC(long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DUInt8& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%llu",
                        _SC(unsigned long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DUInt16& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%llu",
                        _SC(unsigned long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DUInt32& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%llu",
                        _SC(unsigned long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DUInt64& value)
{
  if (maxSize == 0)
    return 0;

  if (value.IsNull())
  {
    utf8Dest[0] = 0;
    return 1;
  }

  int result = snprintf(_RC(char*, utf8Dest),
                        maxSize,
                        "%llu",
                        _SC(unsigned long long, value.mValue));
  if ((result < 0) || (_SC(uint_t, result) >= maxSize))
    return 0;

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DReal& value)
{
  char tempBuffer[MAX_INTEGER_STR_SIZE];
  uint_t result = 0;

  if (maxSize <= 1)
    return 0;

  *utf8Dest = 0;
  if (value.IsNull())
    return 1;

  uint64_t intPart = _SC(uint64_t, value.mValue.Integer());
  uint64_t fracPart = _SC(uint64_t, value.mValue.Fractional());
  if ((MAX_SIGNED_ABS <= intPart) || (MAX_SIGNED_ABS <= fracPart))
  {
    strcat(_RC(char*, utf8Dest), "-");

    intPart -= 1;
    intPart = ~intPart;
    fracPart -= 1;
    fracPart = ~fracPart;

    result++;
  }

  if (snprintf(tempBuffer, sizeof(tempBuffer), "%llu", _SC(unsigned long long, intPart)) < 0)
  {
    return 0;
  }

  if (result + strlen(tempBuffer) >= maxSize)
    return 0;

  strcpy(_RC(char*, utf8Dest + result), tempBuffer);
  result += strlen(tempBuffer);
  if (fracPart > 0)
  {
    if (result + 1 >= maxSize)
      return 0;
    else
    {
      utf8Dest[result++] = '.';
      utf8Dest[result] = 0;
    }

    uint64_t precision = value.mValue.Precision() / 10;
    while (precision > fracPart)
    {
      if (result + 1 >= maxSize)
        return 0;

      else
      {
        utf8Dest[result++] = '0';
        utf8Dest[result] = 0;
      }
      precision /= 10;
    }

    while ((fracPart % 10) == 0)
      fracPart /= 10;

    if (snprintf(tempBuffer, sizeof(tempBuffer), "%llu", _SC(unsigned long long, fracPart)) < 0)
    {
      return 0;
    }

    if (result + strlen(tempBuffer) >= maxSize)
      return 0;

    strcpy(_RC(char*, utf8Dest + result), tempBuffer);
    result += strlen(tempBuffer);
  }
  else
  {
    if (result + 2 >= maxSize)
      return 0;

    utf8Dest[result++] = '.';
    utf8Dest[result++] = '0';
    utf8Dest[result] = 0;
  }


  assert(result < maxSize);
  assert((strlen(_RC(char*, utf8Dest))) == result);

  return result + 1;
}


uint_t
Utf8Translator::Write(uint8_t* const utf8Dest, const uint_t maxSize, const DRichReal& value)
{
  char tempBuffer[MAX_INTEGER_STR_SIZE];
  uint_t result = 0;

  if (maxSize <= 1)
    return 0;

  *utf8Dest = 0;
  if (value.IsNull())
    return 1;

  uint64_t intPart = _SC(uint64_t, value.mValue.Integer());
  uint64_t fracPart = _SC(uint64_t, value.mValue.Fractional());
  if ((MAX_SIGNED_ABS <= intPart) || (MAX_SIGNED_ABS <= fracPart))
  {
    strcat(_RC(char*, utf8Dest), "-");

    intPart -= 1;
    intPart = ~intPart;
    fracPart -= 1;
    fracPart = ~fracPart;

    result++;
  }

  if (snprintf(tempBuffer, sizeof(tempBuffer), "%llu", _SC(unsigned long long, intPart)) < 0)
  {
    return 0;
  }

  if (result + strlen(tempBuffer) >= maxSize)
    return 0;

  strcpy(_RC(char*, utf8Dest + result), tempBuffer);
  result += strlen(tempBuffer);
  if (fracPart > 0)
  {
    if (result + 1 >= maxSize)
      return 0;

    else
    {
      utf8Dest[result++] = '.';
      utf8Dest[result] = 0;
    }

    uint64_t precision = value.mValue.Precision() / 10;
    while (precision > fracPart)
    {
      if (result + 1 >= maxSize)
        return 0;

      else
      {
        utf8Dest[result++] = '0';
        utf8Dest[result] = 0;
      }
      precision /= 10;
    }

    while ((fracPart % 10) == 0)
      fracPart /= 10;

    if (snprintf(tempBuffer, sizeof(tempBuffer), "%llu", _SC(unsigned long long, fracPart)) < 0)
    {
      return 0;
    }

    if (result + strlen(tempBuffer) >= maxSize)
      return 0;

    strcpy(_RC(char*, utf8Dest + result), tempBuffer);
    result += strlen(tempBuffer);
  }
  else
  {
    if (result + 2 >= maxSize)
      return 0;

    utf8Dest[result++] = '.';
    utf8Dest[result++] = '0';
    utf8Dest[result] = 0;
  }

  assert(result < maxSize);
  assert((strlen(_RC(char*, utf8Dest))) == result);

  return result + 1;
}

