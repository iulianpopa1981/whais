/******************************************************************************
WOD - Whisper Object Dumper.
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

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "wod_cmdline.h"

using namespace std;

#define VER_MAJOR       0
#define VER_MINOR       4

static inline bool
isStrEqual (const D_CHAR * str1, const D_CHAR * str2)
{
  return::strcmp (str1, str2) == 0;
}

WodCmdLineParser::WodCmdLineParser (D_INT argc, D_CHAR ** argv):mArgCount (argc),
mArgs (argv),
mSourceFile (NULL), mOutStream (&cout), mDisplayHelp (false)
{
  Parse ();
}

WodCmdLineParser::~WodCmdLineParser ()
{
  if (mOutStream != &cout)
    delete mOutStream;
}

void
WodCmdLineParser::Parse ()
{
  int index = 1;
  if (index >= mArgCount)
    throw WodCmdLineException ("No arguments! Use --help first!", _EXTRA(0));

  while (index < mArgCount)
    {
      if (isStrEqual (mArgs[index], "-h")
	  || isStrEqual (mArgs[index], "--help"))
	{
	  mDisplayHelp = true;
	  ++index;
	}
      else if (isStrEqual (mArgs[index], "-o"))
	{
	  if (mOutStream != &cout)
	    throw WodCmdLineException ("Parameter '-o' is given twice",
	        _EXTRA(0));

	  if ((++index >= mArgCount) || (mArgs[index][0] == '-'))
	    throw WodCmdLineException ("Missing file name argument for "
				       "for parameter -o", _EXTRA(0));
	  else
	  mOutStream = new ofstream (mArgs[index++]);
	}
      else if ((mArgs[index][0] != '-') && (mArgs[index][0] != '\\'))
	{
	  if ((void *) mSourceFile != NULL)
	    throw
	      WodCmdLineException ("The source file was already specified!",
	          _EXTRA(0));

	  mSourceFile = mArgs[index++];
	}
      else
	throw WodCmdLineException ("Unknown arguments! Use --help first!",
	    _EXTRA(0));
    }
  CheckArguments ();
}

void
WodCmdLineParser::CheckArguments ()
{
  if (mDisplayHelp)
    {
      DisplayUsage ();
      exit (0);
    }
  else if (mSourceFile == NULL)
    throw WodCmdLineException ("No given input file!", _EXTRA(0));
}

void
WodCmdLineParser::DisplayUsage () const
{
  using namespace std;

  cout << "Whisper Object Dumper ver. " << VER_MAJOR << '.' << VER_MINOR;
  cout << " by Iulian POPA (popaiulian@gmail.com)" << endl
    << "Usage: " << "wod input_file [-o output_file] [--help | -h]" << endl;
}
