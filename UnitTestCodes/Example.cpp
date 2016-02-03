#include "Include/Foundation/SystemInclude.h"
#include <cppunit/extensions/HelperMacros.h>

#include "Example.h"
using namespace std;

CxxBeginNameSpace(UnitTest)
    
/**********************Xxx**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(Example);

void Example::TestXxx()
{
	CPPUNIT_ASSERT(true);
}

CxxEndNameSpace
