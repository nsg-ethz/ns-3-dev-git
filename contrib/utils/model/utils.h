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
#include <ctime>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3 {

/* ... */
    extern Ptr<UniformRandomVariable> random_variable;

    struct IpMask {
        std::string ip;
        std::string mask;
    };


    Ipv4Address GetNodeIp(std::string node_name);

    Ipv4Address GetNodeIp(Ptr<Node> node);

    Ptr<Node> GetNode(std::string name);

    std::string GetNodeName(Ptr<Node> node);

    std::string Ipv4AddressToString(Ipv4Address address);

    IpMask GetIpMask(std::string prefix);

    uint64_t BytesFromRate(DataRate dataRate, double time);

    std::vector<std::pair<double, uint64_t>> GetDistribution(std::string distributionFile);

    uint64_t GetFlowSizeFromDistribution(std::vector<std::pair<double, uint64_t>> distribution, double uniformSample);

    std::pair<uint16_t, uint16_t> GetHostPositionPair(std::string name);

    void PrintNow(double delay);

    void PrintMemUsage(double delay);

    void PrintNowMem(double delay, std::clock_t starting_time);

    void SaveNow(double delay, Ptr<OutputStreamWrapper> file);

    uint64_t HashString(std::string message);

    double MeasureInterfaceLoad(Ptr<Queue<Packet>> q, double next_schedule, std::string name, DataRate linkBandwidth);


    void PrintQueueSize(Ptr<Queue<Packet>> q);

    void ChangeLinkDropRate(NetDeviceContainer link_to_change, double drop_rate);

    void FailLink(NetDeviceContainer link_to_fail);

    void RecoverLink(NetDeviceContainer link_to_recover);

    uint64_t LeftMostPowerOfTen(uint64_t number);

//gets a random element from a vector!
    template<typename T>
    T RandomFromVector(std::vector<T> &vect) {
        return vect[random_variable->GetInteger(0, vect.size() - 1)];
    }

    double FindClosest(std::vector<double> vect, double value);

    //Ptr<Queue<Packet>> GetPointToPointNetDeviceQueue(NetDevice netDevice);

    }

#endif /* UTILS_H */

