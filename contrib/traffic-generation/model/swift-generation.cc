/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <random>
#include "ns3/custom-applications-module.h"
#include "ns3/applications-module.h"
#include "ns3/utils-module.h"
#include "ns3/traffic-generation-module.h"


NS_LOG_COMPONENT_DEFINE ("swift-generation");


namespace ns3 {

NodesUsage SendSwiftTraffic(std::unordered_map<double, std::vector<Ptr<Node>>> rtt_to_senders,
                            std::vector<double> rtt_cdf,
                            std::unordered_map<std::string, PrefixMetadata> prefixes,
                            PrefixMappings mapping,
                            std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                            std::string flowDistFile,
                            std::string logOutput,
                            uint32_t seed,
                            uint32_t interArrivalFlow,
                            double duration, double rtt_shift)

{

  //Log output file
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> sent_flows_file;
  if (logOutput != "")
  {
    sent_flows_file = asciiTraceHelper.CreateFileStream(logOutput);
  }

  //Load flow distribution
  std::vector<FlowMetadata> flowDist = GetFlowsPerPrefix(flowDistFile, mapping.trace_to_sim);

  NS_LOG_DEBUG("Number of flows to use: " << flowDist.size());

  //Usage object
  NodesUsage nodes_usage = NodesUsage();

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  gen.seed(seed);
  std::exponential_distribution<double> interArrivalTime(interArrivalFlow);

  double startTime = 1.0;
  double simulationTime = duration;
  uint64_t num_flows_started;
//        double bytes_per_sec = 0;
//        double time_reference = 1;
//        std::ofstream out_rates("bytes_per_sec.txt");

  Ptr<UniformRandomVariable> random_variable = CreateObject<UniformRandomVariable>();

  while ((startTime - 1) < simulationTime) {

    //get a flow
    FlowMetadata flow = RandomFromVector<FlowMetadata>(flowDist, random_variable);

    //ugly temporal fix
    if (flow.duration == 0) {
      flow.duration = 0.25;
    }

    flow.rtt = flow.rtt * rtt_shift;

    double rtt = FindClosest(rtt_cdf, flow.rtt);

    NS_ASSERT_MSG(rtt_to_senders[rtt].size() >= 1, "There are no source hosts for rtt: " << rtt);

    Ptr<Node> src = RandomFromVector<Ptr<Node>>(rtt_to_senders[rtt], random_variable);

    //Get Destination
    Ptr<Node> dst = prefixes[flow.prefix].server;

    NS_LOG_DEBUG(
      "Flow Features:    " << "\tReal_rtt: " << flow.rtt
                           << "\tRtt Found: " << rtt
                           << "\tSrc: " << GetNodeName(src) << "(" << GetNodeIp(src) << ")"
                           << "\tDst: " << GetNodeName(dst) << "(" << GetNodeIp(dst) << ")");

    //Destination port
    std::vector<uint16_t> availablePorts = hostsToPorts[GetNodeName(dst)];
    uint16_t dport = RandomFromVector<uint16_t>(availablePorts, random_variable);

    startTime += interArrivalTime(gen);
    InstallRateSend(src, dst, dport, flow.packets, flow.bytes, flow.duration, rtt, startTime);
    //Save log into a file
    if (logOutput != "")
    {
      *(sent_flows_file->GetStream()) << GetNodeIp(src) << " " << GetNodeIp(dst)
                                      << " " << dport << " " << flow.packets << " "
                                      << flow.bytes << " " << flow.duration << " " << startTime
                                      << " " << rtt << "\n";
    }

    num_flows_started++;
    nodes_usage.Update(src, dst, flow.bytes);
  }

  NS_LOG_DEBUG("Number of flows Started: " << num_flows_started);
  return nodes_usage;
}

}
