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
main(int argc, char *argv[]) {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(2);

  LogComponentEnable("traffic-generation", LOG_DEBUG);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Gbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("50ms"));

  //Tcp Socket (general socket conf)
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(4000000));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(4000000));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1446)); //MTU 1446
  Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(4294967295));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));

  //Can be much slower than my rtt because packet size of syn is 60bytes
  Config::SetDefault("ns3::TcpSocket::ConnCount", UintegerValue(5)); //retrnamissions during connection
  Config::SetDefault("ns3::TcpSocket::DataRetries", UintegerValue(15)); //retranmissions
  Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(MilliSeconds(150 / 50)));
// 	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue(2));
  Config::SetDefault("ns3::TcpSocket::TcpNoDelay", BooleanValue(true)); //disable nagle's algorithm
  Config::SetDefault("ns3::TcpSocket::PersistTimeout",
                     TimeValue(NanoSeconds(6000000000))); //persist timeout to porbe for rx window

  //Tcp Socket Base: provides connection orientation, sliding window, flow control; congestion control is delegated to the subclasses (i.e new reno)

  Config::SetDefault("ns3::TcpSocketBase::MaxSegLifetime", DoubleValue(10));
  Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(200))); //min RTO value that can be set
  Config::SetDefault("ns3::TcpSocketBase::ClockGranularity", TimeValue(MicroSeconds(1)));
  Config::SetDefault("ns3::TcpSocketBase::ReTxThreshold", UintegerValue(3)); //same than DupAckThreshold

  Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(true)); //enable sack
  Config::SetDefault("ns3::TcpSocketBase::LimitedTransmit", BooleanValue(true)); //enable sack

  NetDeviceContainer link1;
  link1 = pointToPoint.Install(NodeContainer(nodes.Get(0), nodes.Get(1)));

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign(link1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Packet::EnablePrinting();

  //traffic
  uint16_t dst_port = 80;
  InstallSink(nodes.Get(1), dst_port, 0, "TCP");

  InstallRateSend(nodes.Get(0), nodes.Get(1), dst_port, 10, 3500, 5.2, 0.1, 1);
  //InstallNormalBulkSend(nodes.Get(0), nodes.Get(2), dst_port, 1500000, 1);
  //InstallNormalBulkSend(nodes.Get(0), nodes.Get(2), dst_port, 1500000, 1);

  pointToPoint.EnablePcap("../../test_duration/out", link1.Get(0), bool(1));


  //Simulator::Stop (Seconds (100000));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
