/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SWIFT_UTILS_H
#define SWIFT_UTILS_H

#include <vector>
#include <string>
#include <unordered_map>
#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3 {

    struct flow_metadata {
        std::string prefix;
        uint32_t packets;
        double duration;
        uint64_t bytes;
        double rtt;
    };

    struct prefix_features {
        double loss;
        double out_of_order;
    };

// Possible structure when using multiple server at the receivers side
//    struct prefix_metadata {
//        std::string trace_prefix;
//        prefix_features features;
//        NetDeviceContainer main_link;
//        std::vector<Ptr<Node>> servers;
//        std::unordered_map<std::string, NetDeviceContainer> server_links;
//    };

    struct prefix_metadata {
        std::string trace_prefix;
        prefix_features features;
        NetDeviceContainer link;
        Ptr<Node> server;
    };


    struct failure_event {
        double failure_time;
        double recovery_time;
        float failure_intensity;
    };

    struct prefix_mappings {
        std::unordered_map<std::string, std::string> sim_to_trace;
        std::unordered_map<std::string, std::set<std::string>> trace_to_sim;
        std::set<std::string> trace_set;
        std::set<std::string> sim_set;
    };

    std::vector<flow_metadata> GetFlowsPerPrefix(std::string flows_per_prefix_file, std::unordered_map<std::string, std::set<std::string>> trace_to_sim_prefixes);
    std::unordered_map<std::string, std::vector<failure_event>> GetPrefixFailures(std::string prefix_failure_file, std::string subnetwork_name);
    std::vector<double> GetSubnetworkRtts(std::string rttsFile, std::string subnet_name);
    prefix_mappings GetSubnetworkPrefixMappings(std::string prefixesFile, std::string subnetwork_name);
    std::unordered_map<std::string, prefix_features> GetPrefixFeatures(std::string prefixFeaturesFile, std::set<std::string> subnetwork_trace_prefixes);
    std::unordered_map<std::string, prefix_metadata> LoadPrefixesMetadata
            (prefix_mappings mappings, std::unordered_map<std::string, prefix_features> trace_prefixes_features);

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

}

#endif /* SWIFT_UTILS_H */

