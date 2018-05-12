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
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("flow-model-test");

Ptr<BulkSendApplication>
_InstallNormalBulkSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, uint64_t size, double startTime) {

  Ipv4Address addr = GetNodeIp(dstHost);
  Address sinkAddress(InetSocketAddress(addr, dport));

  Ptr<BulkSendApplication> bulkSender = CreateObject<BulkSendApplication>();

  bulkSender->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
  bulkSender->SetAttribute("MaxBytes", UintegerValue(size));
  bulkSender->SetAttribute("Remote", AddressValue(sinkAddress));

  //Install app
  srcHost->AddApplication(bulkSender);

  bulkSender->SetStartTime(Seconds(startTime));
  //bulkSender->SetStopTime(Seconds(1000));

  return bulkSender;
}

void SetSocketTraces(Ptr<BulkSendApplication> app)
{
  Ptr<Socket> socket = app->GetSocket();
  //Same way of doing it and probably useful
  Simulator::Schedule(Seconds(0), &Socket::TraceConnectWithoutContext, socket, "CongestionWindow", MakeBoundCallback (&CwndChange, "CongestionWindow"));
//  app->GetSocket()->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback (&CwndChange, "CongestionWindow"));
//  app->GetSocket()->TraceConnectWithoutContext("AdvWND", MakeBoundCallback (&CwndChange, "AdvWND"));
//  app->GetSocket()->TraceConnectWithoutContext("RWND", MakeBoundCallback (&CwndChange, "RWND"));
//  app->GetSocket()->TraceConnectWithoutContext("BytesInFlight", MakeBoundCallback (&CwndChange, "BytesInFlight"));
}


int
main(int argc, char *argv[]) {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(3);

  LogComponentEnable("traffic-generation", LOG_DEBUG);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));
  pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize("100p")));

  //Tcp Socket (general socket conf)
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(4000000));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(4000000));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1446)); //MTU 1446
  Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(4294967295));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(500));

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
  NetDeviceContainer link2;
  link1 = pointToPoint.Install(NodeContainer(nodes.Get(0), nodes.Get(1)));
  link2 = pointToPoint.Install(NodeContainer(nodes.Get(1), nodes.Get(2)));

  link2.Get(0)->GetChannel()->SetAttribute("Delay", StringValue("5ms"));
  link2.Get(0)->SetAttribute("DataRate", StringValue("1Mbps"));
  link2.Get(0)->GetObject<PointToPointNetDevice>()->GetQueue()->SetAttribute("MaxSize", QueueSizeValue(QueueSize("10p")));

  TimeValue time;
  link2.Get(0)->GetChannel()->GetAttribute("Delay", time);
  DataRateValue rate;
  link2.Get(0)->GetAttribute("DataRate", rate);
  NS_LOG_UNCOND("delay " << time.Get() << " Datarate " << rate.Get());

  link2.Get(1)->GetChannel()->GetAttribute("Delay", time);
  link2.Get(1)->GetAttribute("DataRate", rate);
  NS_LOG_UNCOND("delay " << time.Get() << " Datarate " << rate.Get());
  return 0;

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign(link1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  TrafficControlHelper tch;
  tch.Uninstall(link1);

  Packet::EnablePrinting();

  //traffic
  uint16_t dst_port = 80;
  InstallSink(nodes.Get(1), dst_port, 0, "TCP");

  //InstallRateSend(nodes.Get(0), nodes.Get(1), dst_port, 14, 5000, 2.3, 1, 1);
  Ptr<BulkSendApplication> app = _InstallNormalBulkSend(nodes.Get(0), nodes.Get(1), dst_port, 150000, 1);
  //Simulator::Schedule(Seconds(1.01), &SetSocketTraces, app);

  //InstallNormalBulkSend(nodes.Get(0), nodes.Get(2), dst_port, 1500000, 1);

  pointToPoint.EnablePcap("../../test_congestion/out", link1.Get(0), bool(1));

  PcapHelper pcapHelper;
  Ptr<PcapFileWrapper> pcap_file0 = pcapHelper.CreateFile("../../test_congestion/drops0.pcap", std::ios::out, PcapHelper::DLT_PPP);
  Ptr<PcapFileWrapper> pcap_file1 = pcapHelper.CreateFile("../../test_congestion/drops1.pcap", std::ios::out, PcapHelper::DLT_PPP);

  link1.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&TracePcap, pcap_file0));
  link1.Get (0)->TraceConnectWithoutContext ("PhyTxDrop", MakeBoundCallback (&TracePcap, pcap_file0));
  link1.Get (0)->TraceConnectWithoutContext ("MacTxDrop", MakeBoundCallback (&TracePcap, pcap_file0));

  link1.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&TracePcap, pcap_file1));
  link1.Get (1)->TraceConnectWithoutContext ("PhyTxDrop", MakeBoundCallback (&TracePcap, pcap_file1));
  link1.Get (1)->TraceConnectWithoutContext ("MacTxDrop", MakeBoundCallback (&TracePcap, pcap_file1));


  Ptr<DropTailQueue<Packet>> queue = CreateObject<DropTailQueue<Packet>>();
  queue->SetMaxSize(QueueSize("10p"));
  Ptr<Packet> p = Create<Packet> (1000);

  for (int i = 0; i < 15; i++)
  {
    Ptr<Packet> pp = p->Copy();
    if (queue->Enqueue(pp)){
      queue->Dequeue();
      NS_LOG_UNCOND(i);
    }
  }



  //Simulator::Stop (Seconds (100000));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
