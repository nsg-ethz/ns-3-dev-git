/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "swift-utils.h"
#include "utils.h"

NS_LOG_COMPONENT_DEFINE ("swift-utils");


namespace ns3 {

    //Reads a file with RTTs.
    std::vector<double> get_subnetwork_rtts(std::string rttsFile, std::string subnet_name, uint32_t max_lines) {

        std::vector<double> rttVector;
        std::ifstream infile(rttsFile);

        NS_ASSERT_MSG(infile, "Please provide a valid file for reading RTT values");
        double rtt;
        std::string line;
        uint32_t count_limit = 0;
        bool subnetwork_found = false;

        while (getline(infile, line) and (count_limit < max_lines or max_lines == 0)) {

            //Check if the line starts with #
            if (0 == line.find("#")){
                if (line.find(subnet_name) != std::string::npos){
                    subnetwork_found = true;
                }
                else if (subnetwork_found == true){
                    break;
                }
            }

            else if (subnetwork_found == true){
                rtt = atof(line.c_str());
                rttVector.push_back(rtt);
                count_limit++;
            }
        }

        infile.close();
        return rttVector;
    }

    std::vector<flow_size_metadata> getFlowSizes(std::string flowSizeFile, uint32_t max_lines) {
        std::vector<flow_size_metadata> flows;

        std::ifstream infile(flowSizeFile);

        NS_ASSERT_MSG(infile, "Please provide a valid file for the flow Size values");

        flow_size_metadata flow_metadata;

        uint32_t count_limit = 0;
        while (infile >> flow_metadata.packets >> flow_metadata.duration >> flow_metadata.bytes and
               (count_limit < max_lines or max_lines == 0)) {
            flows.push_back(flow_metadata);
            count_limit++;
        }
        infile.close();

        return flows;
    }

    std::pair<Ptr<Node>, Ptr<Node>> rttToNodePair(std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_senders,
                                                  std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_receivers,
                                                  double rtt) {

        //I assume that the RTT is in seconds so first we convert it to time
        //Since rtt = propagation time * 2 , we devide rtt time by 2
        Time rtt_t = Time((rtt / 2) * 1e9);

        std::pair<Ptr<Node>, Ptr<Node>> src_and_dst;

        uint64_t sender_delay = leftMostPowerOfTen(rtt_t.GetInteger());
        uint64_t receiver_delay = leftMostPowerOfTen(rtt_t.GetInteger() - sender_delay);

        NS_LOG_DEBUG("Sender's Link Delay: " << Time(sender_delay).GetSeconds() << " --- Receiver's Link Delay: "
                                             << Time(receiver_delay).GetSeconds());

        //Assumes that the desired delay exists in the unordered map, that is a big assumption....
        //multiple hosts could be set with the same delay, so we pick up one randomly

        std::unordered_map<uint64_t, std::vector<Ptr<Node>>>::iterator iter = rtt_to_senders.find(sender_delay);

        if (iter != rtt_to_senders.end()) {
            src_and_dst.first = randomFromVector<Ptr<Node>>(iter->second);
        }

        iter = rtt_to_receivers.find(receiver_delay);
        if (iter != rtt_to_receivers.end()) {
            src_and_dst.second = randomFromVector<Ptr<Node>>(iter->second);
        }
//	src_and_dst.first  = randomFromVector(rtt_to_senders[sender_delay]);
//	src_and_dst.second = randomFromVector(rtt_to_receivers[receiver_delay]);

        return src_and_dst;

    }


    std::unordered_map<std::string, prefix_features> getPrefixFeatures(std::string prefixFeaturesFile, std::set<std::string> subnetwork_trace_prefixes){

        std::unordered_map<std::string, prefix_features> trace_prefix_features;

        std::ifstream prefix_features_file(prefixFeaturesFile);
        NS_ASSERT_MSG(prefix_features_file, "Please provide a valid prefixes file");

        std::string prefix;

        prefix_features features;

        while (prefix_features_file >> prefix >> features.loss){
            if (subnetwork_trace_prefixes.find(prefix) != subnetwork_trace_prefixes.end()){
                trace_prefix_features[prefix] = features;
            }
        }
        return trace_prefix_features;

    };

    std::set<std::string> get_trace_prefixes_set(std::string prefixesFile, std::string subnetwork_name){

        std::set<std::string> trace_prefixes_set;

        std::ifstream prefixes_file(prefixesFile);
        NS_ASSERT_MSG(prefixes_file, "Please provide a valid prefixes file");

        std::string line;
        bool subnetwork_found = false;

        std::string sim_prefix;
        std::string trace_prefix;

        while (std::getline(prefixes_file, line)){

            if (0 == line.find("#")){
                if (line.find(subnetwork_name) != std::string::npos){
                    subnetwork_found = true;
                }

                else if (subnetwork_found == true){
                    break;
                }
            }
            else if (subnetwork_found == true){
                std::istringstream lineStream(line);
                lineStream >> sim_prefix >> trace_prefix;
                trace_prefixes_set.insert(trace_prefix);
            }
        }
        return trace_prefixes_set;
    };


