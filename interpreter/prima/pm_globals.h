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

#ifndef PM_GLOBALS_H_
#define PM_GLOBALS_H_

#include <vector>

#include "operands.h"

namespace prima
{

struct GlobalEntry
{
  D_UINT32  m_IdOffet;
  D_UINT32  m_TypeOffset;
  bool      m_Resolved;
};

class GlobalManager
{
public:
  GlobalManager ()
  {
  }

  ~GlobalManager ()
  {
  }

  D_UINT             AddGlobal (const D_UINT8 *const pIdentifier,
                                const D_UINT32       typeOffset,
                                bool                 external);
  D_UINT             FindGlobal (const D_UINT8 *const pIdentifier);
  bool               IsResolved (const D_UINT glbEntry);
  StackedOperand&    GetGlobalValue (const D_UINT glbEntry);
  const GlobalEntry& GetGlobalDescriptor (const D_UINT glbEntry);

  static const D_UINT INVALID_ENTRY = ~0;

protected:
  std::vector<D_UINT8>        m_Identifiers;
  std::vector<StackedOperand> m_Storage;
  std::vector<GlobalEntry>    m_GlobalsEntrys;
};


}

#endif /* PM_GLOBALS_H_ */
