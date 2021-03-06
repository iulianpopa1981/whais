#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "dbs/dbs_mgr.h"
#include "interpreter.h"
#include "custom/include/test/test_fmw.h"

#include "interpreter/prima/pm_interpreter.h"
#include "interpreter/prima/pm_processor.h"
#include "compiler//wopcodes.h"

using namespace whais;
using namespace prima;

static const char admin[] = "administrator";
static const char procName[] = "p1";

const uint8_t dummyProgram[] = ""
    "PROCEDURE p1(a1 INT8, a2 INT8) RETURN BOOL\n"
    "DO\n"
      "VAR hd1, hd2 HIRESTIME\n;"
      "\n"
      "hd1 = '2012/12/12 13:00:00.100';\n"
      "hd2 = '2012/12/11 13:00:00';\n"
      "RETURN hd1 < hd2;\n"
    "ENDPROC\n"
    "\n";

static const char *MSG_PREFIX[] = {
                                      "", "error ", "warning ", "error "
                                    };

static uint_t
get_line_from_buffer(const char * buffer, uint_t buff_pos)
{
  uint_t count = 0;
  int result = 1;

  if (buff_pos == WHC_IGNORE_BUFFER_POS)
    return -1;

  while (count < buff_pos)
    {
      if (buffer[count] == '\n')
        ++result;
      else if (buffer[count] == 0)
        {
          assert(0);
        }
      ++count;
    }
  return result;
}

void
my_postman(WH_MESSENGER_CTXT data,
            uint_t            buff_pos,
            uint_t            msg_id,
            uint_t            msgType,
            const char*     pMsgFormat,
            va_list           args)
{
  const char *buffer = (const char *) data;
  int buff_line = get_line_from_buffer(buffer, buff_pos);

  fprintf(stderr, MSG_PREFIX[msgType]);
  fprintf(stderr, "%d : line %d: ", msg_id, buff_line);
  vfprintf(stderr, pMsgFormat, args);
  fprintf(stderr, "\n");
};

static uint_t
w_encode_opcode(W_OPCODE opcode, uint8_t* pOutCode)
{
  return wh_compiler_encode_op(pOutCode, opcode);
}


