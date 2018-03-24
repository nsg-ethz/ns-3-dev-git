/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SWIFT_UTILS_H
#define SWIFT_UTILS_H

#include <vector>
#include <string>
#include <unordered_map>
#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3 {

    struct flow_size_metadata {
        uint32_t packets;
        uint32_t duration;
        uint64_t bytes;
    };

    struct flow_metadata {
        std::string prefix_ip;
        std::string prefix_mask;
        uint32_t packets;
        uint32_t duration;
        uint64_t bytes;
        double rtt;
    };

    struct prefix_metadata {
        std::string prefix_ip;
        std::string prefix_mask;
        double loss;
        std::vector<std::string> ips_to_allocate;
    };

    struct prefix_info {
        prefix_metadata metadata;
        Ptr<Node> server_dst;
        NetDeviceContainer link;
    };

    //Reads a file with RTTs.
    std::vector<double> getRtts(std::string rttsFile, uint32_t max_lines = 5000000);

    std::vector<flow_size_metadata> getFlowSizes(std::string flowSizeFile, uint32_t max_lines = 5000000);

    std::pair<Ptr<Node>, Ptr<Node>> rttToNodePair(std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_senders,
                                                  std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_receivers,
                                                  double rtt);

    //New headers
    template<typename T>
    std::vector<T> read_lines(std::string file_name, uint32_t max_lines = 20000000) {

        std::vector<T> lines;
        T line;
        uint32_t line_count = 0;

        std::ifstream file_in(file_name);
        NS_ASSERT_MSG(file_in, "Invalid File " + file_name);

        while (file_in >> line and (line_count < max_lines or max_lines == 0)) {
            lines.push_back(line);
        }
        return lines;
    }


std::unordered_map<std::string, prefix_metadata> getPrefixes(std::string prefixesFile, std::string prefixesLossFile,
                                                                 uint32_t max_prefixes = 20000000);
    std::vector<std::string> getPrefixesToFail(std::string prefixesToFailFile);
    std::vector<flow_metadata> getPrefixesDistribution(std::string prefixesDistributionFile, uint32_t max_flows = 20000000);
    }

#endif /* SWIFT_UTILS_H */

