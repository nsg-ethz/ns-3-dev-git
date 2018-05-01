/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SWIFT_GENERATION_H
#define SWIFT_GENERATION_H

#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/utils-module.h"
#include <unordered_map>
#include <vector>

namespace ns3 {

    NodesUsage  SendSwiftTraffic(std::unordered_map<double, std::vector<Ptr<Node>>> rtt_to_senders,
                                 std::vector<double> rtt_cdf,
                                 std::unordered_map<std::string, prefix_metadata> prefixes,
                                 PrefixMappings mapping,
                                 std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                                 std::string flowDistFile,
                                 uint32_t seed = 1,
                                 uint32_t interArrivalFlow = 1000,
                                 double duration = 5, double rtt_shift=1);

}

#endif /* SWIFT_GENERATION_H */
