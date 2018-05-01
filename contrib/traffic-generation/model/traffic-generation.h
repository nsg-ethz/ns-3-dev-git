/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef TRAFFIC_GENERATION_H
#define TRAFFIC_GENERATION_H

#include <string.h>
#include <string>
#include "ns3/network-module.h"
#include <unordered_map>
#include <vector>

namespace ns3 {

void
InstallSink(Ptr<Node> node, uint16_t sinkPort, uint32_t duration, std::string protocol);

std::unordered_map <std::string, std::vector<uint16_t>>
InstallSinks(NodeContainer hosts, uint16_t sinksPerHost, uint32_t duration, std::string protocol);


Ptr<Socket> InstallSimpleSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t sinkPort, DataRate dataRate, uint32_t numPackets, std::string protocol);

void InstallBulkSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, uint64_t size, double startTime,
					 Ptr<OutputStreamWrapper> fctFile = NULL, Ptr<OutputStreamWrapper> counterFile = NULL,
					 Ptr<OutputStreamWrapper> flowsFile = NULL, uint64_t flowId = 0,
					 uint64_t *recordedFlowsCounter = NULL, double *startRecordingTime = NULL,
					 double recordingTime = -1);

void InstallNormalBulkSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, uint64_t size, double startTime);
void InstallOnOffSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, DataRate dataRate, uint32_t packet_size, uint64_t max_size, double startTime);
void InstallRateSend(Ptr<Node> srcHost, Ptr<Node> dstHost, uint16_t dport, uint32_t n_packets, uint64_t max_size, double duration, double startTime);
void SendBindTest(Ptr<Node> src,NodeContainer receivers, std::unordered_map<std::string, std::vector<uint16_t>> hostsToPorts, uint32_t flows);

}

#endif /* TRAFFIC_GENERATION_H */

