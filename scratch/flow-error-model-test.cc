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

NS_LOG_COMPONENT_DEFINE ("flow-error-model-test");


static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
    *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
    NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
    file->Write (Simulator::Now (), p);
}

int
main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    NodeContainer nodes;
    nodes.Create (3);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    Config::SetDefault("ns3::TcpSocket::DataRetries", UintegerValue(5)); //retranmissions
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(200))); //min RTO value that can be set

    NetDeviceContainer link1;
    NetDeviceContainer link2;
    link1 = pointToPoint.Install (NodeContainer(nodes.Get(0), nodes.Get(1)));
    link2 = pointToPoint.Install (NodeContainer(nodes.Get(1), nodes.Get(2)));

    Ptr<FlowErrorModel> em = CreateObject<FlowErrorModel> ();
//  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
    DynamicCast<PointToPointNetDevice>(link2.Get (0))->SetReceiveErrorModel(em);

    InternetStackHelper stack;
    stack.Install (nodes);


    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign (link1);
    address.NewNetwork();
    Ipv4InterfaceContainer interfaces1 = address.Assign (link2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //traffic
    uint16_t dst_port = 7000;
    InstallSink(nodes.Get(2), dst_port, 0, "TCP");
    InstallNormalBulkSend(nodes.Get(0), nodes.Get(2), dst_port, 15000, 1);

    pointToPoint.EnablePcap("out", link1.Get(0), bool(1));

    Simulator::Stop (Seconds (1000));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