    std::unordered_map<std::string, std::string> getSimPrefixesToTracePrefixes(std::string prefixesFile, std::string subnetwork_name){

        //mapping
        std::unordered_map<std::string, std::string> sim_prefix_to_trace_prefix;

        std::ifstream prefixes_file(prefixesFile);
        NS_ASSERT_MSG(prefixes_file, "Please provide a valid prefixes file");

        std::string line;
        bool subnetwork_found = false;

        std::string sim_prefix;
        std::string trace_prefix;

        while (std::getline(prefixes_file, line)){

            if (0 == line.find("#")){
                if (line.find(subnetwork_name) != std::string::npos){
                    subnetwork_found = true;
                }

                else if (subnetwork_found == true){
                    break;
                }
            }
            else if (subnetwork_found == true){
                std::istringstream lineStream(line);
                lineStream >> sim_prefix >> trace_prefix;
                sim_prefix_to_trace_prefix[sim_prefix] = trace_prefix;
            }
        }
        return sim_prefix_to_trace_prefix;
    };

    //NEW VERSION
    std::unordered_map<std::string, prefix_metadata> getPrefixes(std::string prefixesFile, std::string prefixesLossFile,
                                                                 uint32_t max_prefixes){

        //empty prefixes list
        std::unordered_map<std::string, prefix_metadata> prefixes;

        std::ifstream prefixFile(prefixesFile);
        std::ifstream lossFile(prefixesLossFile);

        NS_ASSERT_MSG(prefixFile, "Please provide a valid prefixes file");
        if (!lossFile){
            NS_LOG_WARN("Prefixes loss file was not provided");
        }

        prefix_metadata prefix;
        uint32_t count_limit = 0;

        std::string line;

        while (std::getline(prefixFile, line) and
                (count_limit < max_prefixes or max_prefixes == 0)){

            std::vector<std::string> line_vector;
            std::istringstream lineStream(line);
            std::string element;
            while (lineStream >> element){
                line_vector.push_back(element);
            }

            prefix.prefix_ip = line_vector[0];
            prefix.prefix_mask = line_vector[1];

            //pop first two elements
            line_vector.erase(line_vector.begin());
            line_vector.erase(line_vector.begin());

            prefix.ips_to_allocate = line_vector;

            prefix.loss = 0;
            prefixes[prefix.prefix_ip + "/" + prefix.prefix_mask] = prefix;
            count_limit ++;
        }
        prefixFile.close();

        //Read prefix loss and update if the prefix exist
        while (lossFile >> prefix.prefix_ip >> prefix.prefix_mask  >> prefix.loss){

            if (prefixes.count(prefix.prefix_ip + "/" + prefix.prefix_mask)){
                prefixes[prefix.prefix_ip + "/" + prefix.prefix_mask].loss = prefix.loss;
            }
        }
        lossFile.close();

        return prefixes;
    }

    std::vector<std::string> getPrefixesToFail(std::string prefixesToFailFile){

        std::vector<std::string> prefixes_to_fail;

        std::ifstream prefixesFile(prefixesToFailFile);
        NS_ASSERT_MSG(prefixesFile, "Please provide a valid prefixes to fail file");

        std::string prefix;
        std::string mask;
        while (prefixesFile >> prefix >> mask){
            prefixes_to_fail.push_back(prefix+"/"+mask);
        }
        prefixesFile.close();
        return prefixes_to_fail;
    }

    std::vector<flow_metadata_new> get_flows_per_prefix(std::string flows_per_prefix_file, std::unordered_map<std::string, std::set<std::string>> trace_to_sim_prefixes){

        std::vector<flow_metadata> flows;
        std::ifstream flowsDist(flows_per_prefix_file);
        NS_ASSERT_MSG(flowsDist, "Please provide a valid prefixes dist file");

        std::string line;
        flow_metadata flow;

        std::string current_prefix;
        std::string strip_a, strip_b;

        while(std::getline(flowsDist, line)){

            //ignore empty lines
            if (0== line.find(" ")){
                continue;
            }

            else if (0== line.find("#")){
                std::istringstream lineStream(line);
                lineStream >> strip_a >> current_prefix >> strip_b;
            }

        }
        while (flowsDist >> flow.prefix_ip >> flow.prefix_mask >> flow.packets >> flow.duration >> flow.bytes >> flow.rtt
               and (count_limit < max_flows or max_flows == 0)){
            flows.push_back(flow);
            count_limit ++;
        }
        flowsDist.close();
        return flows;
    }

    std::vector<flow_metadata> getPrefixesDistribution(std::string prefixesDistributionFile, uint32_t max_flows){
        std::vector<flow_metadata> flows;
        std::ifstream flowsDist(prefixesDistributionFile);
        NS_ASSERT_MSG(flowsDist, "Please provide a valid prefixes dist file");

        flow_metadata flow;
        uint32_t count_limit;
        while (flowsDist >> flow.prefix_ip >> flow.prefix_mask >> flow.packets >> flow.duration >> flow.bytes >> flow.rtt
                and (count_limit < max_flows or max_flows == 0)){
            flows.push_back(flow);
            count_limit ++;
        }
        flowsDist.close();
        return flows;
    }
}