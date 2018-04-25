/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <fstream>
#include <ctime>
#include <set>
#include <string>
#include <iostream>
#include <unordered_map>
#include <stdlib.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/traffic-generation-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/utils.h"
#include "ns3/swift-utils.h"

// TOPOLOGY
//+---+                                                 +---+
//|s1 |                                           +-----+d1 |
//+---+----+                                      |     +---+
//         |                                      |
//  .      |    +------+              +------+    |       .
//  .      +----+      |              |      +----+       .
//  .           |  sw1 +--------------+  sw2 |            .
//  .           |      |              |      +---+        .
//  .      +----+------+              +------+   |        .
//         |                                     |
//+---+    |                                     |      +---+
//|sN +----+                                     +------+dN |
//+---+                                                 +---+

using namespace ns3;

std::string fileNameRoot = "congestion-test";    // base name for trace files, etc

NS_LOG_COMPONENT_DEFINE (fileNameRoot);

const char *file_name = g_log.Name();
std::string script_name = file_name;

int
main(int argc, char *argv[]) {

    //INITIALIZATION
    std::clock_t simulation_execution_time = std::clock();


    //Set simulator's time resolution (click)
    Time::SetResolution(Time::NS);

    //Fat tree parameters
    DataRate networkBandwidth("100Gbps");
    DataRate sendersBandwidth("4Gbps");
    DataRate receiversBandwidth("4Gbps");

    //Command line arguments
    std::string simulationName = "test";
    std::string outputDir = "outputs/";

    std::string inputDir = "swift_datasets/test/";
    std::string prefixes_failures_file = "";
    std::string prefixes_features_file = "";

    uint16_t queue_size = 1000;
    uint64_t runStep = 1;
    double rtt_shift = 1;

    bool enable_uniform_loss = false;
    double prefixes_loss = 0;
    uint64_t network_delay = 1; //ns?
    uint16_t num_hosts_per_rtt = 1;
    uint16_t num_servers_per_prefix = 1;
    uint32_t flowsPersec = 100;
    double duration = 5;
    double failure_time = 0;
    double stop_time = 0;
    bool debug = false;
    bool save_pcap = false;
    bool fail_all_prefixes = false;

    CommandLine cmd;

    //General
    //Links properties
    cmd.AddValue("LinkBandwidth", "Bandwidth of link, used in multiple experiments", networkBandwidth);
    cmd.AddValue("NetworkDelay", "Added delay between nodes", network_delay);

    //Experiment
    cmd.AddValue("QueueSize", "Interfaces Queue length", queue_size);
    cmd.AddValue("RunStep", "Random generator starts at", runStep);

    cmd.Parse(argc, argv);

    //Change that if i want to get different random values each run otherwise i will always get the same.
    RngSeedManager::SetRun(runStep);   // Changes run number from default of 1 to 7

    //Update root name
    std::ostringstream run;
    run << runStep;

    //Timeout calculations (in milliseconds)
    std::string outputNameRoot = outputDir;

    outputNameRoot = outputNameRoot + simulationName + "/";

    //TCP
    uint16_t rtt = 200;
    uint16_t initial_rto = 2000;

    //GLOBAL CONFIGURATION
    //Routing

    Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

    //Tcp Socket (general socket conf)
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(4000000));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(4000000));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1446)); //MTU 1446
    Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(4294967295));
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));

    //Can be much slower than my rtt because packet size of syn is 60bytes
    Config::SetDefault("ns3::TcpSocket::ConnTimeout",
                       TimeValue(MilliSeconds(initial_rto))); // connection retransmission timeout
    Config::SetDefault("ns3::TcpSocket::ConnCount", UintegerValue(5)); //retrnamissions during connection
    Config::SetDefault("ns3::TcpSocket::DataRetries", UintegerValue(15)); //retranmissions
    Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(MilliSeconds(rtt / 50)));
// 	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue(2));
    Config::SetDefault("ns3::TcpSocket::TcpNoDelay", BooleanValue(true)); //disable nagle's algorithm
    Config::SetDefault("ns3::TcpSocket::PersistTimeout",
                       TimeValue(NanoSeconds(6000000000))); //persist timeout to porbe for rx window

    //Tcp Socket Base: provides connection orientation, sliding window, flow control; congestion control is delegated to the subclasses (i.e new reno)

    Config::SetDefault("ns3::TcpSocketBase::MaxSegLifetime", DoubleValue(10));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MilliSeconds(rtt))); //min RTO value that can be set
    Config::SetDefault("ns3::TcpSocketBase::ClockGranularity", TimeValue(MicroSeconds(1)));
    Config::SetDefault("ns3::TcpSocketBase::ReTxThreshold", UintegerValue(3)); //same than DupAckThreshold

    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(true)); //enable sack
    Config::SetDefault("ns3::TcpSocketBase::LimitedTransmit", BooleanValue(true)); //enable sack

    //TCP L4
    //TcpNewReno
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
    Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue(MicroSeconds(initial_rto)));//defautlt 1sec

    //QUEUES
    //PFIFO
    Config::SetDefault("ns3::PfifoFastQueueDisc::Limit", UintegerValue(queue_size));

    //Define acsii helper
    AsciiTraceHelper asciiTraceHelper;

    //Define Interfaces

    //Define point to point
    PointToPointHelper p2p;

