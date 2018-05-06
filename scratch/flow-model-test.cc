//
// Created by edgar costa molero on 05.05.18.
//
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/utils-module.h"
#include "ns3/traffic-generation-module.h"
#include "ns3/custom-applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("flow-model-test");


int
main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Gbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("150ms"));

    Config::SetDefault("ns3::TcpSocket::DataRetries", UintegerValue(5)); //retranmissions
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(200))); //min RTO value that can be set
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(true)); //enable sack


    NetDeviceContainer link1;
    link1 = pointToPoint.Install (NodeContainer(nodes.Get(0), nodes.Get(1)));

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign (link1);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Packet::EnablePrinting();

    //traffic
    uint16_t dst_port = 7001;
    InstallSink(nodes.Get(1), dst_port, 0, "TCP");

    InstallRateSend(nodes.Get(0),nodes.Get(1),dst_port,5,1545,0.342, 1);

    //InstallNormalBulkSend(nodes.Get(0), nodes.Get(2), dst_port, 1500000, 1);
    //InstallNormalBulkSend(nodes.Get(0), nodes.Get(2), dst_port, 1500000, 1);

    pointToPoint.EnablePcap("out", link1.Get(0), bool(1));


    //Simulator::Stop (Seconds (100000));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
