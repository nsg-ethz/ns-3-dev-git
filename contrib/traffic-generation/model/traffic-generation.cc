/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <random>
#include "traffic-generation.h"
#include "ns3/custom-applications-module.h"
#include "ns3/applications-module.h"
#include "ns3/utils.h"
#include "ns3/traffic-generation-module.h"

NS_LOG_COMPONENT_DEFINE ("traffic-generation");


namespace ns3 {

void
InstallSink(Ptr<Node> node, uint16_t sinkPort, uint32_t duration, std::string protocol)
{
  //create sink helper
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));

  if (protocol == "UDP"){
  	packetSinkHelper.SetAttribute("Protocol",StringValue("ns3::UdpSocketFactory"));
	}

  ApplicationContainer sinkApps = packetSinkHelper.Install(node);

  sinkApps.Start (Seconds (0));
  //Only schedule a stop it duration is bigger than 0 seconds
  if (duration != 0){
  	sinkApps.Stop (Seconds (duration));
  }
}

Ptr<Socket>
InstallSimpleSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t sinkPort, DataRate dataRate, uint32_t numPackets, std::string protocol)
{
  Ptr<Socket> ns3Socket;
  uint32_t num_packets = numPackets;

  //had to put an initial value
  if (protocol == "TCP")
  {
    ns3Socket = Socket::CreateSocket (srcHost, TcpSocketFactory::GetTypeId ());
  }
  else
	{
    ns3Socket = Socket::CreateSocket (srcHost, UdpSocketFactory::GetTypeId ());
	}


  Ipv4Address addr = GetNodeIp(dstHost);

  Address sinkAddress (InetSocketAddress (addr, sinkPort));

  Ptr<SimpleSend> app = CreateObject<SimpleSend> ();
  app->Setup (ns3Socket, sinkAddress, 1440, num_packets, dataRate);

  srcHost->AddApplication (app);

  app->SetStartTime (Seconds (1.));
  //Stop not needed since the simulator stops when there are no more packets to send.
  //app->SetStopTime (Seconds (1000.));
  return ns3Socket;
}

void
InstallOnOffSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, DataRate dataRate, uint32_t packet_size, uint64_t max_size, double startTime)
{
  Ipv4Address addr = GetNodeIp(dstHost);
  Address sinkAddress (InetSocketAddress (addr, dport));

  Ptr<OnOffApplication> onoff_sender =  CreateObject<OnOffApplication>();

  onoff_sender->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
  onoff_sender->SetAttribute("Remote", AddressValue(sinkAddress));

  onoff_sender->SetAttribute("DataRate", DataRateValue(dataRate));
  onoff_sender->SetAttribute("PacketSize", UintegerValue(packet_size));
  onoff_sender->SetAttribute("MaxBytes", UintegerValue(max_size));

  srcHost->AddApplication(onoff_sender);
  onoff_sender->SetStartTime(Seconds(startTime));
  //TODO: check if this has some implication.
  onoff_sender->SetStopTime(Seconds(1000));
}

void
InstallRateSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, uint32_t n_packets, uint64_t max_size, double duration, double startTime)
{
    if (duration <= 0){
        NS_LOG_DEBUG("Install Rate Send: \t Bulk Send Instead!");
        installNormalBulkSend(srcHost,dstHost,dport,max_size,startTime);
        return;
    }

  Ipv4Address addr = GetNodeIp(dstHost);
  Address sinkAddress (InetSocketAddress (addr, dport));

  Ptr<RateSendApplication> rate_send_app =  CreateObject<RateSendApplication>();


  rate_send_app->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
  rate_send_app->SetAttribute("Remote", AddressValue(sinkAddress));

  double avg_size_per_packet;
  double interval_duration;

  max_size = max_size - (n_packets * 54);

  double bytes_per_sec = double(max_size) / duration;

  avg_size_per_packet = double(max_size) / n_packets;

  interval_duration = avg_size_per_packet / bytes_per_sec;

  uint64_t bytes_per_period = uint64_t(avg_size_per_packet);

  NS_LOG_DEBUG("Flow Features:    " << "\tNumber Packets: " << n_packets
                                << "\t Bytes To send: " <<  max_size
                                << "\t Duration: " << duration);

  NS_LOG_DEBUG("Install Rate Send: " << "\tBytes per sec: " << bytes_per_sec
                                     << "\t Avg Pkt Size: " <<  avg_size_per_packet
                                     << "\t Interval Duration: " << interval_duration
                                     << "\t Packets Per Second: " << 1/interval_duration << "\n");

  rate_send_app->SetAttribute("MaxBytes", UintegerValue(max_size));
  rate_send_app->SetAttribute("BytesPerInterval", UintegerValue(bytes_per_period));
  rate_send_app->SetAttribute("IntervalDuration", DoubleValue(interval_duration));

  srcHost->AddApplication(rate_send_app);
  rate_send_app->SetStartTime(Seconds(startTime));
  //TODO: check if this has some implication.
  rate_send_app->SetStopTime(Seconds(10000));
}