//   create point-to-point link from A to R
    p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(networkBandwidth)));
    p2p.SetChannelAttribute("Delay", TimeValue(MicroSeconds(network_delay)));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(1500));

     //New way of setting the queue length and mode
    std::stringstream queue_size_str;
    queue_size_str << queue_size << "p";
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(queue_size_str.str()));

    //SIMULATION STARTS
    std::clock_t begin_time = std::clock();


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //Build Topology
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    //Number of senders and receivers
    int num_senders = 4;
    int num_receivers = 1;
    //Senders
    NodeContainer senders;
    senders.Create(num_senders);

    NodeContainer receivers;
    senders.Create(num_receivers);

    //naming

    uint32_t i = 0;
    for (NodeContainer::Iterator host = senders.Begin(); host != senders.End(); host++, i++) {

        //Assign host name, and add it to the names system
        std::stringstream host_name;
        host_name << "s" << i;
        Names::Add(host_name.str(), (*host));

    }
    i = 0;
    for (NodeContainer::Iterator host = receivers.Begin(); host != receivers.End(); host++, i++) {

        //Assign host name, and add it to the names system
        std::stringstream host_name;
        host_name << "d" << i;
        Names::Add(host_name.str(), (*host));
    }

    //Single link Switches
    Ptr<Node> sw1 = CreateObject<Node>();
    Ptr<Node> sw2 = CreateObject<Node>();

    //Add two switches
    Names::Add("sw1", sw1);
    Names::Add("sw2", sw2);

    // Install Internet stack : aggregate ipv4 udp and tcp implementations to nodes
    InternetStackHelper stack;
    stack.Install(senders);
    stack.Install(d1);
    stack.Install(sw1);
    stack.Install(sw2);

    //Install net devices between nodes (so add links) would be good to save them somewhere I could maybe use the namesystem or map
    //Install internet stack to nodes, very easy
    std::unordered_map<std::string, NetDeviceContainer> links;

    //SETTING LINKS: delay and bandwidth.

    //sw1 -> sw2
    //Interconnect middle switches : sw1 -> sw2
    links[GetNodeName(sw1) + "->" + GetNodeName(sw2)] = p2p.Install(NodeContainer(sw1, sw2));
    //Set delay and bandwdith
    links[GetNodeName(sw1) + "->" + GetNodeName(sw2)].Get(0)->GetChannel()->SetAttribute("Delay", TimeValue(
            NanoSeconds(network_delay)));
    links[GetNodeName(sw1) + "->" + GetNodeName(sw2)].Get(0)->SetAttribute("DataRate", DataRateValue(networkBandwidth));

    NS_LOG_DEBUG("Link Add: " << GetNodeName(sw1)
                              << " -> "
                              << GetNodeName(sw2)
                              << " delay(ns):"
                              << network_delay
                              << " bandiwdth"
                              << networkBandwidth);


    //Assing senders, sw1, sw2 ips since they do not depend on the prefixes
    Ipv4AddressHelper address("10.0.1.0", "255.255.255.0");
    address.Assign(links[GetNodeName(sw1) + "->" + GetNodeName(sw2)]);
    address.NewNetwork();

    //Senders
    for (NodeContainer::Iterator host = senders.Begin(); host != senders.End(); host++) {

        //add link host-> sw1
        links[GetNodeName(*host) + "->" + GetNodeName(sw1)] = p2p.Install(NodeContainer(*host, sw1));
        links[GetNodeName(*host) + "->" + GetNodeName(sw1)].Get(0)->SetAttribute("DataRate", DataRateValue(sendersBandwidth));

        //Assign IP
        address.Assign(links[GetNodeName(*host) + "->" + GetNodeName(sw1)]);
        address.NewNetwork();

    }
    //Receivers
    for (auto host = receivers.begin(); it != receivers.end(); host++) {

        links[GetNodeName(sw2) + "->" + GetNodeName(*host)] = p2p.Install(NodeContainer(sw2, receivers.Get(*host)));
        links[GetNodeName(sw2) + "->" + GetNodeName(*host)].Get(0)->SetAttribute("DataRate", DataRateValue(receiversBandwidth));
        address.Assign(links[GetNodeName(sw2) + "->" + GetNodeName(*host)]);
        address.NewNetwork();

    }

    //Assign IPS
    //Uninstall FIFO queue //  //uninstall qdiscs
    TrafficControlHelper tch;
    for (auto it : links) {
        tch.Uninstall(it.second);
    }

    //DEBUG FEATURE
    //Create a ip to node mapping, saves the ip to name for debugging
    std::unordered_map<std::string, Ptr<Node>> ipToNode;

    for (uint32_t host_i = 0; host_i < senders.GetN(); host_i++) {
        Ptr<Node> host = senders.Get(host_i);
        ipToNode[ipv4AddressToString(GetNodeIp(host))] = host;
    }
    for (uint32_t host_i = 0; host_i < receivers.GetN(); host_i++) {
        Ptr<Node> host = receivers.Get(host_i);
        ipToNode[ipv4AddressToString(GetNodeIp(host))] = host;
    }
    //Store in a file ip -> node name
    Ptr<OutputStreamWrapper> ipToName_file = asciiTraceHelper.CreateFileStream(outputNameRoot + "ip_to_name.txt");
    for (auto it = ipToNode.begin(); it != ipToNode.end(); it++) {
        *(ipToName_file->GetStream()) << it->first << " " << GetNodeName(it->second) << "\n";
    }
    ipToName_file->GetStream()->flush();

    //Saving metadata file with simulation configurations
    Ptr<OutputStreamWrapper> metadata_file = asciiTraceHelper.CreateFileStream(outputNameRoot + "metadata.txt");
    *(metadata_file->GetStream()) << "simulation_name " << simulationName << "\n";
    *(metadata_file->GetStream()) << "run_step " << runStep << "\n";
    *(metadata_file->GetStream()) << "duration " << duration << "\n";
    *(metadata_file->GetStream()) << "failure_time " << failure_time << "\n";
    *(metadata_file->GetStream()) << "flows_per_sec " << flowsPersec << "\n";
    *(metadata_file->GetStream()) << "input_dir " << inputDir << "\n";
    *(metadata_file->GetStream()) << "output_dir " << outputDir << "\n";
    *(metadata_file->GetStream()) << "queue_size " << queue_size << "\n";
    *(metadata_file->GetStream()) << "network_bw " << networkBandwidth << "\n";
    *(metadata_file->GetStream()) << "senders_bw " << sendersBandwidth << "\n";
    *(metadata_file->GetStream()) << "receivers_bw " << receiversBandwidth << "\n";
    *(metadata_file->GetStream()) << "emulated_congestion_on " << enable_uniform_loss << "\n";
    *(metadata_file->GetStream()) << "prefixes_loss " << prefixes_loss << "\n";
    *(metadata_file->GetStream()) << "rtt_cdf_size " << src_rtts.size() << "\n";
    *(metadata_file->GetStream()) << "num_senders " << num_senders << "\n";
    *(metadata_file->GetStream()) << "num_receivers " << num_receivers << "\n";
    *(metadata_file->GetStream()) << "prefixes " << prefixes.size() << "\n";
    *(metadata_file->GetStream()) << "prefixes_failed " << prefix_failures.size() << "\n";
    *(metadata_file->GetStream()) << "rtt_shift " << rtt_shift << "\n";
    metadata_file->GetStream()->flush();

    NS_LOG_DEBUG("Time To Set Hosts: " << float(clock() - begin_time) / CLOCKS_PER_SEC);

    //Populate tables
    begin_time = std::clock();

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    float routing_time = float(clock() - begin_time) / CLOCKS_PER_SEC;
    *(metadata_file->GetStream()) << "routing_time " << routing_time << "\n";
    metadata_file->GetStream()->flush();
    NS_LOG_DEBUG("Time Installing Routes: " << routing_time);

    //START TRAFFIC
    //Install Traffic sinks at receivers

    NS_LOG_DEBUG("Starting Sinks");
    begin_time = std::clock();

    std::unordered_map<std::string, std::vector<uint16_t>> hostToPort = installSinks(receivers, 35, 0, "TCP");

    NS_LOG_DEBUG("Time Starting Sinks: " << float(clock() - begin_time) / CLOCKS_PER_SEC);


    NS_LOG_DEBUG("Starting Traffic Scheduling");
    begin_time = std::clock();

    NodesUsage nodes_usage = NodesUsage();