static bool
test_op_stb(Session& session)
{
  std::cout << "Testing bool assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DBool op;
  DBool op2(true);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STB, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DBool result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

static bool
test_op_stc(Session& session)
{
  std::cout << "Testing char assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DChar op;
  DChar op2('A');

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STC, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DChar result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

static bool
test_op_std(Session& session)
{
  std::cout << "Testing date assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DDate op;
  DDate op2(1989, 12, 25);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STD, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);


  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DDate result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

static bool
test_op_stdt(Session& session)
{
  std::cout << "Testing date time assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DDateTime op;
  DDateTime op2(1989, 12, 25, 12, 0, 0);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STDT, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DDateTime result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

static bool
test_op_stht(Session& session)
{
  std::cout << "Testing hires date time assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DHiresTime op;
  DHiresTime op2(1989, 12, 25, 12, 0, 0, 1);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STHT, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DHiresTime result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

template <typename DBS_T> bool
test_op_stXX(Session& session, const W_OPCODE code, const char*  pText)
{
  std::cout << "Testing " << pText << " assignment...\n";

  const uint32_t procId = session.FindProcedure(_RC(const uint8_t*, procName),
                                                sizeof procName - 1);
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DBS_T op;
  DBS_T op2(0x61);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(code, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DBS_T result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

template <typename DBS_T> bool
test_op_stud(Session& session, const DBS_T refValue, const char*  pText)
{
  std::cout << "Testing " << pText << " undefined assignments...\n";

  const uint32_t procId = session.FindProcedure(_RC(const uint8_t*, procName),
                                                sizeof procName - 1);
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STUD, testCode + opSize);
  opSize += w_encode_opcode(W_CTS, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 0;
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push();
  stack.Push(StackValue::Create(refValue));


  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DBS_T result;
  stack[0].Operand().GetValue(result);

  if (result != refValue)
    return false;

  return true;
}


template <typename DBS_T> bool
test_op_stud_array(Session& session, const DBS_T refValue, const char*  pText)
{
  std::cout << "Testing " << pText << " array undefined assignments...\n";

  const uint32_t procId = session.FindProcedure(_RC(const uint8_t*, procName),
                                                sizeof procName - 1);
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STUD, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  DArray ref(_SC(DBS_T*, nullptr));
  stack.Push(StackValue::Create(ref));

  ref.Add(refValue);
  stack.Push(StackValue::Create(ref));

  DArray temp0, temp1;
  stack[0].Operand().GetValue(temp0);
  stack[1].Operand().GetValue(temp1);

  if (!temp0.IsNull()
      || temp1.IsNull())
  {
    return false;
  }

  DBS_T tempVal;
  temp1.Get(0, tempVal);
  if (tempVal != refValue)
    return false;

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  stack[0].Operand().GetValue(temp0);
  if (temp0.IsNull())
  {
    return false;
  }

  tempVal = DBS_T();
  temp0.Get(0, tempVal);
  if (tempVal != refValue)
    return false;

  return true;
}


bool
test_op_stud_text(Session& session, const DText& refValue)
{
  std::cout << "Testing text undefined assignments...\n";

  const uint32_t procId = session.FindProcedure(_RC(const uint8_t*, procName),
                                                sizeof procName - 1);
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STUD, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  GlobalValue gtext(TextOperand{DText{}});
  stack.Push(StackValue{GlobalOperand{gtext}});
  stack.Push(StackValue::Create(refValue));

  DText temp0, temp1;
  stack[0].Operand().GetValue(temp0);
  stack[1].Operand().GetValue(temp1);

  if (!temp0.IsNull()
      || temp1.IsNull())
  {
    return false;
  }

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  stack[0].Operand().GetValue(temp0);
  if (temp0.IsNull())
    return false;

  else if (temp0 != refValue)
    return false;

  return true;
}




static bool
test_op_stt(Session& session)
{
  std::cout << "Testing text assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DText op;
  DText op2(_RC(const uint8_t*, "Testing the best way to future!"));

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STT, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DText result;
  stack[0].Operand().GetValue(result);

  if (result != op2)
    return false;

  return true;
}

static bool
test_op_stta(Session& session)
{
  std::cout << "Testing table assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  const DUInt32 firstVal(0x31);
  const DUInt32 secondVal(0x32);

  DBSFieldDescriptor fd = {"first_field",  T_UINT32, false};

  ITable& firstTable = session.DBSHandler().CreateTempTable(1, &fd);
  ITable& secondTable = session.DBSHandler().CreateTempTable(1, &fd);

  secondTable.Set(secondTable.GetReusableRow(true), 0, firstVal);
  secondTable.Set(secondTable.GetReusableRow(true), 0, secondVal);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STTA, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);


  stack.Push(firstTable);
  stack.Push(secondTable);

  if (stack[0].Operand().IsNull() == false)
    return false;

  if (stack[1].Operand().IsNull())
    return false;

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  ITable& result = stack[0].Operand().GetTable();

  if (&result != &secondTable)
    return false;

  return true;
}

static bool
test_op_stf(Session& session)
{
  std::cout << "Testing field assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  const DUInt32 firstVal(0x31);
  const DUInt32 secondVal(0x32);

  DBSFieldDescriptor fd = {"first_field",  T_UINT32, false};

  ITable& firstTable = session.DBSHandler().CreateTempTable(1, &fd);
  TableOperand tableOp(firstTable, true);

  FieldOperand op;
  FieldOperand op2(tableOp, 0);

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STF, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(StackValue(op));
  stack.Push(StackValue(op2));

  if (stack[0].Operand().IsNull() == false)
    return false;

  if (stack[1].Operand().IsNull())
    return false;

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  ITable& result = stack[0].Operand().GetTable();

  if (&result != &firstTable)
    return false;

  if (stack[0].Operand().GetField() != 0)
    return false;

  return true;
}

static bool
test_op_sta(Session& session)
{
  std::cout << "Testing array assignment...\n";

  const uint32_t procId = session.FindProcedure(
                                              _RC(const uint8_t*, procName),
                                              sizeof procName - 1
                                                );
  const Procedure& proc   = session.GetProcedure(procId);
  uint8_t* testCode = _CC(uint8_t*, proc.mProcMgr->Code(proc, nullptr));
  SessionStack stack;

  DArray op;
  DArray op2;

  op2.Add(DUInt8(11));
  op2.Add(DUInt8(12));

  uint8_t opSize = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode);
  testCode[opSize++] = 0;
  opSize += w_encode_opcode(W_LDLO8, testCode + opSize);
  testCode[opSize++] = 1;
  opSize += w_encode_opcode(W_STA, testCode + opSize);
  w_encode_opcode(W_RET, testCode + opSize);

  stack.Push(op);
  stack.Push(op2);

  session.ExecuteProcedure(procName, stack);

  if (stack.Size() != 1)
    return false;

  if (stack[0].Operand().IsNull())
    return false;

  DArray result;
  stack[0].Operand().GetValue(result);

  if (result.Count() != op2.Count())
    return false;

  for (uint_t index = 0; index < result.Count(); ++index)
    {
      DUInt8 first, second;

      result.Get(index, first);
      op2.Get(index, second);

      if (first != second)
        return false;
    }

  return true;
}

int
main()
{
  bool success = true;

  {
    DBSInit(DBSSettings());
  }

  DBSCreateDatabase(admin);
  InitInterpreter();

  {
    ISession& commonSession = GetInstance(nullptr);

    CompiledBufferUnit dummy(dummyProgram,
                               sizeof dummyProgram,
                               my_postman,
                               dummyProgram);

    commonSession.LoadCompiledUnit(dummy);

    success = success && test_op_stb(_SC(Session&, commonSession));
    success = success && test_op_stc(_SC(Session&, commonSession));
    success = success && test_op_std(_SC(Session&, commonSession));
    success = success && test_op_stdt(_SC(Session&, commonSession));
    success = success && test_op_stht(_SC(Session&, commonSession));
    success = success && test_op_stXX<DInt8> (
                                          _SC(Session&, commonSession),
                                          W_STI8,
                                          "8 bit integer"
                                                );
    success = success && test_op_stXX<DInt16> (
                                          _SC(Session&, commonSession),
                                          W_STI16,
                                          "16 bit integer"
                                                 );
    success = success && test_op_stXX<DInt32> (
                                          _SC(Session&, commonSession),
                                          W_STI32,
                                          "32 bit integer"
                                                 );
    success = success && test_op_stXX<DInt64> (
                                          _SC(Session&, commonSession),
                                          W_STI64,
                                          "64 bit integer"
                                                 );
    success = success && test_op_stXX<DUInt8> (
                                          _SC(Session&, commonSession),
                                          W_STUI8,
                                          "8 bit unsigned integer"
                                                );
    success = success && test_op_stXX<DUInt16> (
                                          _SC(Session&, commonSession),
                                          W_STUI16,
                                          "16 bit unsigned integer"
                                                 );
    success = success && test_op_stXX<DUInt32> (
                                          _SC(Session&, commonSession),
                                          W_STUI32,
                                          "32 bit unsigned integer"
                                                 );
    success = success && test_op_stXX<DUInt64> (
                                           _SC(Session&, commonSession),
                                           W_STUI64,
                                           "64 bit unsigned integer"
                                                 );

    success = success && test_op_stXX<DReal> (
                                           _SC(Session&, commonSession),
                                           W_STR,
                                           "real"
                                                );

    success = success && test_op_stXX<DRichReal> (
                                           _SC(Session&, commonSession),
                                           W_STRR,
                                           "rich real"
                                                   );
    success = success && test_op_stt(_SC(Session&, commonSession));
    success = success && test_op_stta(_SC(Session&, commonSession));
    success = success && test_op_stf(_SC(Session&, commonSession));
    success = success && test_op_sta(_SC(Session&, commonSession));


    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DBool(true),
                                      "bool");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DChar(true),
                                      "char");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DDate(1989,12,24),
                                      "date");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DDateTime(1996, 9, 15, 9, 30, 0),
                                      "datetime");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DHiresTime(1944,8,23,10,11,10,12334),
                                      "hirestime");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DInt8(-3),
                                      "int8");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DInt16(-8900),
                                      "int16");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DInt32(-9901223),
                                      "int32");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DInt64(-9000129901223),
                                      "int64");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DUInt8(3),
                                      "uint8");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DUInt16(8900),
                                      "uint16");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DUInt32(9901223),
                                      "uint32");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DUInt64(9000129901223),
                                      "uint64");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DReal(0.1),
                                      "real");

    success = success && test_op_stud(_SC(Session&, commonSession),
                                      DRichReal(-0.122231123341),
                                      "richreal");

    success = success && test_op_stud_text(_SC(Session&, commonSession), DText("Simple test!"));


    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DBool(true),
                                      "bool");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DChar(true),
                                      "char");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DDate(1989,12,24),
                                      "date");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DDateTime(1996, 9, 15, 9, 30, 0),
                                      "datetime");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DHiresTime(1944,8,23,10,11,10,12334),
                                      "hirestime");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DInt8(-3),
                                      "int8");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DInt16(-8900),
                                      "int16");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DInt32(-9901223),
                                      "int32");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DInt64(-9000129901223),
                                      "int64");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DUInt8(3),
                                      "uint8");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DUInt16(8900),
                                      "uint16");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DUInt32(9901223),
                                      "uint32");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DUInt64(9000129901223),
                                      "uint64");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DReal(0.1),
                                      "real");

    success = success && test_op_stud_array(_SC(Session&, commonSession),
                                      DRichReal(-0.122231123341),
                                      "richreal");


    ReleaseInstance(commonSession);
  }

  CleanInterpreter();
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
