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
#include <string.h>

#include "procdecl.h"
#include "wlog.h"
#include "vardecl.h"

struct Statement*
find_proc_decl (struct ParserState* parser,
                const char*         name,
                const uint_t        nameLength,
                const bool_t        refer)
{
  const struct WArray* procs  = &(parser->globalStmt.spec.glb.procsDecls);
  uint_t               procIt = wh_array_count (procs);

  while (procIt-- > 0)
    {
      struct Statement* result = (struct Statement*) wh_array_get (procs,
                                                                   procIt);
      assert (result->type == STMT_PROC);
      assert (result->spec.proc.name != NULL);
      assert (result->spec.proc.nameLength != 0);

      if ((result->spec.proc.nameLength == nameLength)
          && (strncmp (result->spec.proc.name, name, nameLength) == 0))
        {
          if (refer && ! IS_REFERRED (result->spec.proc.procId))
            {
              const uint32_t id = parser->globalStmt.spec.glb.procsCount++;

              assert (RETRIVE_ID (result->spec.proc.procId) == 0);
              assert (IS_EXTERNAL (result->spec.proc.procId));

              result->spec.proc.procId |= id;

              MARK_AS_REFERENCED (result->spec.proc.procId);
            }
          return result;
        }
    }
  return NULL;
}


YYSTYPE
add_proc_param_decl (YYSTYPE    paramsList,
                     YYSTYPE    id,
                     YYSTYPE    type)
{
  struct SemTypeSpec typeDesc = type->val.u_tspec;

  assert (paramsList == NULL || paramsList->val_type == VAL_PRCDCL_LIST);
  assert (id->val_type == VAL_ID);
  assert (type->val_type == VAL_TYPE_SPEC);

  /* reconvert s_type to a VAL_PRCDCL_LIST */
  type->val_type         = VAL_PRCDCL_LIST;
  type->val.u_prdcl.type = typeDesc;
  type->val.u_prdcl.id   = id->val.u_id;
  type->val.u_prdcl.next = paramsList;

  /* recycle some things */
  free_sem_value (id);

  return type;
}


void
install_proc_args (struct ParserState* const parser,
                   struct SemValue*          paramsList)
{
  struct SemValue identifier;
  struct SemValue type;

  identifier.val_type = VAL_ID;
  type.val_type       = VAL_TYPE_SPEC;

  while (paramsList != NULL)
    {
      assert (paramsList->val_type == VAL_PRCDCL_LIST);

      identifier.val.u_id = paramsList->val.u_prdcl.id;
      type.val.u_tspec    = paramsList->val.u_prdcl.type;

      add_declaration (parser, &identifier, &type, TRUE, TRUE);

      paramsList->val_type = VAL_REUSE;
      paramsList           = paramsList->val.u_prdcl.next;
    }
}


void
install_proc_decl (struct ParserState* const parser,
                   struct SemValue* const    id)
{
  struct WArray* const procs = &(parser->globalStmt.spec.glb.procsDecls);
  struct Statement     stmt;

  assert (parser->pCurrentStmt->type == STMT_GLOBAL);
  assert (id->val_type == VAL_ID);

  if (init_proc_stmt (parser->pCurrentStmt, &stmt) == FALSE)
    log_message (parser, IGNORE_BUFFER_POS, MSG_NO_MEM);

  stmt.spec.proc.name       = id->val.u_id.name;
  stmt.spec.proc.nameLength = id->val.u_id.length;

  if (find_proc_decl (parser,
                      stmt.spec.proc.name,
                      stmt.spec.proc.nameLength,
                      FALSE) != NULL)
    {
      char tname[128];

      wh_copy_first (tname,
                     stmt.spec.proc.name,
                     sizeof tname,
                     stmt.spec.proc.nameLength);

      log_message (parser, parser->bufferPos, MSG_PROC_ADECL, tname);

      clear_proc_stmt (&stmt);
    }
  else
    {
      void* check = NULL;

      if (parser->externDecl)
        {
          stmt.spec.proc.procId = 0;

          MARK_AS_EXTERNAL(stmt.spec.proc.procId);
          MARK_AS_NOT_REFERENCED(stmt.spec.proc.procId);
        }
      else
        stmt.spec.proc.procId = parser->globalStmt.spec.glb.procsCount++;

      check = wh_array_add (procs, &stmt);
      if (check == NULL)
        {
          log_message (parser, IGNORE_BUFFER_POS, MSG_NO_MEM);

          parser->abortError = TRUE;
          clear_proc_stmt (&stmt);
        }
      else
        parser->pCurrentStmt = check;
    }

  free_sem_value (id);

  return;
}


void
set_proc_rettype (struct ParserState* const parser,
                  struct SemValue* const    type)
{
  struct DeclaredVar* retVar = (struct DeclaredVar*)
      wh_array_get (&(parser->pCurrentStmt->spec.proc.paramsList), 0);

  assert(type->val_type == VAL_TYPE_SPEC);

  memset (retVar, 0, sizeof (*retVar));

  retVar->type  = type->val.u_tspec.type;
  retVar->extra = type->val.u_tspec.extra;

  if (IS_TABLE (retVar->type))
    {
      struct DeclaredVar* field = retVar->extra;

      if (field == NULL)
        retVar->extra = retVar;
      else
        {
          while (field->extra != NULL)
            {
              assert(IS_TABLE_FIELD (field->type));
              field = field->extra;
            }
          field->extra = retVar;
        }
    }

  retVar->typeSpecOff = fill_type_spec (&parser->globalStmt.spec.glb.typesDescs,
                                        retVar);
  free_sem_value (type);

  return;
}


void
finish_proc_decl(struct ParserState* const parser)
{
  assert(parser->pCurrentStmt->type = STMT_PROC);
  assert(&(parser->globalStmt) == parser->pCurrentStmt->parent);

  parser->pCurrentStmt = &parser->globalStmt;

  return;
}

