/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SWIFT_GENERATION_H
#define SWIFT_GENERATION_H

#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/utils.h"
#include <unordered_map>
#include <vector>

namespace ns3 {

    NodesUsage  sendTestTraffic(std::unordered_map<double, std::vector<Ptr<Node>>> rtt_to_senders,
                                std::vector<double> rtt_cdf,
                                std::unordered_map<std::string, Ptr<Node>> prefix_to_dst,
                                std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                                std::string flowDistFile,
                                uint32_t num_flows,
                                double duration,  double rtt_shift);


    NodesUsage  sendSwiftTraffic(std::unordered_map<double, std::vector<Ptr<Node>>> rtt_to_senders,
                                 std::vector<double> rtt_cdf,
                                 std::unordered_map<std::string, Ptr<Node>> prefix_to_dst,
                                 std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                                 std::string flowDistFile,
                                 uint32_t seed = 1,
                                 uint32_t interArrivalFlow = 1000,
                                 double duration = 5, double rtt_shift=1);

    NodesUsage  sendSwiftTraffic_old(std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_senders,
                                     std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_receivers,
                                     std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                                     std::string rttFile,
                                     std::string flowSizeFile,
                                     uint32_t seed = 1,
                                     uint32_t interArrivalFlow = 1000,
                                     double duration = 5);

}

#endif /* SWIFT_GENERATION_H */