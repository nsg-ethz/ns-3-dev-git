/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef FAT_TREE_UTILS_H
#define FAT_TREE_UTILS_H

#include <string.h>
#include <string>

#include "utils.h"

namespace ns3 {

struct network_load{
	double stopThreshold;
	double *startTime;
};

struct network_metadata{
	uint32_t k;
	DataRate linkBandwidth;
};

std::pair<uint16_t, uint16_t> GetHostPositionPair(std::string name);

void MeasureInOutLoad(std::unordered_map<std::string, NetDeviceContainer> links, std::unordered_map<std::string, double> linkToPreviousLoad,
		network_metadata metadata, double next_schedule, network_load load_data);


void allocateNodesFatTree(int k);

}

#endif /* FAT_TREEUTILS_H */

