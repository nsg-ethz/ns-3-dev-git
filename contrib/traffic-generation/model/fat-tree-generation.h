/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef FAT_TREE_GENERATION_H
#define FAT_TREE_GENERATION_H


#include <string.h>
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include <unordered_map>
#include <vector>

namespace ns3 {



void startStride(NodeContainer hosts, std::unordered_map <std::string, std::vector<uint16_t>> hostsToPorts,
                 uint64_t flowSize, uint16_t nFlows, uint16_t offset, Ptr<OutputStreamWrapper> fctFile, Ptr<OutputStreamWrapper> counterFile,
                 Ptr<OutputStreamWrapper> flowsFile);

void startRandom(NodeContainer hosts, std::unordered_map <std::string, std::vector<uint16_t>> hostsToPorts,
                 DataRate sendingRate, uint16_t flowsPerHost, uint16_t k, Ptr<OutputStreamWrapper> fctFile, Ptr<OutputStreamWrapper> counterFile,
                 Ptr<OutputStreamWrapper> flowsFile);

void sendFromDistribution(NodeContainer hosts, std::unordered_map <std::string, std::vector<uint16_t>> hostsToPorts,
                          uint16_t k, Ptr<OutputStreamWrapper> fctFile,Ptr<OutputStreamWrapper> counterFile, Ptr<OutputStreamWrapper> flowsFile,
                          std::string distributionFile,uint32_t seed, uint32_t interArrivalFlow,
                          double intraPodProb, double interPodProb, double simulationTime,
                          double *startRecordingTime, double recordingTime, uint64_t * recordedFlowsCounter);


}

#endif /* FAT_TREE_GENERATION_H */
