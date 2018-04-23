/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "sys/types.h"
#include "sys/sysinfo.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3 {

/* ... */
    extern Ptr<UniformRandomVariable> random_variable;


    struct usage_struct {
        std::string name;
        std::string ipAddr;
        uint64_t rx_flows;
        uint64_t tx_flows;
        uint64_t tx_bytes;
        uint64_t rx_bytes;
    };

    struct ip_mask {
        std::string ip;
        std::string mask;
    };

    struct less_rx_flows {
        bool operator()(const usage_struct &x, const usage_struct &y) const {
            return x.rx_flows < y.rx_flows;
        }
    };

    struct less_tx_flows {
        bool operator()(const usage_struct &x, const usage_struct &y) const {
            return x.tx_flows < y.tx_flows;
        }
    };

    struct less_rx_bytes {
        bool operator()(const usage_struct &x, const usage_struct &y) const {
            return x.rx_bytes < y.rx_bytes;
        }
    };

    struct less_tx_bytes {
        bool operator()(const usage_struct &x, const usage_struct &y) const {
            return x.tx_bytes < y.tx_bytes;
        }
    };

    class NodesUsage {

    private:

        std::unordered_map<std::string, usage_struct> senders;
        std::unordered_map<std::string, usage_struct> receivers;

        /*Have to flatten maps to sort them */
        std::vector<usage_struct> senders_vector;
        std::vector<usage_struct> receivers_vector;

        void update_tx_senders(Ptr<Node> node, uint64_t flow_size);

        void update_rx_receivers(Ptr<Node> node, uint64_t flow_size);

        void sort_vector(std::vector<usage_struct> &vector, std::string sort_by);

        void save_vector_in_file(std::string output_file, std::vector<usage_struct> vector);

    public:

        NodesUsage(void);

        uint64_t get_total_rx(void);

        std::vector<usage_struct> getReceiversVector(void);

        void update(Ptr<Node> src, Ptr<Node> dst, uint64_t flow_size);

        void map_to_vector(std::unordered_map<std::string, usage_struct> &map, std::vector<usage_struct> &vector);

        void print(std::vector<usage_struct> vector);

        /*Saves senders and receivers in two different files. Sorts using: num flows as default */
        void save(std::string output_file, bool sorted_by_flows = true, bool sorted_by_bytes = false);

    };

    Ipv4Address GetNodeIp(std::string node_name);

    Ipv4Address GetNodeIp(Ptr<Node> node);

    Ptr<Node> GetNode(std::string name);

    std::string GetNodeName(Ptr<Node> node);

    std::string ipv4AddressToString(Ipv4Address address);

    ip_mask GetIpMask(std::string prefix);

    uint64_t BytesFromRate(DataRate dataRate, double time);

    std::vector<std::pair<double, uint64_t>> GetDistribution(std::string distributionFile);

    uint64_t GetFlowSizeFromDistribution(std::vector<std::pair<double, uint64_t>> distribution, double uniformSample);

    std::pair<uint16_t, uint16_t> GetHostPositionPair(std::string name);

    void printNow(double delay);

    void printMemUsage(double delay);

    void printNowMem(double delay);

    void saveNow(double delay, Ptr<OutputStreamWrapper> file);

    uint64_t hash_string(std::string message);


    double MeasureInterfaceLoad(Ptr<Queue<Packet>> q, double next_schedule, std::string name, DataRate linkBandwidth);

// trace sinks
    void
    CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd);

    void
    TracePcap(Ptr<PcapFileWrapper> file, Ptr<const Packet> packet);

    void
    RxDropAscii(Ptr<OutputStreamWrapper> file, Ptr<const Packet> packet);

    void
    TxDrop(std::string s, Ptr<const Packet> p);

    void PrintQueueSize(Ptr<Queue<Packet>> q);


    void ChangeLinkDropRate(NetDeviceContainer link_to_change, double drop_rate);

    void FailLink(NetDeviceContainer link_to_fail);

    void RecoverLink(NetDeviceContainer link_to_recover);


    uint64_t leftMostPowerOfTen(uint64_t number);

//gets a random element from a vector!
    template<typename T>
    T randomFromVector(std::vector<T> &vect) {
        return vect[random_variable->GetInteger(0, vect.size() - 1)];
    }

    double find_closest(std::vector<double> vect, double value);

}

#endif /* UTILS_H */

