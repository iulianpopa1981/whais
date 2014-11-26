/*
 * test_stack_update_array.cpp
 *
 *  Created on: Feb 14, 2013
 *      Author: ipopa
 */
#include <iostream>
#include <cstring>

#include "test_client_common.h"

using namespace std;

static const uint16_t MAX_VALUES_ENTRIES = 10;

struct ArrayValuesEntry
{
  uint16_t      type;
  uint16_t      elementsCount;
  const char* values[MAX_VALUES_ENTRIES];
};

struct ArrayValuesEntry _values[] =
    {
        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},

        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},

        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},


        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},


        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},


        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},


        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},



        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},


        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},

        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},

        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},

        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }},


        /* Repeat */

        {WHC_TYPE_BOOL, 5, {"0", "0", "1", "0", "1", }},
        {WHC_TYPE_CHAR, 5, {"A", "a", "Z", "\xC5\xA2", "\xE2\x8B\xA0", }},
        {WHC_TYPE_DATE, 3, {"2012/11/21", "2981/3/1", "3123/4/4", }},
        {WHC_TYPE_DATETIME, 3, {"2012/11/21 14:23:12", "2981/3/1 5:1:1", "3123/8/2 0:1:2", }},
        {WHC_TYPE_HIRESTIME, 3, {"2012/11/21 14:23:12.001", "2981/3/1 5:1:1.1", "3123/8/2 0:1:2.0004", }},
        {WHC_TYPE_UINT8, 7, {"127", "128", "254", "0", "1", "255", "123", }},
        {WHC_TYPE_UINT16, 7, {"32767", "32768", "65534", "0", "1", "65535", "123", }},
        {WHC_TYPE_UINT32, 7, {"2147483647", "2147483648", "4294967294", "0", "1", "4294967295", "123", }},
        {WHC_TYPE_UINT64, 7, {"9223372036854775807", "9223372036854775808", "18446744073709551614", "0", "1", "18446744073709551615", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813888.999999", "549755813887.999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775808.99999999999999", "9223372036854775807.99999999999999", "-123.123", "124.012", "-235451.167" }},
        {WHC_TYPE_INT8, 5, {"-128", "127", "0", "1", "123", }},
        {WHC_TYPE_INT16, 5, {"-32768", "32767", "0", "1", "123", }},
        {WHC_TYPE_INT32, 5, {"-2147483648", "2147483647", "0", "1", "123", }},
        {WHC_TYPE_INT64, 5, {"-9223372036854775808", "9223372036854775807", "0", "1", "123", }},
        {WHC_TYPE_REAL, 5, {"-549755813887.000001", "-0.000001", "0.000001", "1.1", "0" }},
        {WHC_TYPE_RICHREAL, 5, {"-9223372036854775807.00000000000001", "-0.00000000000001", "0.00000000000001", "1.1", "0" }}
    };

static const uint_t _valuesCount  = sizeof( _values) / sizeof( _values[0]);

static bool
fill_array_entries_step( WH_CONNECTION hnd)
{
  for (uint_t i = 0; i < _valuesCount; i++)
    {
      if ((WPushValue( hnd,
                       _values[i].type | WHC_TYPE_ARRAY_MASK,
                       0,
                       NULL) != WCS_OK)
          || (WFlush( hnd) != WCS_OK))
        {
          return false;
        }

      for (uint_t j = 0; j < _values[i].elementsCount; ++j)
        {
          if ((WUpdateValue( hnd,
                             _values[i].type,
                             WIGNORE_FIELD,
                             WIGNORE_ROW,
                             j,
                             WIGNORE_OFF,
                             _values[i].values[j]) != WCS_OK)
              || (WFlush( hnd) != WCS_OK))
            {
              return false;
            }
        }
    }

  return true;
}

static bool
fill_array_entries_bulk( WH_CONNECTION hnd)
{
  for (uint_t i = 0; i < _valuesCount; i++)
    {
      if (WPushValue( hnd,
                           _values[i].type | WHC_TYPE_ARRAY_MASK,
                           0,
                           NULL) != WCS_OK)
        {
          return false;
        }

      for (uint_t j = 0; j < _values[i].elementsCount; ++j)
        {
          if (WUpdateValue( hnd,
                            _values[i].type,
                            WIGNORE_FIELD,
                            WIGNORE_ROW,
                            j,
                            WIGNORE_OFF,
                            _values[i].values[j]) != WCS_OK)
            {
              return false;
            }
        }
    }

  if (WFlush( hnd) != WCS_OK)
    return false;

  return true;
}

static bool
check_array_entry( WH_CONNECTION hnd, const int index)
{
  const ArrayValuesEntry* pEntry = &_values[index];
  const char*           value;
  unsigned long long int  count;

  if ((WValueArraySize( hnd,
                        WIGNORE_FIELD,
                        WIGNORE_ROW,
                        &count) != WCS_OK)
      || (count != pEntry->elementsCount))
    {
      return false;
    }

  for (uint_t i = 0; i < pEntry->elementsCount; ++i)
    {
      if ((WValueEntry( hnd,
                        WIGNORE_FIELD,
                        WIGNORE_ROW,
                        i,
                        WIGNORE_OFF,
                        &value) != WCS_OK)
          || (strcmp( value, pEntry->values[i]) != 0))
        {
          return false;
        }
    }

  if ((WValueEntry( hnd,
                    WIGNORE_FIELD,
                    WIGNORE_ROW,
                    pEntry->elementsCount,
                    WIGNORE_OFF,
                    &value) != WCS_INVALID_ARRAY_OFF)
      || (WValueEntry( hnd,
                       WIGNORE_FIELD,
                       WIGNORE_ROW,
                       pEntry->elementsCount,
                       WIGNORE_OFF,
                       NULL) != WCS_INVALID_ARGS))
    {
      return false;
    }

  return true;
}

static bool
test_array_step_update( WH_CONNECTION hnd)
{
  cout << "Testing array step update ... ";

  if (! fill_array_entries_step( hnd))
    goto test_array_step_update_fail;

  for (int i = _valuesCount - 1; i >= 0; i--)
    {
      if (! check_array_entry( hnd, i))
        goto test_array_step_update_fail;

      if ((WPopValues( hnd, 1) != WCS_OK)
          || (WFlush( hnd) != WCS_OK))
        {
          goto test_array_step_update_fail;
        }
    }

  cout << "OK\n";
  return true;

test_array_step_update_fail:
  cout << "FAIL\n";
  return false;

}

static bool
test_array_bulk_update( WH_CONNECTION hnd)
{
  cout << "Testing array bulk update ... ";

  if (! fill_array_entries_bulk( hnd))
    goto test_array_bulk_update_fail;

  for (int i = _valuesCount - 1; i >= 0; i--)
    {
      if (! check_array_entry( hnd, i))
        goto test_array_bulk_update_fail;

      if ((WPopValues( hnd, 1) != WCS_OK)
          || (WFlush( hnd) != WCS_OK))
        {
          goto test_array_bulk_update_fail;
        }
    }

  cout << "OK\n";
  return true;

test_array_bulk_update_fail:
  cout << "FAIL\n";
  return false;

}

static bool
test_for_errors( WH_CONNECTION hnd)
{
  unsigned long long count;

  cout << "Testing against error conditions ... ";

  if ((WPushValue( hnd, WHC_TYPE_BOOL | WHC_TYPE_ARRAY_MASK, 0, NULL) != WCS_OK)
      || (WFlush( hnd) != WCS_OK)
      || (WFlush( hnd) != WCS_OK) //Just for fun!
      || (WValueRowsCount( NULL, NULL) != WCS_INVALID_ARGS)
      || (WValueRowsCount( NULL, &count) != WCS_INVALID_ARGS)
      || (WValueRowsCount( hnd, NULL) != WCS_INVALID_ARGS)
      || (WValueRowsCount( hnd, &count) != WCS_TYPE_MISMATCH)
      || (WValueArraySize( hnd, WIGNORE_FIELD, WIGNORE_ROW, &count) != WCS_OK)
      || (count != 0)
      || (WValueArraySize( hnd, "some_f", WIGNORE_ROW, &count) != WCS_INVALID_FIELD)
      || (WValueArraySize( hnd, WIGNORE_FIELD, 0, &count) != WCS_INVALID_ROW)
      || (WValueTextLength( hnd, WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, &count) != WCS_INVALID_ARRAY_OFF)
      || (WValueTextLength( hnd, "some_f", WIGNORE_ROW, WIGNORE_OFF, &count) != WCS_INVALID_FIELD)
      || (WValueTextLength( hnd, WIGNORE_FIELD, 0, WIGNORE_OFF, &count) != WCS_INVALID_ROW)
      || (WValueTextLength( hnd, WIGNORE_FIELD, WIGNORE_OFF, 0, &count) != WCS_INVALID_ARRAY_OFF)
      || (WPopValues( hnd, WPOP_ALL) != WCS_OK)
      || (WFlush( hnd) != WCS_OK))
    {
      goto test_for_errors_fail;
    }

  cout << "OK\n";

  return true;

test_for_errors_fail :

  cout << "FAIL\n";
  return false;
}


const char*
DefaultDatabaseName()
{
  return "test_list_db";
}

const uint_t
DefaultUserId()
{
  return 1;
}

const char*
DefaultUserPassword()
{
  return "test_password";
}

int
main( int argc, const char** argv)
{
  WH_CONNECTION hnd        = NULL;

  bool success = tc_settup_connection( argc, argv, &hnd);

  success = success && test_for_errors( hnd);
  success = success && test_array_step_update( hnd);
  success = success && test_array_bulk_update( hnd);

  WClose( hnd);

  if (!success)
    {
      cout << "TEST RESULT: FAIL" << std::endl;
      return 1;
    }

  cout << "TEST RESULT: PASS" << std::endl;

  return 0;
}
