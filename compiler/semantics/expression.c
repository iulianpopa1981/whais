/******************************************************************************
 WHISPERC - A compiler for whisper programs
 Copyright (C) 2009  Iulian Popa

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

#include "whisper.h"
#include "whisperc/wopcodes.h"

#include "expression.h"
#include "wlog.h"
#include "statement.h"
#include "vardecl.h"
#include "opcodes.h"
#include "procdecl.h"


YYSTYPE
create_exp_link (struct ParserState* pState,
		 YYSTYPE             firstOp,
		 YYSTYPE             secondOp,
		 YYSTYPE             thirdOp,
		 enum EXP_OPERATION  op)
{
  struct SemValue *const result = get_sem_value (pState);

  if (result == NULL)
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return NULL;
    }

  result->val_type            = VAL_EXP_LINK;
  result->val.u_exp.pFirstOp  = firstOp;
  result->val.u_exp.pSecondOp = secondOp;
  result->val.u_exp.pThirdOp  = thirdOp;
  result->val.u_exp.opcode = op;

  return result;
}

static D_BOOL
is_unsigned (D_UINT type)
{
  return ((type >= T_UINT8) && (type <= T_UINT64));
}

static D_BOOL
is_signed (D_UINT type)
{
  return ((type >= T_INT8) && (type <= T_INT64));
}

static D_BOOL
is_integer (D_UINT type)
{
  return is_unsigned (type) || is_signed (type);
}

struct ExpResultType
{
  const struct DeclaredVar *extra;	/* only for composite types */
  D_UINT type;
};

static struct ExpResultType
translate_exp_tree (struct ParserState *const state,
		    struct Statement *const statement,
		    struct SemExpression *const call_exp);

static D_BOOL
is_leaf_exp (struct SemExpression *exp)
{
  return (exp->opcode == OP_NULL) && (exp->pSecondOp == NULL);
}

static const struct DeclaredVar *
find_field (const D_CHAR * const label,
	    const D_UINT l_label, const struct DeclaredVar *fields)
{

  assert (fields != NULL);
  while ((fields->type & T_FIELD_MASK) != 0)
    {
      if ((fields->l_label == l_label) &&
	  (memcmp (fields->label, label, l_label) == 0))
	{
	  break;
	}
      fields = fields->extra;
    }

  if ((fields->type & T_FIELD_MASK) == 0)
    {
      /* the field was not found */
      return NULL;
    }

  return fields;
}

static D_BOOL
are_compatible_fields (const struct DeclaredVar *const dst,
		       const struct DeclaredVar *const src)
{
  const D_UINT d_type = dst->type & ~(T_FIELD_MASK | T_ARRAY_MASK);
  const D_UINT s_type = src->type & ~(T_FIELD_MASK | T_ARRAY_MASK);

  assert ((dst->type & src->type & T_FIELD_MASK) != 0);

  if ((dst->type & T_ARRAY_MASK) != (src->type & T_ARRAY_MASK))
    return FALSE;
  else if ((d_type == T_UNDETERMINED) && (dst->type & T_ARRAY_MASK))
    return TRUE;
  else if (W_NA == store_op[d_type][s_type])
    return FALSE;

  return TRUE;
}

static const D_CHAR *
type_to_text (D_UINT type)
{
  type &= ~T_L_VALUE;

  if (type == T_BOOL)
    {
      return "BOOL";
    }
  else if (type == T_CHAR)
    {
      return "CHARACTER";
    }
  else if (type == T_DATE)
    {
      return "DATE";
    }
  else if (type == T_DATETIME)
    {
      return "DATETIME";
    }
  else if (type == T_HIRESTIME)
    {
      return "HIRESTIME";
    }
  else if (type == T_INT8)
    {
      return "INT8";
    }
  else if (type == T_INT16)
    {
      return "INT16";
    }
  else if (type == T_INT32)
    {
      return "INT32";
    }
  else if (type == T_INT64)
    {
      return "INT64";
    }
  else if (type == T_REAL)
    {
      return "REAL";
    }
  else if (type == T_RICHREAL)
    {
      return "RICHREAL";
    }
  else if (type == T_TEXT)
    {
      return "TEXT";
    }
  else if (type == T_UINT8)
    {
      return "UNSIGNED INT8";
    }
  else if (type == T_UINT16)
    {
      return "UNSIGNED INT16";
    }
  else if (type == T_UINT32)
    {
      return "UNSIGNED INT32";
    }
  else if (type == T_UINT64)
    {
      return "UNSIGNED INT64";
    }
  else if (type & T_ARRAY_MASK)
    {
      return "ARRAY";
    }
  else if (type & T_TABLE_MASK)
    {
      return "TABLE";
    }

  assert (0);
  return NULL;
}

static struct ExpResultType
translate_exp_inc (struct ParserState *state,
		   const struct ExpResultType *const ft)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  D_UINT type = ft->type & ~T_L_VALUE;
  enum W_OPCODE opcode = W_NA;

  assert (stmt->type == STMT_PROC);

  if ((ft->type & T_L_VALUE) == 0)
    {
      w_log_msg (state, state->bufferPos, MSG_INC_ELV);
      return r_unk;
    }

  if (type < T_END_OF_TYPES)
    opcode = inc_op[type];

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_INC_NA, type_to_text (type));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  return *ft;
}

static struct ExpResultType
translate_exp_dec (struct ParserState *state,
		   const struct ExpResultType *const ft)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  D_UINT type = ft->type & ~T_L_VALUE;
  enum W_OPCODE opcode = W_NA;

  assert (stmt->type == STMT_PROC);

  if ((ft->type & T_L_VALUE) == 0)
    {
      w_log_msg (state, state->bufferPos, MSG_DEC_ELV);
      return r_unk;
    }

  if (type < T_END_OF_TYPES)
    opcode = dec_op[type];

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_DEC_NA, type_to_text (type));

      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  return *ft;
}

