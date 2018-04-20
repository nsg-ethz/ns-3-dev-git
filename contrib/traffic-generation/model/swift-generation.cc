/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <random>
#include "traffic-generation.h"
#include "swift-generation.h"
#include "ns3/custom-applications-module.h"
#include "ns3/applications-module.h"

#include "ns3/swift-utils.h"


NS_LOG_COMPONENT_DEFINE ("swift-generation");


namespace ns3 {

    NodesUsage  sendSwiftTraffic(std::unordered_map<double, std::vector<Ptr<Node>>> rtt_to_senders,
                                 std::vector<double> rtt_cdf,
                                 std::unordered_map<std::string, Ptr<Node>> prefix_to_dst,
                                 std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                                 std::string flowDistFile,
                                 uint32_t seed,
                                 uint32_t interArrivalFlow,
                                 double duration, double rtt_shift){


        //Load flow distribution
        std::vector<flow_metadata> flowDist = getPrefixesDistribution(flowDistFile);

        //Usage object
        NodesUsage nodes_usage = NodesUsage();

        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        gen.seed(seed);
        std::exponential_distribution<double> interArrivalTime(interArrivalFlow);

        double startTime = 1.0;
        double simulationTime = duration;

//        double bytes_per_sec = 0;
//        double time_reference = 1;
//        std::ofstream out_rates("bytes_per_sec.txt");

        while ((startTime -1) < simulationTime){

            //get a flow
            flow_metadata flow = randomFromVector<flow_metadata>(flowDist);

            flow.rtt = flow.rtt * rtt_shift;

            double rtt = find_closest(rtt_cdf , flow.rtt);

            NS_ASSERT_MSG(rtt_to_senders[rtt].size() >= 1, "There are no source hosts for rtt: " <<rtt);

            Ptr<Node> src = randomFromVector<Ptr<Node>>(rtt_to_senders[rtt]);


            //Get Destination
            Ptr<Node> dst = prefix_to_dst[flow.prefix_ip + "/" + flow.prefix_mask];

            NS_LOG_DEBUG(
                    "Flow Features:" << "          Real_rtt: " << flow.rtt
                                     << "\tRtt Found: " << rtt
                                     << "\tSrc: " << GetNodeName(src)  << "(" << GetNodeIp(src) << ")"
                                     << "\tDst: " << GetNodeName(dst)  << "(" << GetNodeIp(dst) << ")");

            //Destination port
            std::vector<uint16_t> availablePorts = hostsToPorts[GetNodeName(dst)];
            uint16_t dport = randomFromVector<uint16_t>(availablePorts);

            startTime += interArrivalTime(gen);

//            installBulkSend(src, dst, dport, flowSize, startTime);
            installRateSend(src,dst,dport,flow.packets, flow.bytes, flow.duration, startTime);
            nodes_usage.update(src, dst, flow.bytes);
//            if (flow.duration == 0){
//                flow.duration = 1;
//            }

//            bytes_per_sec = bytes_per_sec + ((double(flow.bytes) / flow.duration)
//                                             * std::min(std::min(flow.duration,uint32_t(8)),uint32_t(8 - (startTime-1))));
//
//            if ((startTime -1) >= time_reference) {
//                out_rates << (startTime - 1 ) << " " << bytes_per_sec << "\n";
//                time_reference +=1;
//            }

        }

        return nodes_usage;
    }


    NodesUsage  sendTestTraffic(std::unordered_map<double, std::vector<Ptr<Node>>> rtt_to_senders,
                                 std::vector<double> rtt_cdf,
                                 std::unordered_map<std::string, Ptr<Node>> prefix_to_dst,
                                 std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                                 std::string flowDistFile,
                                 uint32_t num_flows,
                                 double duration, double rtt_shift){


        //Load flow distribution
        std::vector<flow_metadata> flowDist = getPrefixesDistribution(flowDistFile);

        //Usage object
        NodesUsage nodes_usage = NodesUsage();

        double startTime = 1.0;
        double simulationTime = duration;

//        double bytes_per_sec = 0;
//        double time_reference = 1;
//        std::ofstream out_rates("bytes_per_sec.txt");

        while ((startTime -1) < simulationTime){

            for (uint32_t i = 0;  i < num_flows; i++) {
                //get a flow
                flow_metadata flow = randomFromVector<flow_metadata>(flowDist);


                flow.rtt = flow.rtt * rtt_shift;

                //Get Source
                double rtt = find_closest(rtt_cdf, flow.rtt);

                NS_ASSERT_MSG(rtt_to_senders[rtt].size() >= 1, "There are no source hosts for rtt: " << rtt);

                Ptr<Node> src = randomFromVector<Ptr<Node>>(rtt_to_senders[rtt]);



                //Get Destination
                Ptr<Node> dst = prefix_to_dst[flow.prefix_ip + "/" + flow.prefix_mask];

                NS_LOG_DEBUG(
                        "Flow Features:" << "          Real_rtt: " << flow.rtt
                                         << "\tRtt Found: " << rtt
                                         << "\tSrc: " << GetNodeName(src)  << "(" << GetNodeIp(src) << ")"
                                         << "\tDst: " << GetNodeName(dst)  << "(" << GetNodeIp(dst) << ")");

                //Destination port
                std::vector<uint16_t> availablePorts = hostsToPorts[GetNodeName(dst)];
                uint16_t dport = randomFromVector<uint16_t>(availablePorts);


//            installBulkSend(src, dst, dport, flowSize, startTime);
                installRateSend(src, dst, dport, flow.packets, flow.bytes, flow.duration, startTime);
                std::cout << startTime << "\n";
                nodes_usage.update(src, dst, flow.bytes);

//                if (flow.duration == 0) {
//                    flow.duration = 1;
//                }

//                bytes_per_sec = bytes_per_sec + ((double(flow.bytes) / flow.duration)
//                                                 * std::min(std::min(flow.duration, uint32_t(8)),
//                                                            uint32_t(8 - (startTime - 1))));
//
//                if ((startTime - 1) >= time_reference) {
//                    out_rates << (startTime - 1) << " " << bytes_per_sec << "\n";
//                    time_reference += 1;
//                }
            }
            startTime++;
        }

        return nodes_usage;
    }





