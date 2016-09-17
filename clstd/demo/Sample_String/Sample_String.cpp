// Sample_String.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <tchar.h>

#include "clstd.h"
#include "clString.H"
#include "clPathFile.h"

int _tmain(int argc, _TCHAR* argv[])
{
  b32 bresult;
  bresult = clpathfile::MatchSpec("abcdefghijklmnopqrst", "abcd*opq");
  ASSERT(bresult == FALSE);

  bresult = clpathfile::MatchSpec("abcdefghijklmnopq", "abcd*opq");
  ASSERT(bresult == TRUE);
  
  bresult = clpathfile::MatchSpec("abcdefghijklmnopqrst", "abcd*opq*");
  ASSERT(bresult == TRUE);

  bresult = clpathfile::MatchSpec("abcdefghijklmnopq.rst", "*.rst");
  ASSERT(bresult == TRUE);

	return 0;
}