static struct ExpResultType
translate_exp_not (struct ParserState *state,
		   const struct ExpResultType *const ft)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT type = ft->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  assert (stmt->type == STMT_PROC);

  if (type < T_END_OF_TYPES)
    {
      opcode = not_op[type];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_NOT_NA, type_to_text (type));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  result.extra = ft->extra, result.type = type;
  return result;
}

static struct ExpResultType
translate_exp_add (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = add_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_ADD_NA, type_to_text (ftype),
		 type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  switch (opcode)
    {
    case W_ADD:
      if (is_unsigned (ftype) != is_unsigned (stype))
	{
	  w_log_msg (state, state->bufferPos, MSG_ADD_SIGN);
	  result.type = T_INT64;
	}
      else if (is_unsigned (ftype))
	{
	  result.type = T_UINT64;
	}
      else
	{
	  result.type = T_INT64;
	}
      break;
    case W_ADDR:
      result.type = T_REAL;
      break;
    case W_ADDRR:
      result.type = T_RICHREAL;
      break;
    case W_ADDT:
      result.type = T_TEXT;
      break;
    default:
      assert (0);
    }
  /* this operator can not be used with container types */
  assert ((ft->extra == NULL) && (st->extra == NULL));

  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_sub (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = sub_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos,
		 MSG_SUB_NA, type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  switch (opcode)
    {
    case W_SUB:
      if (is_unsigned (ftype) != is_unsigned (stype))
	{
	  w_log_msg (state, state->bufferPos, MSG_SUB_SIGN);
	  result.type = T_INT64;
	}
      else if (is_unsigned (ftype))
	{
	  result.type = T_UINT64;
	}
      else
	{
	  result.type = T_INT64;
	}
      break;
    case W_SUBR:
      result.type = T_REAL;
      break;
    case W_SUBRR:
      result.type = T_RICHREAL;
      break;
    default:
      assert (0);
    }
  /* this operator can not be used with container types */
  assert ((ft->extra == NULL) && (st->extra == NULL));

  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_mul (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = mul_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos,
		 MSG_MUL_NA, type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  switch (opcode)
    {
    case W_MUL:
      if (is_unsigned (ftype) != is_unsigned (stype))
	{
	  w_log_msg (state, state->bufferPos, MSG_MUL_SIGN);
	  result.type = T_INT64;
	}
      else if (is_unsigned (ftype))
	{
	  result.type = T_UINT64;
	}
      else
	{
	  result.type = T_INT64;
	}
      break;
    case W_MULR:
      result.type = T_REAL;
      break;
    case W_MULRR:
      result.type = T_RICHREAL;
      break;
    default:
      assert (0);
    }
  /* this operator can not be used with container types */
  assert ((ft->extra == NULL) && (st->extra == NULL));

  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_div (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = div_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_DIV_NA, type_to_text (ftype),
		 type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  switch (opcode)
    {
    case W_DIV:
      if (is_unsigned (ftype) != is_unsigned (stype))
	{
	  w_log_msg (state, state->bufferPos, MSG_DIV_SIGN);
	  result.type = T_INT64;
	}
      else if (is_unsigned (ftype))
	{
	  result.type = T_UINT64;
	}
      else
	{
	  result.type = T_INT64;
	}
      break;
    case W_DIVR:
      result.type = T_REAL;
      break;
    case W_DIVRR:
      result.type = T_RICHREAL;
      break;
    default:
      assert (0);
    }
  /* this operator can not be used with container types */
  assert ((ft->extra == NULL) && (st->extra == NULL));

  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_mod (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = mod_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_MOD_NA, type_to_text (ftype),
		 type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert (opcode == W_MOD);

  result.type = T_UINT64;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_less (struct ParserState *state,
		    const struct ExpResultType *const ft,
		    const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = less_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_LT_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_LT) || (opcode == W_LTR) || (opcode == W_LTRR) ||
	  (opcode == W_LTC) || (opcode == W_LTD) || (opcode == W_LTDT) ||
	  (opcode == W_LTHT) || (opcode == W_LTR) || (opcode == W_LTRR) ||
	  (opcode == W_LTT));

  result.type = T_BOOL;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_less_equal (struct ParserState *state,
			  const struct ExpResultType *const ft,
			  const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = less_eq_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_LE_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_LE) || (opcode == W_LER) || (opcode == W_LERR) ||
	  (opcode == W_LEC) || (opcode == W_LED) || (opcode == W_LEDT) ||
	  (opcode == W_LEHT) || (opcode == W_LER) || (opcode == W_LERR) ||
	  (opcode == W_LET));

  result.type = T_BOOL;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_grater (struct ParserState *state,
		      const struct ExpResultType *const ft,
		      const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = grater_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_GT_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_GT) || (opcode == W_GTR) || (opcode == W_GTRR) ||
	  (opcode == W_GTC) || (opcode == W_GTD) || (opcode == W_GTDT) ||
	  (opcode == W_GTHT) || (opcode == W_GTR) || (opcode == W_GTRR) ||
	  (opcode == W_GTT));
  result.type = T_BOOL;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_grater_equal (struct ParserState *state,
			    const struct ExpResultType *const ft,
			    const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = grater_eq_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_GE_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_GE) || (opcode == W_GER) || (opcode == W_GERR) ||
	  (opcode == W_GEC) || (opcode == W_GED) || (opcode == W_GEDT) ||
	  (opcode == W_GEHT) || (opcode == W_GER) || (opcode == W_GERR) ||
	  (opcode == W_GET));

  result.type = T_BOOL;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_equals (struct ParserState *state,
		      const struct ExpResultType *const ft,
		      const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = equals_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_EQ_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_EQ) || (opcode == W_EQR) || (opcode == W_EQRR) ||
	  (opcode == W_EQC) || (opcode == W_EQD) || (opcode == W_EQDT) ||
	  (opcode == W_EQHT) || (opcode == W_EQR) || (opcode == W_EQRR) ||
	  (opcode == W_EQB) || (opcode == W_EQT));

  result.type = T_BOOL;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_not_equals (struct ParserState *state,
			  const struct ExpResultType *const ft,
			  const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = not_equals_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_NE_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_NE) || (opcode == W_NER) || (opcode == W_NERR) ||
	  (opcode == W_NEC) || (opcode == W_NED) || (opcode == W_NEDT) ||
	  (opcode == W_NEHT) || (opcode == W_NER) || (opcode == W_NERR) ||
	  (opcode == W_NEB) || (opcode == W_NET));

  result.type = T_BOOL;
  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_or (struct ParserState *state,
		  const struct ExpResultType *const ft,
		  const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = or_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_OR_NA,
		 type_to_text (ftype), type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  assert ((opcode == W_OR) || (opcode == W_ORB));

  if (opcode == W_OR)
    {
      assert (is_integer (stype));
      result.type = T_UINT64;
    }
  else
    {
      assert ((ftype == T_BOOL) && (stype == T_BOOL) && (opcode == W_ORB));
      result.type = T_BOOL;
    }

  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_and (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = and_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_AND_NA, type_to_text (ftype),
		 type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  if (opcode == W_AND)
    {
      assert (is_integer (stype));
      result.type = T_UINT64;
    }
  else
    {
      assert ((ftype == T_BOOL) && (stype == T_BOOL) && (opcode == W_ANDB));
      result.type = T_BOOL;
    }

  result.extra = NULL;
  return result;
}

static struct ExpResultType
translate_exp_xor (struct ParserState *state,
		   const struct ExpResultType *const ft,
		   const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ftype < T_END_OF_TYPES) && (stype < T_END_OF_TYPES))
    {
      opcode = xor_op[ftype][stype];
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_XOR_NA, type_to_text (ftype),
		 type_to_text (stype));
      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  if (opcode == W_XOR)
    {
      assert (is_integer (stype));
      result.type = T_UINT64;
    }
  else
    {
      assert ((ftype == T_BOOL) && (stype == T_BOOL) && (opcode == W_XORB));
      result.type = T_BOOL;
    }

  result.extra = NULL;
  return result;
}

