#!/bin/bash

DB_NAME=t_wcmd_add
DB_DIR=./tmp
DB_FILE=$DB_DIR/temp_file


execute_wcmd_command () {
	wcmd -u $DB_NAME -d $DB_DIR <<< "$1" > $DB_FILE
	[ $? -eq 0 ] || fail_test "Failed to execute test command: $1"
	err_chk=`cat $DB_FILE | grep "\<FAIL\>"`
	[ -z "$err_chk"  ] || fail_test "Failed to execute test command: $1"
}

check_listed_table () {
	wcmd -u $DB_NAME -d $DB_DIR <<< "table" > $DB_FILE
	[ $? -eq 0 ] || fail_test "Failed to list tables!"
	echo ""
	while [[ $# > 0 ]] ; do 
		table=`cat $DB_FILE | grep "$1"`
		[ -n "$table" ] || fail_test "Missing table $1"
		echo "Found table $1."
		shift ;
	done
}

check_table_fields () {
	echo "Testing fields of table "$1" ... "
	wcmd -u $DB_NAME -d $DB_DIR <<< "table $1" > $DB_FILE
	[ $? -eq 0 ] || fail_test "Failed to list tables!"
	shift ;
	while [[ $# > 0 ]] ; do 
		table=`cat $DB_FILE | grep "$1"`
		[ -n "$table" ] || fail_test "Failing field test $1"
		echo "Passing field test $1."
		shift ;
	done
}

check_missing_table_fields () {
	echo "Testing lack of field for table "$1" ... "
	wcmd -u $DB_NAME -d $DB_DIR <<< "table $1" > $DB_FILE
	[ $? -eq 0 ] || fail_test "Failed to list tables!"
	shift ;
	while [[ $# > 0 ]] ; do 
		table=`cat $DB_FILE | grep "$1"`
		[ -z "$table" ] || fail_test "Field '$1' should not be present $1"
		echo "Passing missing field '$1' test."
		shift ;
	done
}

fail_test () {
	echo "$1"
	rm -rf "$DB_DIR"
	echo "TEST RESULT:  FAIL"
	exit 1
}


mkdir -p $DB_DIR || fail_test "Failed to create temporal directory '$DB_DIR'."
wcmd -c $DB_NAME -d $DB_DIR <<< "echo $DB_NAME is created...."
[ $? -eq 0 ] || fail_test "Cannot create the database!"
execute_wcmd_command "add test_table text_field TEXT bool_field BOOL char_field \
                     CHAR date_field DATE dt_field DATETIME ht_field HIRESTIME\
                     int8_field INT8 int16_field INT16 int32_field INT32 int64_field INT64\
                     uint8_field UINT8 uint16_field UINT16 uint32_field UINT32 uint64_field UINT64\
                     real_field REAL rr_field RICHREAL\
           	     a_bool_field ARRAY BOOL a_char_field ARRAY CHAR a_date_field ARRAY DATE a_dt_field ARRAY DATETIME a_ht_field ARRAY HIRESTIME\
                     a_int8_field ARRAY INT8 a_int16_field ARRAY INT16 a_int32_field ARRAY INT32 a_int64_field ARRAY INT64\
                     a_uint8_field ARRAY UINT8 a_uint16_field ARRAY UINT16 a_uint32_field ARRAY UINT32 a_uint64_field ARRAY UINT64\
                     a_real_field ARRAY REAL a_rr_field ARRAY RICHREAL "
execute_wcmd_command "copy test_table test_table_cp"
execute_wcmd_command "copy test_table test_table_2 text_field"
execute_wcmd_command "copy test_table test_table_3 text_field,a_rr_field"
execute_wcmd_command "copy test_table test_table_4 date_field,a_int16_field@array_int16_field"
execute_wcmd_command "copy test_table test_table_5 * 1"
execute_wcmd_command "copy test_table test_table_6 real_field,rr_field 0"
execute_wcmd_command "copy test_table test_table_7 char_field * int64_field=min@max"

check_listed_table "test_table" "test_table_2" "test_table_3" "test_table_4" "test_table_5" "test_table_6" "test_table_7"
check_table_fields "test_table" 'text_field *: TEXT$' \
				'char_field *: CHAR$' \
				'date_field *: DATE$' \
				'dt_field *: DATETIME$' \
				'ht_field *: HIRESTIME$' \
				'int8_field *: INT8$' \
				'int16_field *: INT16$' \
				'int32_field *: INT32$' \
				'int64_field *: INT64$' \
				'uint8_field *: UINT8$' \
				'uint16_field *: UINT16$' \
				'uint32_field *: UINT32$' \
				'uint64_field *: UINT64$' \
				'real_field *: REAL$' \
				'rr_field *: RICHREAL$' \
				'a_char_field *: CHAR ARRAY$' \
				'a_date_field *: DATE ARRAY$' \
				'a_dt_field *: DATETIME ARRAY$' \
				'a_ht_field *: HIRESTIME ARRAY$' \
				'a_int8_field *: INT8 ARRAY$' \
				'a_int16_field *: INT16 ARRAY$' \
				'a_int32_field *: INT32 ARRAY$' \
				'a_int64_field *: INT64 ARRAY$' \
				'a_uint8_field *: UINT8 ARRAY$' \
				'a_uint16_field *: UINT16 ARRAY$' \
				'a_uint32_field *: UINT32 ARRAY$' \
				'a_uint64_field *: UINT64 ARRAY$' \
				'a_real_field *: REAL ARRAY$' \
				'a_rr_field *: RICHREAL ARRAY$'

check_table_fields "test_table_cp" 'text_field *: TEXT$' \
				'char_field *: CHAR$' \
				'date_field *: DATE$' \
				'dt_field *: DATETIME$' \
				'ht_field *: HIRESTIME$' \
				'int8_field *: INT8$' \
				'int16_field *: INT16$' \
				'int32_field *: INT32$' \
				'int64_field *: INT64$' \
				'uint8_field *: UINT8$' \
				'uint16_field *: UINT16$' \
				'uint32_field *: UINT32$' \
				'uint64_field *: UINT64$' \
				'real_field *: REAL$' \
				'rr_field *: RICHREAL$' \
				'a_char_field *: CHAR ARRAY$' \
				'a_date_field *: DATE ARRAY$' \
				'a_dt_field *: DATETIME ARRAY$' \
				'a_ht_field *: HIRESTIME ARRAY$' \
				'a_int8_field *: INT8 ARRAY$' \
				'a_int16_field *: INT16 ARRAY$' \
				'a_int32_field *: INT32 ARRAY$' \
				'a_int64_field *: INT64 ARRAY$' \
				'a_uint8_field *: UINT8 ARRAY$' \
				'a_uint16_field *: UINT16 ARRAY$' \
				'a_uint32_field *: UINT32 ARRAY$' \
				'a_uint64_field *: UINT64 ARRAY$' \
				'a_real_field *: REAL ARRAY$' \
				'a_rr_field *: RICHREAL ARRAY$'


check_table_fields "test_table_2" 'text_field *: TEXT$'
check_table_fields "test_table_3" 'text_field *: TEXT$'\
				  'a_rr_field *: RICHREAL ARRAY'
check_table_fields "test_table_4" 'date_field *: DATE$'\
				  'array_int16_field *: INT16 ARRAY'
check_table_fields "test_table_5" 'text_field *: TEXT$' \
				'char_field *: CHAR$' \
				'date_field *: DATE$' \
				'dt_field *: DATETIME$' \
				'ht_field *: HIRESTIME$' \
				'int8_field *: INT8$' \
				'int16_field *: INT16$' \
				'int32_field *: INT32$' \
				'int64_field *: INT64$' \
				'uint8_field *: UINT8$' \
				'uint16_field *: UINT16$' \
				'uint32_field *: UINT32$' \
				'uint64_field *: UINT64$' \
				'real_field *: REAL$' \
				'rr_field *: RICHREAL$' \
				'a_char_field *: CHAR ARRAY$' \
				'a_date_field *: DATE ARRAY$' \
				'a_dt_field *: DATETIME ARRAY$' \
				'a_ht_field *: HIRESTIME ARRAY$' \
				'a_int8_field *: INT8 ARRAY$' \
				'a_int16_field *: INT16 ARRAY$' \
				'a_int32_field *: INT32 ARRAY$' \
				'a_int64_field *: INT64 ARRAY$' \
				'a_uint8_field *: UINT8 ARRAY$' \
				'a_uint16_field *: UINT16 ARRAY$' \
				'a_uint32_field *: UINT32 ARRAY$' \
				'a_uint64_field *: UINT64 ARRAY$' \
				'a_real_field *: REAL ARRAY$' \
				'a_rr_field *: RICHREAL ARRAY$'
check_table_fields "test_table_6" 'real_field *: REAL$'\
				  'rr_field *: RICHREAL'
check_table_fields "test_table_7" 'char_field *: CHAR$'

echo "Starting the second part of the test .... "


execute_wcmd_command "rows test_table reuse text_field='This is a simple text',int8_field=1,rr_field=0.99"
execute_wcmd_command "rows test_table reuse text_field='This is another smart test',rr_field=-1.11"
execute_wcmd_command "rows test_table reuse a_int16_field=[5 -2 100 10 101 -10111]"
execute_wcmd_command "rows test_table add rr_field=10.13"
execute_wcmd_command "rows test_table reuse text_field='Once again...'"
execute_wcmd_command "rows test_table reuse text_field='Once again and again another long test that has to be long enough... or lonnger than long enough!'"
execute_wcmd_command "remove test_table_cp test_table_2 test_table_3 test_table_4 test_table_5 test_table_6 test_table_7"

echo "Testing values of copy table .... "

TEXT_FIELD_ROWS=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table list text_field" | cut -s -d \| -f 3`
INT8_FIELD_ROWS=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table list int8_field" | cut -s -d \| -f 3`
RR_FIELD_ROWS=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table list rr_field" | cut -s -d \| -f 3`
A_INT16_FIELD_ROWS=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table list a_int16_field" | cut -s -d \| -f 3`

execute_wcmd_command "copy test_table test_table_cp"
execute_wcmd_command "copy test_table test_table_2 int8_field@f_int8,rr_field@f_rr "
execute_wcmd_command "copy test_table test_table_3 text_field@t_f,a_int16_field 2"
execute_wcmd_command "copy test_table test_table_4 char_field * int64_field=null rr_field=min@max"


echo "TEXT_FIELD_ROWS: "
echo "$TEXT_FIELD_ROWS"
echo "INT8_FIELD_ROWS: "
echo "$INT8_FIELD_ROWS"
echo "RR_FIELD_ROWS: "
echo "$RR_FIELD_ROWS"
echo "A_INT16_FIELD_ROWS: "
echo "$A_INT16_FIELD_ROWS"

TEXT_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_cp list text_field" | cut -s -d \| -f 3`
INT8_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_cp list int8_field" | cut -s -d \| -f 3`
RR_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_cp list rr_field" | cut -s -d \| -f 3`
A_INT16_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_cp list a_int16_field" | cut -s -d \| -f 3`

[ "$TEXT_FIELD_ROWS" == "$TEXT_FIELD_ROWS_2" ] || fail_test "Text field rows does not match."
[ "$INT8_FIELD_ROWS" == "$INT8_FIELD_ROWS_2" ] || fail_test "Int8 field rows does not match."
[ "$RR_FIELD_ROWS" == "$RR_FIELD_ROWS_2" ] || fail_test "Int8 field rows does not match."
[ "$A_INT16_FIELD_ROWS" == "$A_INT16_FIELD_ROWS_2" ] || fail_test "Array field rows does not match."

echo "Testing values of table 2 .... "

INT8_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_2 list f_int8" | cut -s -d \| -f 3`
RR_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_2 list f_rr" | cut -s -d \| -f 3`

[ "$INT8_FIELD_ROWS" == "$INT8_FIELD_ROWS_2" ] || fail_test 'Int8 field rows does not match (table_test_2).'
[ "$RR_FIELD_ROWS" == "$RR_FIELD_ROWS_2" ] || fail_test 'Int8 field rows does not match (table_test_2).'

echo "Testing values of table 3 .... "

TEXT_FIELD_ROWS=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_cp list text_field" | grep '2 *|' | cut -s -d \| -f 3`
A_INT16_FIELD_ROWS=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_cp list a_int16_field" | grep '2 *|' | cut -s -d \| -f 3`

TEXT_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_3 list t_f" |  cut -s -d \| -f 3`
A_INT16_FIELD_ROWS_2=`wcmd -u $DB_NAME -d $DB_DIR <<< "rows test_table_3 list a_int16_field" | cut -s -d \| -f 3`


echo "TEXT_FIELD_ROWS: "
echo "$TEXT_FIELD_ROWS"
echo "A_INT16_FIELD_ROWS: "
echo "$A_INT16_FIELD_ROWS"


[ "$TEXT_FIELD_ROWS" == "$TEXT_FIELD_ROWS_2" ] || fail_test 'Text field rows does not match. (test_table_3)'
[ "$A_INT16_FIELD_ROWS" == "$A_INT16_FIELD_ROWS_2" ] || fail_test 'Array field rows does not match (test_table_3).'

check_table_fields "test_table_cp" 'text_field *: TEXT$' \
				'char_field *: CHAR$' \
				'date_field *: DATE$' \
				'dt_field *: DATETIME$' \
				'ht_field *: HIRESTIME$' \
				'int8_field *: INT8$' \
				'int16_field *: INT16$' \
				'int32_field *: INT32$' \
				'int64_field *: INT64$' \
				'uint8_field *: UINT8$' \
				'uint16_field *: UINT16$' \
				'uint32_field *: UINT32$' \
				'uint64_field *: UINT64$' \
				'real_field *: REAL$' \
				'rr_field *: RICHREAL$' \
				'a_char_field *: CHAR ARRAY$' \
				'a_date_field *: DATE ARRAY$' \
				'a_dt_field *: DATETIME ARRAY$' \
				'a_ht_field *: HIRESTIME ARRAY$' \
				'a_int8_field *: INT8 ARRAY$' \
				'a_int16_field *: INT16 ARRAY$' \
				'a_int32_field *: INT32 ARRAY$' \
				'a_int64_field *: INT64 ARRAY$' \
				'a_uint8_field *: UINT8 ARRAY$' \
				'a_uint16_field *: UINT16 ARRAY$' \
				'a_uint32_field *: UINT32 ARRAY$' \
				'a_uint64_field *: UINT64 ARRAY$' \
				'a_real_field *: REAL ARRAY$' \
				'a_rr_field *: RICHREAL ARRAY$'


check_listed_table "test_table" "test_table_2" "test_table_3" "test_table_4"
check_table_fields "test_table_2" 'f_int8 *: INT8$' 'f_rr *: RICHREAL$'
check_table_fields "test_table_3" 't_f *: TEXT$' 'a_int16_field *: INT16 ARRAY'
check_table_fields "test_table_4" 'char_field *: CHAR'
check_missing_table_fields "test_table_2" 'text_field'
check_missing_table_fields "test_table_3" 'text_field' 'rr_field' '\<int16_field'
check_missing_table_fields "test_table_4" 'text_field' 'rr_field' 'int64_field'

rm -rf "$DB_DIR"
echo "TEST RESULT:  PASS"