void
installNormalBulkSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, uint64_t size, double startTime)
{

  Ipv4Address addr = GetNodeIp(dstHost);
  Address sinkAddress (InetSocketAddress (addr, dport));

  Ptr<BulkSendApplication> bulkSender = CreateObject<BulkSendApplication>();

  bulkSender->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
  bulkSender->SetAttribute("MaxBytes", UintegerValue(size));
  bulkSender->SetAttribute("Remote", AddressValue(sinkAddress));

  //Install app
  srcHost->AddApplication(bulkSender);

  bulkSender->SetStartTime(Seconds(startTime));
  bulkSender->SetStopTime(Seconds(1000));

  return;
//  return socket;
}

std::unordered_map <std::string, std::vector<uint16_t>>
InstallSinks(NodeContainer hosts, uint16_t sinksPerHost, uint32_t duration, std::string protocol)
{

	NS_ASSERT_MSG(sinksPerHost < ((uint16_t) -2), "Can not create such amount of sinks");

    std::unordered_map <std::string, std::vector<uint16_t>> hostsToPorts;
    Ptr<UniformRandomVariable> random_generator = CreateObject<UniformRandomVariable> ();
    uint16_t starting_port;

    for(NodeContainer::Iterator host = hosts.Begin(); host != hosts.End (); host ++){

        starting_port = random_generator->GetInteger(0, (uint16_t)-2 - sinksPerHost);
        std::string host_name = GetNodeName((*host));

        hostsToPorts[host_name] = std::vector<uint16_t>();

        for (int i = 0; i < sinksPerHost ; i++){
            InstallSink((*host), starting_port+i, duration, protocol);
            //Add port into the vector
            //NS_LOG_DEBUG("Install Sink: " << host_name << " port:" << starting_port+i);
            hostsToPorts[host_name].push_back(starting_port+i);
        }

      }

  return hostsToPorts;
}

//DO THE SAME WITH THE BULK APP, WHICH IS PROBABLY WHAT WE WANT TO HAVE.
void installBulkSend(Ptr <Node> srcHost, Ptr <Node> dstHost, uint16_t dport, uint64_t size, double startTime,
                     Ptr <OutputStreamWrapper> fctFile, Ptr <OutputStreamWrapper> counterFile,
                     Ptr <OutputStreamWrapper> flowsFile,
                     uint64_t flowId, uint64_t *recordedFlowsCounter, double *startRecordingTime,
                     double recordingTime) {

    Ipv4Address addr = GetNodeIp(dstHost);
    Address sinkAddress(InetSocketAddress(addr, dport));

    Ptr <CustomBulkApplication> bulkSender = CreateObject<CustomBulkApplication>();

//  Ptr<Socket> socket;
//  socket = Socket::CreateSocket (srcHost, TcpSocketFactory::GetTypeId ());
//  bulkSender->SetSocket(socket);

    bulkSender->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
    bulkSender->SetAttribute("MaxBytes", UintegerValue(size));
    bulkSender->SetAttribute("Remote", AddressValue(sinkAddress));
    bulkSender->SetAttribute("FlowId", UintegerValue(flowId));

    bulkSender->SetOutputFile(fctFile);
    bulkSender->SetCounterFile(counterFile);
    bulkSender->SetFlowsFile(flowsFile);

    bulkSender->SetStartRecordingTime(startRecordingTime);
    bulkSender->SetRecordingTime(recordingTime);
    bulkSender->SetRecordedFlowsCounter(recordedFlowsCounter);

    //Install app
    srcHost->AddApplication(bulkSender);

    bulkSender->SetStartTime(Seconds(startTime));
    bulkSender->SetStopTime(Seconds(1000));

    return;
}

void
SendBindTest(Ptr<Node> src, NodeContainer receivers, std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts, uint32_t flows)
{
	for (uint32_t i = 0 ; i < flows ;i++){
		Ptr<Node> dst  = receivers.Get(random_variable->GetInteger(0, receivers.GetN()-1));
		std::vector<uint16_t> availablePorts = hostsToPorts[GetNodeName(dst)];
		uint16_t dport = randomFromVector<uint16_t>(availablePorts);
		installNormalBulkSend(src, dst, dport, 50000, random_variable->GetValue(1, 10));
	}
}


}