static D_BOOL
are_compatible_tables (struct ParserState *const         pState,
                       const struct ExpResultType *const pFirstType,
                       const struct ExpResultType *const pSecondType,
                       D_BOOL                            ignore_unfound_fields)
{
  const struct DeclaredVar *dst_field = NULL;
  const struct DeclaredVar *src_field = NULL;

  if ((pFirstType->type & T_TABLE_MASK) != (pSecondType->type & T_TABLE_MASK))
    {
      w_log_msg (pState, pState->bufferPos, MSG_CONTAINER_NA);
      return FALSE;
    }

  dst_field = pFirstType->extra;
  if (dst_field == NULL)
    return TRUE;

  src_field = pSecondType->extra;

  while ((dst_field->type & T_FIELD_MASK) != 0)
    {
      D_CHAR temp[128];
      const struct DeclaredVar *found = find_field (dst_field->label,
						    dst_field->l_label,
						    src_field);

      assert ((dst_field->type & T_FIELD_MASK) != 0);
      assert ((src_field == NULL) || ((src_field->type & T_FIELD_MASK) != 0));

      if (found == NULL)
	{
	  if (!ignore_unfound_fields)
	    {
	      w_log_msg (pState, pState->bufferPos, MSG_NO_FIELD,
			 copy_text_truncate (temp, dst_field->label,
					     sizeof temp,
					     dst_field->l_label));

	      return FALSE;
	    }
	}
      else if (are_compatible_fields (dst_field, found) == FALSE)
	{
	  w_log_msg (pState, pState->bufferPos, MSG_FIELD_NA,
		     copy_text_truncate (temp, dst_field->label,
					 sizeof temp, dst_field->l_label),
		     type_to_text (dst_field->type & ~T_FIELD_MASK),
		     type_to_text (found->type & ~T_FIELD_MASK));
	  pState->abortError = TRUE;
	  return FALSE;
	}
      dst_field = dst_field->extra;
    }

  return TRUE;
}

static struct ExpResultType
translate_exp_store (struct ParserState *const state,
		     const struct ExpResultType *const ft,
		     const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result;
  enum W_OPCODE opcode = W_NA;

  if ((ft->type & T_L_VALUE) == 0)
    {
      w_log_msg (state, state->bufferPos, MSG_STORE_ELV);

      return r_unk;
    }

  if (stype != T_UNDETERMINED)
    {

      if ((ftype < T_UNDETERMINED) && (stype < T_UNDETERMINED))
	{
	  opcode = store_op[ftype][stype];
	}
      else if (((ftype & T_TABLE_MASK) != 0) && ((stype & T_TABLE_MASK) != 0))
	{
	  if (!are_compatible_tables (state, ft, st, TRUE))
	    {
	      return r_unk;
	    }
	  opcode = W_STTA;
	}
      else if (((ftype & T_ARRAY_MASK) != 0) && ((stype & T_ARRAY_MASK) != 0))
	{
	  const D_UINT temp_ftype = ftype & ~T_ARRAY_MASK;
	  const D_UINT temp_stype = stype & ~T_ARRAY_MASK;

	  assert (temp_ftype <= T_UNDETERMINED);
	  assert (temp_stype <= T_UNDETERMINED);

	  if ((temp_ftype == T_UNDETERMINED) ||
	      (W_NA != store_op[temp_ftype][temp_stype]))
	    {
	      opcode = W_STA;
	    }
	}
    }
  else
    {
      /* a NULL value is assigned */
      if (ftype < T_UNDETERMINED)
	{
	  opcode = store_op[ftype][ftype];
	}
      else if (ftype & T_TABLE_MASK)
	{
	  opcode = W_STTA;
	}
      else if (ftype & T_ARRAY_MASK)
	{
	  opcode = W_STA;
	}
    }

  if (opcode == W_NA)
    {
      w_log_msg (state, state->bufferPos, MSG_STORE_NA,
		 type_to_text (ftype), type_to_text (stype));

      return r_unk;
    }

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  result = *ft;

  return result;

}

