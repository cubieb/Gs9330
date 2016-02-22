// VcUnitTestProject.cpp : 定义控制台应用程序的入口点。
//
#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"

/* ControllerInterface */
#include "Include/Configuration/DirCfgInterface.h"

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

#include "UtSiTable.h"
#include "UtTransportPacket.h"
#include "UtConverter.h"
#include "UtSiTableXmlWrapperInterface.h"

using namespace std;
using namespace UnitTest;

int main(int argc, char* argv[])
{
    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // Add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);        

    // Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);      

    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller );

    // Print test in a compiler compatible format.
    CPPUNIT_NS::CompilerOutputter outputter(&result, CPPUNIT_NS::stdCOut());
    outputter.write();

	return 0;
}