//    std::cout << "Rtt to src length: " << senders_latency_to_node.size() << "\n";
//    for(auto it = senders_latency_to_node.begin(); it != senders_latency_to_node.end(); it++){
//        std::cout << "rtt: " << it->first << " " << it->second.size() << "\n";
//    }

    nodes_usage = sendSwiftTraffic(senders_latency_to_node,
                                   src_rtts,
                                   prefixes,
                                   prefixes_mappings,
                                   hostToPort,
                                   inputDir + "flows_per_prefix.txt",
                                   runStep,
                                   flowsPersec,
                                   duration,
                                   rtt_shift);

    //save ranks
    nodes_usage.save(outputNameRoot + "bytes", false, true);

    std::vector<usage_struct> receivers_vector = nodes_usage.getReceiversVector();
    float time_scheduling_traffic = float(clock() - begin_time) / CLOCKS_PER_SEC;
    *(metadata_file->GetStream()) << "time_scheduling_traffic " << time_scheduling_traffic << "\n";
    metadata_file->GetStream()->flush();
    NS_LOG_DEBUG("Time Scheduling: " << time_scheduling_traffic);

    //Senders function
    //////////////////
    //TRACES
    ///////////////////

    //Capture traffic between sw1 and sw2 at the first switch.
//  p2p.EnablePcap(outputNameRoot, links[GetNodeName(sw1)+"->"+GetNodeName(sw2)].Get(0), bool(1));
//
//  //Only save TX traffic
    if (save_pcap) {

        //p2p.EnablePcap(outputNameRoot + "tx_rx.pcap", links[GetNodeName(sw1) + "->" + GetNodeName(sw2)].Get(0), bool(1));

        PcapHelper pcapHelper;
        Ptr<PcapFileWrapper> pcap_file = pcapHelper.CreateFile(
                outputNameRoot + "tx.pcap", std::ios::out,
                PcapHelper::DLT_PPP);
        links[GetNodeName(sw1) + "->" + GetNodeName(sw2)].Get(0)->TraceConnectWithoutContext("PhyTxBegin",
                                                                                             MakeBoundCallback(&TracePcap,
                                                                                                               pcap_file));
    }

    NS_LOG_DEBUG("Total Bytes Received By Servers: " << nodes_usage.get_total_rx());



    //TODO: DO this better and cleaner
    //Schedule Prefixes to fail

    Ptr<OutputStreamWrapper> prefixes_failed_file = asciiTraceHelper.CreateFileStream(
            outputNameRoot + "failed_prefixes.txt");

    //we ignore what the prefix file says and we fail all the prefixes at failure_time.
    if (fail_all_prefixes){

        if (failure_time > 0) {
            for (auto it : prefixes) {
                NetDeviceContainer link = it.second.link;

                NS_LOG_DEBUG("Scheduling prefix fail: " << it.first);

                *(prefixes_failed_file->GetStream()) << it.first << "\n";
                prefixes_failed_file->GetStream()->flush();

                Simulator::Schedule(Seconds(failure_time), &FailLink, link);
            }
        }
    }

    else{
        for (auto prefix_to_fail: prefix_failures) {

            NetDeviceContainer link = prefixes[prefix_to_fail.first].link;
            NS_LOG_DEBUG("Scheduling prefix fail: " << prefix_to_fail.first);

            *(prefixes_failed_file->GetStream()) << prefix_to_fail.first << "\n";
            prefixes_failed_file->GetStream()->flush();

            //IF there is no events and the default duration was provided we use that
            if (prefix_to_fail.second.size() == 0 and failure_time > 0){
                Simulator::Schedule(Seconds(failure_time), &FailLink, link);
                continue;
            }

            for (auto failure: prefix_to_fail.second){
                if (failure.failure_time > 0 ){
                    Simulator::Schedule(Seconds(failure.failure_time), &ChangeLinkDropRate, link, failure.failure_intensity);
                }
                if (failure.recovery_time > 0) {
                    //set back to loss level
                    Simulator::Schedule(Seconds(failure.failure_time), &ChangeLinkDropRate, link,
                                        prefixes[prefix_to_fail.first].features.loss);
                }
            }
        }
    }

    //Simulation Starts

    if (stop_time != 0) {
        //We schedule it here otherwise simulations never finish
        std::cout <<"\n";
        printNowMem(1, simulation_execution_time);
        Simulator::Stop(Seconds(stop_time));
    }

    Simulator::Run();

    *(metadata_file->GetStream()) << "real_time_duration " << (float(clock() - simulation_execution_time) / CLOCKS_PER_SEC) << "\n";
    metadata_file->GetStream()->flush();
    Simulator::Destroy();

    return 0;
}