static struct ExpResultType
translate_exp_index (struct ParserState *const state,
		     const struct ExpResultType *const ft,
		     const struct ExpResultType *const st)
{
  struct Statement *const stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (stmt);
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const D_UINT ftype = ft->type & ~T_L_VALUE;
  const D_UINT stype = st->type & ~T_L_VALUE;
  struct ExpResultType result = r_unk;
  enum W_OPCODE opcode = W_NA;

  if (((ftype & T_ARRAY_MASK) == 0) &&
      ((ftype & T_TABLE_MASK) == 0) &&
      ((ftype != T_TEXT)))
    {
      w_log_msg (state, state->bufferPos, MSG_INDEX_EAT,
		 type_to_text (ftype));

      return r_unk;
    }

  if (!is_integer (stype))
    {
      w_log_msg (state, state->bufferPos, MSG_INDEX_ENI,
		 type_to_text (stype));

      return r_unk;
    }
  else if ((ftype & T_ARRAY_MASK) != 0)
    {
      assert (ft->extra == NULL);

      opcode = W_INDA;
      result.type = (ftype & ~T_ARRAY_MASK);
      if (result.type == T_UNDETERMINED)
	{
	  w_log_msg (state, state->bufferPos, MSG_INDEX_UNA);
	  state->abortError = TRUE;
	  return r_unk;
	}
      result.type |= T_L_VALUE;
    }
  else
    {
      assert (ftype == T_TEXT);

      opcode = W_INDT;
      result.type = T_CHAR;
    }

  assert (opcode != W_NA);

  if (w_opcode_encode (instrs, opcode) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return r_unk;
    }

  return result;
}

static struct ExpResultType
translate_exp_op (struct ParserState *const state,
		  struct Statement *const statement,
		  const D_UINT16 op,
		  const struct ExpResultType *first_type,
		  const struct ExpResultType *second_type)
{
  struct ExpResultType result = { NULL, T_UNKNOWN };
  switch (op)
    {
    case OP_ADD:
      result = translate_exp_add (state, first_type, second_type);
      break;
    case OP_SUB:
      result = translate_exp_sub (state, first_type, second_type);
      break;
    case OP_MUL:
      result = translate_exp_mul (state, first_type, second_type);
      break;
    case OP_DIV:
      result = translate_exp_div (state, first_type, second_type);
      break;
    case OP_MOD:
      result = translate_exp_mod (state, first_type, second_type);
      break;
    case OP_LT:
      result = translate_exp_less (state, first_type, second_type);
      break;
    case OP_LE:
      result = translate_exp_less_equal (state, first_type, second_type);
      break;
    case OP_GT:
      result = translate_exp_grater (state, first_type, second_type);
      break;
    case OP_GE:
      result = translate_exp_grater_equal (state, first_type, second_type);
      break;
    case OP_EQ:
      result = translate_exp_equals (state, first_type, second_type);
      break;
    case OP_NE:
      result = translate_exp_not_equals (state, first_type, second_type);
      break;
    case OP_INC:
      result = translate_exp_inc (state, first_type);
      break;
    case OP_DEC:
      result = translate_exp_dec (state, first_type);
      break;
    case OP_NOT:
      result = translate_exp_not (state, first_type);
      break;
    case OP_OR:
      result = translate_exp_or (state, first_type, second_type);
      break;
    case OP_AND:
      result = translate_exp_and (state, first_type, second_type);
      break;
    case OP_XOR:
      result = translate_exp_xor (state, first_type, second_type);
      break;
    case OP_GROUP:
      result = *first_type;
      break;
    case OP_INDEX:
      result = translate_exp_index (state, first_type, second_type);
      break;
    case OP_ATTR:
      result = translate_exp_store (state, first_type, second_type);
      break;
    default:
      assert (0);
    }

  return result;
}

static struct ExpResultType
translate_exp_leaf (struct ParserState *const state,
		    struct Statement *const statement,
		    struct SemValue *const exp)
{
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  const struct ExpResultType r_undet = { NULL, T_UNDETERMINED };

  struct OutStream *const instrs = stmt_query_instrs (statement);
  struct ExpResultType result = r_unk;

  if (exp == NULL)
    {
      if (w_opcode_encode (instrs, W_LDNULL) == NULL)
	{
	  w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	  return r_unk;
	}
      return r_undet;
    }

