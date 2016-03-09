#include "Include/Foundation/SystemInclude.h"
#include <cppunit/extensions/HelperMacros.h>

/* Foundation */
#include "Include/Foundation/Type.h"

/* Configuration */
#include "Include/Configuration/NetworkCfgInterface.h"

/* ConfigurationWrapper */
#include "Include/ConfigurationWrapper/NetworkCfgWrapperInterface.h"

#include "UtNetworksCfg.h"
using namespace std;

CxxBeginNameSpace(UnitTest)
/**********************NetworksCfg**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(NetworksCfg);

void NetworksCfg::TestReceiver()
{
    std::fstream xml("receiver.xml", ios_base::out);
    xml << "<?xml version='1.0' encoding='UTF-8'?>"
        "<root>"    
        "    <network netid='1' srcip='10.0.0.1'>"
        "        <receiver ip='225.1.1.1' port='5001' tsid='1'>"
        "            <pid oldpid='16' newpid='116'/>"
        "            <pid oldpid='17' newpid='117'/>"
        "        </receiver>"
        "        <receiver ip='225.1.1.2' port='5002' tsid='2'>"
        "            <pid oldpid='18' newpid='118'/>"
        "        </receiver>"
        "        <receiver ip='225.1.1.3' port='5003' tsid='3'>"
        "        </receiver>"
        "    </network>"  
        "    <network netid='1' srcip='10.0.0.2'>"
        "        <receiver ip='225.1.1.4' port='5004' tsid='4'>"
        "        </receiver>"
        "    </network>"  
        "</root>";
    xml.close();

    struct PidPair
    {
        Pid        oldPid;
        Pid        newPid;
    };

    struct Receiver
    {
        ReceiverId receiverId;
        TsId       tsId;
        string     dstIp;
        uint16_t   port;
        PidPair    pidPairs[32];
        uint_t     pidNumber;
    };

    struct NetworkCfg
    {
        string     srcIp;
        Receiver   receivers[32];
        uint_t     receiverNumber;
    };

    NetworkCfg networks[] =
    {
        {
            "10.0.0.1", 
            {
                {0, 1, "225.1.1.1", 5001, {{16, 116}, {17, 117}}, 2},
                {1, 2, "225.1.1.2", 5002, {{18, 118}}, 1},
                {2, 3, "225.1.1.3", 5003, {}, 0}
            },
            3
        },
        {
            "10.0.0.2", 
            {
                {3, 4, "225.1.1.4", 5004, {}, 0}
            },
            1
        }
    };

    auto_ptr<NetworkCfgsInterface> networkCfgs(NetworkCfgsInterface::CreateInstance());
    NetworkCfgWrapperInterface<NetworkCfgsInterface, NetworkCfgInterface, ReceiverInterface> networkCfgWrapper;
    error_code errCode = networkCfgWrapper.Select(*networkCfgs, "receiver.xml");
    CPPUNIT_ASSERT(!errCode);

    uint_t h1, h2, h3;
    NetworkCfgsInterface::iterator i1;
    for (i1 = networkCfgs->Begin(), h1 = 0; i1 != networkCfgs->End(); ++i1, ++h1)
    {        
        string srcAddr(inet_ntoa((*i1)->GetSrcAddr()));
        CPPUNIT_ASSERT(srcAddr == networks[h1].srcIp);
        NetworkCfgInterface::iterator i2;
        for (i2 = (*i1)->Begin(), h2 = 0; i2 != (*i1)->End(); ++i2, ++h2)
        {
            CPPUNIT_ASSERT((*i2)->GetReceiverId() == networks[h1].receivers[h2].receiverId);
            CPPUNIT_ASSERT((*i2)->GetTsId() == networks[h1].receivers[h2].tsId);
            CPPUNIT_ASSERT(string(inet_ntoa((*i2)->GetDstAddr().sin_addr)) == networks[h1].receivers[h2].dstIp);
            CPPUNIT_ASSERT((*i2)->GetDstAddr().sin_port == htons(networks[h1].receivers[h2].port));

            ReceiverInterface::iterator i3;
            for (i3 = (*i2)->Begin(), h3 = 0; i3 != (*i2)->End(); ++i3, ++h3)
            {
                CPPUNIT_ASSERT(i3->first  == networks[h1].receivers[h2].pidPairs[h3].oldPid);
                CPPUNIT_ASSERT(i3->second == networks[h1].receivers[h2].pidPairs[h3].newPid);
            }
            CPPUNIT_ASSERT(h3 == networks[h1].receivers[h2].pidNumber);
        }
        CPPUNIT_ASSERT(h2 == networks[h1].receiverNumber);
    }
    CPPUNIT_ASSERT(h1 == sizeof(networks) / sizeof(NetworkCfg));

    remove("receiver.xml");	
}

CxxEndNameSpace