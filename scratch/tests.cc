//
// Created by edgar costa molero on 19.04.18.
//

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
#include "ns3/utils-module.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <algorithm>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("test");

void printNow(void){
    std::cout << Simulator::Now().GetSeconds() << " " << Simulator::GetContext() << "\n";
}
static double
CbOne (double a, double b)
{
    std::cout << "invoke cbOne a=" << a << ", b=" << b << std::endl;
    return a;
}

class MyCb {
public:
    int CbTwo (double a) {
        std::cout << "invoke cbTwo a=" << a << std::endl;
        return -5;
    }
};

int
main(int argc, char *argv[]) {


//    for (int i= 0; i < 100; i++){
//        //Simulator::Schedule(Seconds(i+1), &printNow);
//        Simulator::ScheduleNow(&printNow);
//    }

    Callback<double, double, double> one;
    one = MakeCallback(&CbOne);

    std::cout << one(10, 20) << "\n";

    Callback<int, double> two;
    MyCb cb;
    two  = MakeCallback(&MyCb::CbTwo, &cb);

    std::cout << two(5) << "\n";

    Simulator::Run();
    Simulator::Destroy();

//    ip_mask address = GetIpMask("192.168.1.0/24");
//    std::cout << address.ip << "\n" << address.mask << "\n";
//    NS_LOG_UNCOND("TEST\n");
    return 0;

    //Load Subnetwork Prefix mappings
    PrefixMappings mappings = GetSubnetworkPrefixMappings("/home/cedgar/traffic-generator/inputs/caida_dirA/subnetwork_prefixes.txt", "Subnetwork_0");
    std::unordered_map<std::string, prefix_features> features =  GetPrefixFeatures("/home/cedgar/traffic-generator/inputs/caida_dirA/prefix_loss.txt", mappings.trace_set);

    //Populate Prefixes Object
    std::unordered_map<std::string, prefix_metadata> prefixes = LoadPrefixesMetadata(mappings, features);

    for (auto it: prefixes){
        std::cout << it.first << "\n";
    }

    return 0;

//    Ipv4AddressHelper address("10.0.0.0", "255.255.0.0");
//    for (int j=0; j < 1000; j++)
//    {
//        Ipv4Address host_addr = address.NewAddress();
//        std::cout << Ipv4AddressToString(host_addr) << "\n";
//    }

//    std::vector<double> src_rtts = GetSubnetworkRtts("/home/cedgar/traffic-generator/inputs/caida_dirA/rtt_cdfs.txt", "Subnetwork_2");
//
//    std::cout << "Rtt to src length: " << src_rtts.size() << "\n";
//    for (const auto &e : src_rtts) {
//        std::cout << "Rtt(s): " << e << " Link Delay(s): " << e / 2 << "\n";
//    }
//
//    std::unordered_map<std::string, std::vector<failure_event>> events = GetPrefixFailures("/home/cedgar/traffic-generator/inputs/caida_dirA/prefixes_failure.txt", "Subnetwork 0");
//
//    for (auto it: events){
//        std::cout << "Prefix: " << it.first << " " << it.second.size() << "\n";
//
//        for (const auto &e: it.second){
//            std::cout << e.failure_time << " " << e.recovery_time <<" " << e.failure_intensity << "\n";
//        }
//    }

    //prefix_mappings mappings = GetSubnetworkPrefixMappings("/home/cedgar/traffic-generator/inputs/caida_dirA/subnetwork_prefixes.txt", "Subnetwork_0");

//    int test_prefixes_coun = 0;
//    for (auto it: mappings.trace_to_sim){
//        if (it.second.size() > 1){
//            test_prefixes_coun++;
//        }
//    }
//    std::cout << test_prefixes_coun << "\n";
//    return 0;

    //std::unordered_map<std::string, prefix_features> features =  GetPrefixFeatures("/home/cedgar/traffic-generator/inputs/caida_dirA/prefix_loss.txt", mappings.trace_set);
    //std::cout << features.size() << "\n";

    //std::vector<flow_metadata> flows = GetFlowsPerPrefix("/home/cedgar/traffic-generator/inputs/caida_dirA/caida_dirA_10_flows_per_prefix.txt", mappings.trace_to_sim);
    //std::cout << flows.size() << "\n";
//    int counter = 0;
//    for (auto element: flows){
//        std::cout << element.prefix << " "<< element.bytes << " "<< element.duration << " "<< element.packets << " "<< element.rtt << "\n";
//        counter++;
//
//        if (counter >10){
//            return 0;
//        }
//    }



//    for (int i=0; i < 1000; i++){
//        Ipv4Address addr = address.NewNetwork();
//        std::cout << Ipv4AddressToString(addr) << "\n";
//    }
//
//// Initialize an unordered_map through initializer_list
//    std::unordered_map<std::string, int> wordMap(
//            {
//                    {"First",  1},
//                    {"Second", 2},
//                    {"Third",  3}});
//
//// Iterate over an unordered_map using range based for loop
//    for (std::pair<std::string, int> element : wordMap) {
//        std::cout << element.first << " :: " << element.second << std::endl;
//    }
//
//    std::cout << "*******************" << std::endl;
//
//// Get an iterator pointing to begining of map
//    std::unordered_map<std::string, int>::iterator it = wordMap.begin();
//
//// Iterate over the map using iterator
//    while (it != wordMap.end()) {
//        std::cout << it->first << " :: " << it->second << std::endl;
//        it++;
//    }
//
//    std::cout << "*******************" << std::endl;
//
//    std::for_each(wordMap.begin(), wordMap.end(),
//                  [](std::pair<std::string, int> element) {
//                      std::cout << element.first << " :: " << element.second << std::endl;
//                  });



    return 0;
}