  if (exp->val_type == VAL_ID)
    {
      struct Statement *stmt = state->pCurrentStmt;
      struct DeclaredVar *var = stmt_find_declaration (stmt,
						       exp->val.u_id.text,
						       exp->val.u_id.length,
						       TRUE,
						       TRUE);
      D_UINT32 value;
      D_UINT32 value_32 = ~0;
      D_UINT16 value_16 = ~0;
      D_UINT8 value_8 = ~0;
      enum W_OPCODE op_code = W_NA;

      if (var == NULL)
	{
	  D_CHAR temp[128];
	  copy_text_truncate (temp, exp->val.u_id.text,
			      sizeof temp, exp->val.u_id.length);
	  w_log_msg (state, state->bufferPos, MSG_VAR_NFOUND, temp);
	  state->abortError = TRUE;
	  return r_unk;
	}

      value = RETRIVE_ID (var->var_id);
      if (var->var_id & GLOBAL_DECL)
	{
	  if (stmt->pParentStmt != NULL)
	    {
	      assert (stmt->type == STMT_PROC);
	      stmt = stmt->pParentStmt;
	    }
	  assert (stmt->type == STMT_GLOBAL);
	}

      result.extra = var->extra;
      result.type = var->type | T_L_VALUE;

      if (value <= value_8)
	{
	  value_8 = value;
	  op_code = (stmt->type == STMT_GLOBAL) ? W_LDGB8 : W_LDLO8;
	  if ((w_opcode_encode (instrs, op_code) == NULL) ||
	      uint8_outstream (instrs, value_8) == NULL)
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
      else if (value <= value_16)
	{
	  value_16 = value;
	  op_code = (stmt->type == STMT_GLOBAL) ? W_LDGB16 : W_LDLO16;
	  if ((w_opcode_encode (instrs, op_code) == NULL) ||
	      uint16_outstream (instrs, value_16) == NULL)
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
      else
	{
	  value_32 = value;
	  op_code = (stmt->type == STMT_GLOBAL) ? W_LDGB32 : W_LDLO32;
	  if ((w_opcode_encode (instrs, op_code) == NULL)
	      || uint32_outstream (instrs, value_32) == NULL)
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
    }
  else if (exp->val_type == VAL_C_INT)
    {
      const D_UINT32 value_32 = ~0;
      const D_UINT16 value_16 = ~0;
      const D_UINT8 value_8 = ~0;

      if (exp->val.u_int.value <= value_8)
	{
	  result.type = exp->val.u_int.is_signed ? T_INT8 : T_UINT8;
	  if ((w_opcode_encode (instrs, W_LDI8) == NULL) ||
	      (uint8_outstream
	       (instrs, (D_UINT8) exp->val.u_int.value & value_8) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
      else if (exp->val.u_int.value <= value_16)
	{
	  result.type = exp->val.u_int.is_signed ? T_INT16 : T_UINT16;
	  if ((w_opcode_encode (instrs, W_LDI16) == NULL)
	      ||
	      (uint16_outstream
	       (instrs, (D_UINT16) exp->val.u_int.value & value_16) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
      else if (exp->val.u_int.value <= value_32)
	{
	  result.type = exp->val.u_int.is_signed ? T_INT32 : T_UINT32;
	  if ((w_opcode_encode (instrs, W_LDI32) == NULL)
	      ||
	      (uint32_outstream
	       (instrs, (D_UINT32) exp->val.u_int.value & value_32) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
      else
	{
	  result.type = exp->val.u_int.is_signed ? T_INT64 : T_UINT64;
	  if ((w_opcode_encode (instrs, W_LDI64) == NULL)
	      || (uint64_outstream (instrs, (D_UINT64) exp->val.u_int.value)
		  == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
    }
  else if (exp->val_type == VAL_C_CHAR)
    {
      result.type = T_CHAR;
      if ((w_opcode_encode (instrs, W_LDC) == NULL) ||
	  (data_outstream (instrs,
			   (D_UINT8 *) & exp->val.u_char.value,
			   sizeof (exp->val.u_char.value)) == NULL))
	{
	  w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	  return r_unk;
	}
    }
  else if (exp->val_type == VAL_C_TIME)
    {
      struct SemCTime *const value = &exp->val.u_time;

      if (value->usec != 0)
	{
	  result.type = T_HIRESTIME;
	  if ((w_opcode_encode (instrs, W_LDHT) == NULL) ||
	      (uint32_outstream (instrs, value->usec) == NULL) ||
	      (uint8_outstream (instrs, value->sec) == NULL) ||
	      (uint8_outstream (instrs, value->min) == NULL) ||
	      (uint8_outstream (instrs, value->hour) == NULL) ||
	      (uint8_outstream (instrs, value->day) == NULL) ||
	      (uint8_outstream (instrs, value->month) == NULL) ||
	      (uint16_outstream (instrs, value->year) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }

	}
      else if ((value->sec != 0) || (value->min != 0) || (value->hour != 0))
	{
	  result.type = T_DATETIME;
	  if ((w_opcode_encode (instrs, W_LDDT) == NULL) ||
	      (uint8_outstream (instrs, value->sec) == NULL) ||
	      (uint8_outstream (instrs, value->min) == NULL) ||
	      (uint8_outstream (instrs, value->hour) == NULL) ||
	      (uint8_outstream (instrs, value->day) == NULL) ||
	      (uint8_outstream (instrs, value->month) == NULL) ||
	      (uint16_outstream (instrs, value->year) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
      else
	{
	  result.type = T_DATE;
	  if ((w_opcode_encode (instrs, W_LDD) == NULL) ||
	      (uint8_outstream (instrs, value->day) == NULL) ||
	      (uint8_outstream (instrs, value->month) == NULL) ||
	      (uint16_outstream (instrs, value->year) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	}
    }
  else if (exp->val_type == VAL_C_REAL)
    {
      result.type = T_REAL;
      if ((w_opcode_encode (instrs, W_LDR) == NULL) ||
	  (uint64_outstream (instrs, exp->val.u_real.integerPart) == NULL) ||
	  (uint64_outstream (instrs, exp->val.u_real.fractionalPart) == NULL))

	{
	  w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	  return r_unk;
	}
    }
  else if (exp->val_type == VAL_C_TEXT)
    {
      struct SemCText *const value = &exp->val.u_text;
      D_INT32 const_pos = add_text_const (statement,
					  (const D_UINT8 *) value->text,
					  value->length);

      result.type = T_TEXT;
      if ((const_pos < 0) || (w_opcode_encode (instrs, W_LDT) == NULL) ||
	  (uint32_outstream (instrs, const_pos) == NULL))
	{
	  w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	  return r_unk;
	}
    }
  else if (exp->val_type == VAL_C_BOOL)
    {
      const enum W_OPCODE opcode = (exp->val.u_bool.value == FALSE) ?
	W_LDBF : W_LDBT;
      if (w_opcode_encode (instrs, opcode) == NULL)
	{
	  w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	  return r_unk;
	}
      result.type = T_BOOL;
    }
  else
    {
      assert (0);
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_INT_ERR);
      result.type = T_UNKNOWN;
    }

  exp->val_type = VAL_REUSE;
  return result;
}

static struct ExpResultType
translate_exp_call (struct ParserState *const state,
		    struct Statement *const statement,
		    struct SemExpression *const call_exp)
{
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  struct OutStream *const instrs = stmt_query_instrs (statement);
  const struct Statement *proc = NULL;
  const struct DeclaredVar *proc_arg = NULL;
  struct SemValue *exp_arg = NULL;
  struct ExpResultType result;
  D_UINT arg_count = 0;
  D_CHAR temp[128];

  assert (call_exp->pFirstOp->val_type == VAL_ID);
  assert ((call_exp->pSecondOp == NULL) ||
	  (call_exp->pSecondOp->val_type == VAL_PRC_ARG_LINK));

  proc = find_proc_decl (state,
			 call_exp->pFirstOp->val.u_id.text,
			 call_exp->pFirstOp->val.u_id.length,
			 TRUE);
  if (proc == NULL)
    {
      w_log_msg (state, state->bufferPos,
		 MSG_NO_PROC,
		 copy_text_truncate (temp, call_exp->pFirstOp->val.u_id.text,
				     sizeof temp,
				     call_exp->pFirstOp->val.u_id.length));

      return r_unk;
    }

  exp_arg = call_exp->pSecondOp;
  while (exp_arg != NULL)
    {
      struct SemValue *const param = exp_arg->val.u_args.expr;
      D_CHAR temp[128];
      struct ExpResultType arg_type;

      assert (exp_arg->val_type == VAL_PRC_ARG_LINK);
      assert (param->val_type == VAL_EXP_LINK);

      ++arg_count;
      proc_arg = stmt_get_param (proc, arg_count);

      if (proc_arg == NULL)
	{
	  w_log_msg (state,
	             state->bufferPos,
	             MSG_PROC_MORE_ARGS,
		     copy_text_truncate (temp,
					 call_exp->pFirstOp->val.u_id.text,
					 sizeof temp,
					 call_exp->pFirstOp->val.u_id.length),
		     stmt_get_param_count (proc));
	  state->abortError = TRUE;
	  return r_unk;
	}
      else
	{
	  /* convert the declared variable to an expression result */
	  arg_type.type = proc_arg->type;
	  arg_type.extra = proc_arg->extra;
	}

      result = translate_exp_tree (state, statement, &param->val.u_exp);
      if (result.type == T_UNKNOWN)
	{
	  /* An error that must be propagated upwards. The error message
	   * was logged during expression's evaluation. */
	  assert (state->abortError == TRUE);
	  w_log_msg (state, state->bufferPos,
		     MSG_PROC_ARG_COUNT,
		     copy_text_truncate (temp, proc->spec.proc.name,
					 sizeof temp,
					 proc->spec.proc.nameLength), arg_count);
	  return r_unk;
	}
      else
	{
	  param->val_type = VAL_REUSE;
	}

      if (result.type != T_UNDETERMINED)
	{
	  if ((result.type & T_TABLE_MASK) == 0)
	    {
	      const D_UINT arg_t = arg_type.type & ~(T_L_VALUE | T_ARRAY_MASK | T_TABLE_MASK);
	      const D_UINT res_t = result.type & ~(T_L_VALUE | T_ARRAY_MASK | T_TABLE_MASK);
	      const enum W_OPCODE temp_op = store_op[arg_t][res_t];

	      assert (arg_t <= T_UNDETERMINED);
	      assert (res_t <= T_UNDETERMINED);

	      if ((result.type & T_ARRAY_MASK) != (arg_type.type & T_ARRAY_MASK))
		{
		  w_log_msg (state, state->bufferPos,
			     MSG_PROC_ARG_NA,
			     copy_text_truncate (temp, proc->spec.proc.name,
						 sizeof temp,
						 proc->spec.proc.nameLength),
			     arg_count,
			     type_to_text (arg_type.type),
			     type_to_text (result.type));
		  state->abortError = TRUE;
		  return r_unk;
		}
	      else if (W_NA == temp_op)
		{
		  if ((arg_type.type & (T_UNDETERMINED | T_ARRAY_MASK)) !=
                      (T_UNDETERMINED | T_ARRAY_MASK))
		    {
		      w_log_msg (state, state->bufferPos,
				 MSG_PROC_ARG_NA,
				 copy_text_truncate (temp,
						     proc->spec.proc.name,
						     sizeof temp,
						     proc->spec.proc.nameLength),
				 arg_count,
				 type_to_text (arg_type.type),
				 type_to_text (result.type));

		      return r_unk;
		    }
		}
	    }
	  else
	    if (are_compatible_tables (state, &arg_type, &result, FALSE) == FALSE)
	    {
	      /* The two containers's types are not compatible.
	       * The error was already logged. */
	      w_log_msg (state, state->bufferPos,
			 MSG_PROC_ARG_COUNT,
			 copy_text_truncate (temp, proc->spec.proc.name,
					     sizeof temp,
					     proc->spec.proc.nameLength),
			 arg_count);

	      return r_unk;

	    }
	}

      exp_arg->val_type = VAL_REUSE;
      exp_arg = exp_arg->val.u_args.next;
    }

  if (arg_count < stmt_get_param_count (proc))
    {
      w_log_msg (state, state->bufferPos, MSG_PROC_LESS_ARGS,
		 copy_text_truncate (temp, call_exp->pFirstOp->val.u_id.text,
				     sizeof temp,
				     call_exp->pFirstOp->val.u_id.length),
		 stmt_get_param_count (proc), arg_count);

      return r_unk;
    }
  else
    {
      call_exp->pFirstOp->val_type = VAL_REUSE;
    }

  if ((w_opcode_encode (instrs, W_CALL) == NULL) ||
      (uint32_outstream (instrs, stmt_get_import_id (proc)) == NULL))
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  proc_arg = stmt_get_param (proc, 0);
  if (proc_arg == 0)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_INTERNAL_ERROR);
      return r_unk;
    }
  result.type = proc_arg->type;
  if ((proc_arg->extra != NULL) || ((proc_arg->type & T_FIELD_MASK) != 0))
    {
      /* for rows skip the pointing table */
      result.extra = proc_arg->extra->extra;
    }
  else
    {
      result.extra = proc_arg->extra;
    }

  return result;
}

static struct ExpResultType
translate_exp_field (struct ParserState* const   pState,
		     struct Statement* const     pStmt,
		     struct SemExpression* const pCallExp)
{
  const struct ExpResultType  r_unk      = { NULL, T_UNKNOWN };
  const struct DeclaredVar*   pVarField  = NULL;
  struct OutStream *const     pInstrs    = stmt_query_instrs (pState->pCurrentStmt);
  struct SemExpression* const pFirstExp  = &pCallExp->pFirstOp->val.u_exp;
  struct SemExpression* const pSecondExp = &pCallExp->pSecondOp->val.u_exp;
  struct SemId* const         pId        = &pCallExp->pThirdOp->val.u_id;
  struct ExpResultType        tableType;
  struct ExpResultType        expType;

  assert (pCallExp->opcode == OP_FIELD);
  assert (pCallExp->pFirstOp->val_type == VAL_EXP_LINK);
  assert (pCallExp->pSecondOp->val_type == VAL_EXP_LINK);
  assert (pCallExp->pThirdOp->val_type == VAL_ID);

  tableType = translate_exp_tree (pState, pStmt, pFirstExp);
  if ((tableType.type & T_TABLE_MASK) == 0)
    {
      w_log_msg (pState, pState->bufferPos, MSG_MEMSEL_NA, type_to_text (tableType.type));

      pState->abortError = TRUE;
      return r_unk;
    }

  expType = translate_exp_tree (pState, pStmt, pSecondExp);
  if (!is_integer (expType.type & ~T_L_VALUE))
    {
      w_log_msg (pState, pState->bufferPos, MSG_INDEX_ENI, type_to_text (expType.type));

      pState->abortError = TRUE;
      return r_unk;
    }

  if (tableType.extra != NULL)
    pVarField = find_field (pId->text, pId->length, tableType.extra);

  if (pVarField == NULL)
    {
      D_CHAR temp[128];
      copy_text_truncate (temp, pId->text, sizeof temp, pId->length);
      w_log_msg (pState, pState->bufferPos, MSG_MEMSEL_ERD, temp);

      pState->abortError = TRUE;
      return r_unk;
    }

  if ((w_opcode_encode (pInstrs, W_SELF) == NULL)||
      (data_outstream (pInstrs, (const D_UINT8*)pId->text, pId->length) == NULL) ||
      (uint8_outstream (pInstrs, 0) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);
      return r_unk;
    }

  expType.type  = (pVarField->type | T_L_VALUE) & ~T_FIELD_MASK;
  expType.extra = NULL;

  return expType;
}

static struct ExpResultType
translate_exp_tree (struct ParserState *const state,
		    struct Statement *const statement,
		    struct SemExpression *const call_exp)
{
  const struct ExpResultType r_unk = { NULL, T_UNKNOWN };
  struct OutStream *const instrs = stmt_query_instrs (state->pCurrentStmt);
  D_UINT8 *temp_buffer;
  D_INT jmp_position;
  D_INT jmp_data_pos;
  D_BOOL needs_jmp_correction = FALSE;
  struct ExpResultType first_type;
  struct ExpResultType second_type;

  assert (state->pCurrentStmt->type == STMT_PROC);

  if (is_leaf_exp (call_exp))
    {
      return translate_exp_leaf (state, statement, call_exp->pFirstOp);
    }
  else if (call_exp->opcode == OP_CALL)
    {
      /* procedure call */
      return translate_exp_call (state, statement, call_exp);
    }
  else if (call_exp->opcode == OP_FIELD)
    {
      return translate_exp_field (state, statement, call_exp);
    }

  assert (call_exp->pFirstOp->val_type == VAL_EXP_LINK);

  first_type = translate_exp_tree (state, statement,
				   &(call_exp->pFirstOp->val.u_exp));
  if (first_type.type == T_UNKNOWN)
    {
      /* something went wrong, and the error
       * should be already logged  */
      return r_unk;
    }
  call_exp->pFirstOp->val_type = VAL_REUSE;

  if ((first_type.type & ~T_L_VALUE) == T_BOOL)
    {
      /* Handle special case for OR or AND with boolean types not to evaluate
       * the second expression when is unnecessary!
       * Use 0 for jump offset just to reserve the space. It will be corrected
       * after we parse the second expression */
      jmp_position = get_size_outstream (instrs);
      if (call_exp->opcode == OP_OR)
	{
	  if ((w_opcode_encode (instrs, W_JT) == NULL) ||
	      (uint32_outstream (instrs, 0) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	  needs_jmp_correction = TRUE;
	}
      else if (call_exp->opcode == OP_AND)
	{
	  if ((w_opcode_encode (instrs, W_JF) == NULL) ||
	      (uint32_outstream (instrs, 0) == NULL))
	    {
	      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);
	      return r_unk;
	    }
	  needs_jmp_correction = TRUE;
	}
      jmp_data_pos = get_size_outstream (instrs) - sizeof (D_UINT32);
    }

  if (call_exp->pSecondOp != NULL)
    {
      assert (call_exp->pSecondOp->val_type == VAL_EXP_LINK);
      second_type = translate_exp_tree (state, statement,
					&(call_exp->pSecondOp->val.u_exp));
      if (second_type.type == T_UNKNOWN)
	{
	  /* something went wrong, and the error
	   * should be already signaled  */
	  return r_unk;
	}
      call_exp->pSecondOp->val_type = VAL_REUSE;
    }

  /* use second_type to store result */
  second_type = translate_exp_op (state, statement, call_exp->opcode, &first_type,
				  &second_type);

  if (needs_jmp_correction && ((second_type.type & ~T_L_VALUE) == T_BOOL))
    {
      /* lets correct some jumps offsets */
      D_INT current_pos = get_size_outstream (instrs) - jmp_position;
      const D_UINT8 *const data = (D_UINT8 *) & current_pos;
      temp_buffer = get_buffer_outstream (instrs);

      temp_buffer[jmp_data_pos + 0] = data[0];
      temp_buffer[jmp_data_pos + 1] = data[1];
      temp_buffer[jmp_data_pos + 2] = data[2];
      temp_buffer[jmp_data_pos + 3] = data[3];
    }

  return second_type;
}

YYSTYPE
translate_exp (struct ParserState * state, YYSTYPE exp)
{
  struct Statement *const curr_stmt = state->pCurrentStmt;
  struct OutStream *instrs = stmt_query_instrs (curr_stmt);

  assert (exp->val_type = VAL_EXP_LINK);
  assert (curr_stmt->type == STMT_PROC);

  translate_exp_tree (state, curr_stmt, &(exp->val.u_exp));
  exp->val_type = VAL_REUSE;
  if (w_opcode_encode (instrs, W_CTS) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);

    }

  return NULL;
}

YYSTYPE
translate_return_exp (struct ParserState * state, YYSTYPE exp)
{
  struct Statement *const curr_stmt = state->pCurrentStmt;
  struct OutStream *const instrs = stmt_query_instrs (curr_stmt);
  const struct DeclaredVar *const decl_ret = stmt_get_param (curr_stmt, 0);

  struct ExpResultType ret_type = { NULL, T_UNKNOWN };
  struct ExpResultType exp_type = { NULL, T_UNKNOWN };

  assert (exp->val_type = VAL_EXP_LINK);
  assert (curr_stmt->type == STMT_PROC);

  exp_type = translate_exp_tree (state, curr_stmt, &(exp->val.u_exp));
  if (exp_type.type == T_UNKNOWN)
    {
      /* some error has been encountered evaluating of return expression */
      assert (state->abortError != FALSE);
      return NULL;
    }
  exp->val_type = VAL_REUSE;

  /* convert the declared return type to an expression result type */
  ret_type.type = decl_ret->type;
  ret_type.extra = decl_ret->extra;

  if (exp_type.type != T_UNDETERMINED)
    {
      if ((ret_type.type & T_TABLE_MASK) != (exp_type.type & T_TABLE_MASK))
        {
          w_log_msg (state, state->bufferPos,
                     MSG_PROC_RET_NA_EXT,
                     type_to_text (ret_type.type),
                     type_to_text (exp_type.type));
          state->abortError = TRUE;
        }
      else if ((ret_type.type & T_TABLE_MASK) == 0)
	{
	  const D_UINT exp_t = exp_type.type & ~(T_L_VALUE | T_ARRAY_MASK);
	  const D_UINT ret_t = ret_type.type & ~(T_L_VALUE | T_ARRAY_MASK);
	  const enum W_OPCODE temp_op = store_op[ret_t][exp_t];

	  assert (exp_t <= T_UNDETERMINED);
	  assert (ret_t <= T_UNDETERMINED);

	  if ((ret_type.type & T_ARRAY_MASK) !=
	      (exp_type.type & T_ARRAY_MASK))
	    {
	      w_log_msg (state, state->bufferPos,
			 MSG_PROC_RET_NA_EXT,
			 type_to_text (ret_type.type),
			 type_to_text (exp_type.type));
	      state->abortError = TRUE;

	    }
	  else if (temp_op == W_NA)
	    {
	      if ((ret_type.type & (T_UNDETERMINED | T_ARRAY_MASK)) !=
		  (T_UNDETERMINED | T_ARRAY_MASK))
		{
		  w_log_msg (state, state->bufferPos,
			     MSG_PROC_RET_NA_EXT,
			     type_to_text (ret_type.type),
			     type_to_text (exp_type.type));
		  state->abortError = TRUE;
		}
	    }
	}
      else if (are_compatible_tables (state, &ret_type, &exp_type, FALSE) == FALSE)
	{
	  /* The two containers types are not compatible.
	   * The error was already logged. */
	  w_log_msg (state, state->bufferPos, MSG_PROC_RET_NA);
	  state->abortError = TRUE;
	}
    }

  if (w_opcode_encode (instrs, W_RET) == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);

    }
  return NULL;
}

D_BOOL
translate_bool_exp (struct ParserState * state, YYSTYPE exp)
{
  struct ExpResultType exp_type;

  assert (exp->val_type == VAL_EXP_LINK);
  exp_type =
    translate_exp_tree (state, state->pCurrentStmt, &(exp->val.u_exp));
  if (exp_type.type == T_UNKNOWN)
    {
      /* Some error was encounter evaluating expression.
       * The error should be already logged */
      assert (state->abortError == TRUE);
      return FALSE;
    }
  else if ((exp_type.type & ~T_L_VALUE) != T_BOOL)
    {
      w_log_msg (state, state->bufferPos, MSG_EXP_NOT_BOOL);

      return FALSE;
    }
  exp->val_type = VAL_REUSE;
  return TRUE;
}

YYSTYPE
create_arg_link (struct ParserState * state, YYSTYPE arg, YYSTYPE next)
{
  struct SemValue *const result = get_sem_value (state);

  assert ((arg != NULL) && (arg->val_type == VAL_EXP_LINK));
  assert ((next == NULL) || (next->val_type = VAL_PRC_ARG_LINK));

  if (result == NULL)
    {
      w_log_msg (state, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return NULL;
    }

  result->val_type = VAL_PRC_ARG_LINK;
  result->val.u_args.expr = arg;
  result->val.u_args.next = next;

  return result;
}
