#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "dbs/dbs_mgr.h"
#include "interpreter.h"
#include "custom/include/test/test_fmw.h"

#include "interpreter/prima/pm_interpreter.h"
#include "interpreter/prima/pm_operand.h"
#include "compiler//wopcodes.h"

using namespace whais;
using namespace prima;

static const char admin[] = "administrator";

DArray
get_test_array()
{
  DArray array;

  array.Add(DRichReal(1.00));
  array.Add(DRichReal(2.00));
  array.Add(DRichReal(3.0));
  array.Add(DRichReal(5.17));

  return array;
}

template <typename DBS_T> bool
test_self_add(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfAdd(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_sub(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfSub(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_mul(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfMul(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_div(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfDiv(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_mod(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfMod(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_and(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfAnd(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_xor(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfXor(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}

template <typename DBS_T> bool
test_self_or(IOperand& op)
{
  bool result = true;

  try
  {
    DBS_T nullValue;
    op.SelfOr(nullValue);
  }
  catch(InterException& e)
  {
      assert(e.Code() == InterException::INVALID_OP_REQ);
      result = false;
  }
  catch(...)
  {
      assert(false);
  }

  return result;
}


template <typename DBS_T> bool
test_operand_for_exceptions(IOperand& op)
{
  bool result = false;

  try
  {
      DBS_T value;
      op.GetValue(value);
  }
  catch(InterException& e)
  {
      if (e.Code() == InterException::INVALID_OP_REQ)
        result = true;
  }
  catch(...)
  {
  }

  return result;
}

template <typename DBS_T> bool
test_operand_for_conv(IOperand& op)
{
  bool result = true;

  try
  {
      DBS_T value;
      op.GetValue(value);
  }
  catch(...)
  {
      result = false;
  }

  return result;
}

bool
test_op_invalid_conv(IOperand& op)
{
  bool result = true;

  result = result && test_operand_for_conv<DReal> (op);
  result = result && test_operand_for_conv<DRichReal> (op);
  result = result && test_operand_for_conv<DText> (op);

  result = result && test_operand_for_exceptions<DUInt8> (op);
  result = result && test_operand_for_exceptions<DUInt16> (op);
  result = result && test_operand_for_exceptions<DUInt32> (op);
  result = result && test_operand_for_exceptions<DUInt64> (op);
  result = result && test_operand_for_exceptions<DInt8> (op);
  result = result && test_operand_for_exceptions<DInt16> (op);
  result = result && test_operand_for_exceptions<DInt32> (op);
  result = result && test_operand_for_exceptions<DInt64> (op);
  result = result && test_operand_for_exceptions<DBool> (op);
  result = result && test_operand_for_exceptions<DChar> (op);
  result = result && test_operand_for_exceptions<DDate> (op);
  result = result && test_operand_for_exceptions<DDateTime> (op);
  result = result && test_operand_for_exceptions<DHiresTime> (op);
  result = result && test_operand_for_exceptions<DArray> (op);

  result = result && (! test_self_add<DChar> (op));
  result = result && test_self_add<DInt64> (op);
  result = result && test_self_add<DRichReal> (op);

  result = result && test_self_sub<DInt64> (op);
  result = result && test_self_sub<DRichReal> (op);

  result = result && test_self_mul<DInt64> (op);
  result = result && test_self_mul<DRichReal> (op);

  result = result && test_self_div<DInt64> (op);
  result = result && test_self_div<DRichReal> (op);

  result = result && (! test_self_mod<DInt64> (op));

  result = result && (! test_self_and<DBool> (op));
  result = result && (! test_self_and<DInt64> (op));

  result = result && ( ! test_self_xor<DBool> (op));
  result = result && ( ! test_self_xor<DInt64> (op));

  result = result && ( ! test_self_or<DBool> (op));
  result = result && ( ! test_self_or<DInt64> (op));

  try
  {
      op.GetField();
      result = false;
  }
  catch(InterException& e)
  {
      if (e.Code() != InterException::INVALID_OP_REQ)
        result = false;
  }
  catch(...)
  {
      result = false;
  }

  try
  {
      op.GetTable();
      result = false;
  }
  catch(InterException& e)
  {
      if (e.Code() != InterException::INVALID_OP_REQ)
        result = false;
  }
  catch(...)
  {
      result = false;
  }

  try
  {
      op.GetValueAt(0);
      result = false;
  }
  catch(InterException& e)
  {
      if (e.Code() != InterException::INVALID_OP_REQ)
        result = false;
  }
  catch(...)
  {
      result = false;
  }

  return result;
}

template <typename DBS_T> bool
test_null_write(IOperand& op, DBS_T value)
{
  bool result = true;

  op.SetValue(DBS_T());
  op.GetValue(value);

  if (value.IsNull() == false)
    result = false;

  return result;
}

template <typename DBS_T> bool
test_array_read_value(DArray array, const DBS_T testVal)
{
  bool  result = true;
  ArrayOperand arrayOp(array);
  NullOperand nullOp;
  StackValue sv(nullOp);
  IOperand* pOp = nullptr;

  for (uint_t index = 0;
       (index < array.Count()) && result;
       ++index)
    {
      sv = arrayOp.GetValueAt(index);
      pOp = &sv.Operand();


      DBS_T first, second;
      pOp->GetValue(first);
      array.Get(index, second);

      if ( (first != second) && ( ! test_op_invalid_conv(*pOp) ))
        result = false;
    }

  sv = arrayOp.GetValueAt(array.Count() - 1);
  pOp = &sv.Operand();
  result &= test_op_invalid_conv(*pOp);
  pOp->SetValue(testVal);

  DBS_T val;

  arrayOp.GetValue(array);
  array.Get(array.Count() - 1, val);

  if ( val != testVal)
    result = false;

  return result;
}

template <typename DBS_T> bool
test_table_value(ITable& table, DArray& array, const DBS_T testVal)
{
  bool result = true;
  DArray fieldArray;
  DBS_T    simpleVal;

  table.Get(0,
                  table.RetrieveField("simple_type"),
                  simpleVal);
  table.Get(0,
                  table.RetrieveField("array_type"),
                  fieldArray);

  if (testVal != simpleVal)
    return false;

  if (array.Count() != fieldArray.Count())
    return false;

  for (uint64_t el = 0; el < array.Count(); ++el)
    {
      DBS_T firstVal, secondVal;

      fieldArray.Get(el, firstVal);
      array.Get(el, secondVal);

      if (firstVal != secondVal)
        result = false;
    }

  return result;
}

template <typename DBS_T> bool
test_array_tableread_value(IDBSHandler& dbsHnd,
                            DArray      array,
                            const DBS_T   testVal)
{
  bool  result = true;
  const NullOperand nullOp;
  StackValue svArray(nullOp);
  StackValue svSimple(nullOp);
  StackValue svArrayEl(nullOp);

  DBSFieldDescriptor fd[2];

  fd[0].isArray      = true;
  fd[0].type  = testVal.DBSType();
  fd[0].name = "array_type";

  fd[1].isArray      = false;
  fd[1].type  = testVal.DBSType();
  fd[1].name = "simple_type";

  ITable& testTable = dbsHnd.CreateTempTable(2, fd);
  const ROW_INDEX row = testTable.GetReusableRow(true);
  {
    TableOperand tableOp(testTable, true);
    FieldOperand fieldArrayOp(tableOp, testTable.RetrieveField("array_type"));
    FieldOperand fieldSimpleOp(tableOp,
                                testTable.RetrieveField("simple_type"));

    svArray = fieldArrayOp.GetValueAt(row);
    svSimple = fieldSimpleOp.GetValueAt(row);
  }

  DBS_T firstArrayVal;
  array.Get(row, firstArrayVal);

  assert(testVal != firstArrayVal);

  result &= test_op_invalid_conv(svSimple.Operand());

  svArray.Operand().SetValue(array);
  svSimple.Operand().SetValue(firstArrayVal);

  result &= test_table_value(testTable, array, firstArrayVal);

  array.Set(0, testVal);
  svArrayEl = svArray.Operand().GetValueAt(0);

  result &= test_op_invalid_conv(svArrayEl.Operand());

  svArrayEl.Operand().SetValue(testVal);
  svSimple.Operand().SetValue(testVal);

  result &= test_table_value(testTable, array, testVal);

  result &= test_null_write(svSimple.Operand(), DBS_T());

  result &= test_table_value(testTable, array, DBS_T());

  return result;
}


int
main()
{
  bool success = true;

  {
    DBSInit(DBSSettings());
  }

  DBSCreateDatabase(admin);

  {
    IDBSHandler& dbsHnd = DBSRetrieveDatabase(admin);
    RichRealOperand op(DRichReal(10));
    success = success && test_op_invalid_conv(op);
    success = success && test_null_write(op, DRichReal(0.0));
    success = success && test_array_read_value(get_test_array(),
                                                DRichReal(0));
    success = success && test_array_tableread_value(dbsHnd,
                                                     get_test_array(),
                                                     DRichReal(0.0));

    DBSReleaseDatabase(dbsHnd);
  }

  DBSRemoveDatabase(admin);
  DBSShoutdown();

  if (!success)
    {
      std::cout << "TEST RESULT: FAIL" << std::endl;
      return 1;
    }

  std::cout << "TEST RESULT: PASS" << std::endl;

  return 0;
}

#ifdef ENABLE_MEMORY_TRACE
uint32_t WMemoryTracker::smInitCount = 0;
const char* WMemoryTracker::smModule = "T";
#endif