    NodesUsage sendSwiftTraffic_old(std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_senders,
                          std::unordered_map<uint64_t, std::vector<Ptr<Node>>> rtt_to_receivers,
                          std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts,
                          std::string rttFile,
                          std::string flowSizeFile,
                          uint32_t seed,
                          uint32_t interArrivalFlow,
                          double duration){


        //Load RTT File
        std::vector<double> rtts ;//= getRtts(rttFile);
        std::vector<flow_size_metadata> flowSizes = getFlowSizes(flowSizeFile);

        //Usage object
        NodesUsage nodes_usage = NodesUsage();

        //Load Flow sizes File

        //Random generator to select variables...
        //Exponential distribution to select flow inter arrival time per sender
        Ptr<UniformRandomVariable> random_size = CreateObject<UniformRandomVariable> ();


        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        gen.seed(seed);
        std::exponential_distribution<double> interArrivalTime(interArrivalFlow);

        double startTime = 1.0;
        double simulationTime = duration;

        //TEST TO SEE HOSTS DISTRIBUTION
        std::unordered_map<std::string, uint32_t> sender_flows_count;


        while ((startTime -1) < simulationTime){

            double rtt_sample = randomFromVector<double>(rtts);
            flow_size_metadata size_sample = randomFromVector(flowSizes);


            std::pair<Ptr<Node>, Ptr<Node>> pair = rttToNodePair(rtt_to_senders, rtt_to_receivers, rtt_sample);
            while(pair.first == 0  or pair.second == 0){
                rtt_sample = randomFromVector<double>(rtts);
                pair = rttToNodePair(rtt_to_senders, rtt_to_receivers, rtt_sample);
            }

            Ptr<Node> src = pair.first;
            Ptr<Node> dst = pair.second;

            //Store this node in the latency to node map
            if (sender_flows_count.count(GetNodeName(src)) > 0 )
            {
                sender_flows_count[GetNodeName(src)] +=1;
            }
            else
            {
                sender_flows_count[GetNodeName(src)] = 1;
            }

            NS_LOG_DEBUG("Flow Features: rtt:" << rtt_sample << " src:" << GetNodeName(src) <<
                                               "(" << GetNodeIp(src) << ")" << " dst:" << GetNodeName(dst) << "(" << GetNodeIp(dst) << ")");
            //<< " Size: " << size_sample.duration << " " << size_sample.packets << " " << size_sample.bytes);

            //Destination port
            std::vector<uint16_t> availablePorts = hostsToPorts[GetNodeName(dst)];
            uint16_t dport = randomFromVector<uint16_t>(availablePorts);

            //Get Flow size sample
//            uint64_t flowSize = random_size->GetInteger(1000,500000);

            startTime += interArrivalTime(gen);

//            installBulkSend(src, dst, dport, flowSize, startTime);
            installRateSend(src,dst,dport,size_sample.packets, size_sample.bytes, size_sample.duration, startTime);
            nodes_usage.update(src, dst, size_sample.bytes);
        }


        //Prin sender_flows_count if debug.

        /*for (auto it = sender_flows_count.begin(); it != sender_flows_count.end(); it++){

            NS_ASSERT_MSG(it->second < 62000, "Too many bindings at host: " + it->first);
            NS_LOG_DEBUG("Host " << it->first << " sends: " << it->second << " flows");
        }*/

        return nodes_usage;

    }

}
