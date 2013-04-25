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

#include <stdarg.h>

#include "whisper.h"

#include "compiler/whisperc.h"

#ifndef MSGLOG_H_
#define MSGLOG_H_

void
my_postman (WH_MESSENGER_CTXT data,
            uint_t            buffOff,
            uint_t            msgId,
            uint_t            msgType,
            const char*       msgFormat,
            va_list           args);

#endif /* MSGLOG_H_ */

