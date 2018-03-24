/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "utils.h"
#include "fat-tree-utils.h"

NS_LOG_COMPONENT_DEFINE ("fat-tree-utils");


namespace ns3 {

//Assume a fat tree and the following naming h_x_y
std::pair<uint16_t, uint16_t> GetHostPositionPair(std::string name){

	std::stringstream text(name);
	std::string segment;
	std::pair<uint16_t, uint16_t> result;

	std::getline(text, segment, '_');
	std::getline(text, segment, '_');
	result.first = (uint16_t)std::stoi(segment);
	std::getline(text, segment, '_');
	result.second = (uint16_t)std::stoi(segment);

	return result;
}

//Just works for a fat tree.
void MeasureInOutLoad(std::unordered_map<std::string, NetDeviceContainer> links, std::unordered_map<std::string, double> linkToPreviousLoad,
		network_metadata metadata, double next_schedule, network_load load_data){


	uint32_t k = metadata.k;
	DataRate linkBandwidth = metadata.linkBandwidth;

	std::stringstream host_name;
	std::stringstream router_name;

	double sum_of_capacities = 0;
	double current_total_load = 0;
	double capacity_difference = 0;
	int interfaces_count = 0;

	for (uint32_t pod = 0; pod < k; pod++){
		for (uint32_t edge_i = 0; edge_i < k/2; edge_i++){
			for (uint32_t host_i = 0; host_i < k/2; host_i++){

				uint32_t real_host_i = host_i + (edge_i * k/2);

				host_name << "h_" << pod << "_" << real_host_i;
				router_name << "r_" << pod << "_e" << edge_i;

				NetDeviceContainer interface = links[host_name.str() + "->" + router_name.str()];

				//Get Device queues TODO: if we add RED queues.... this will not work....
//
//			  PointerValue p2p_queue;
//			  interface.Get(0)->GetAttribute("TxQueue", p2p_queue);
//			  Ptr<Queue<Packet>> queue_rx = p2p_queue.Get<Queue<Packet>>();
//
//			  PointerValue p2p_queue2;
//			  interface.Get(1)->GetAttribute("TxQueue", p2p_queue2);
//			  Ptr<Queue<Packet>> queue_tx = p2p_queue2.Get<Queue<Packet>>();

				//ONLY THE FAT TREE WILL FIND THE INTERFACES
				Ptr<Queue<Packet>> queue_rx = DynamicCast<PointToPointNetDevice>(interface.Get(0))->GetQueue();
				Ptr<Queue<Packet>> queue_tx = DynamicCast<PointToPointNetDevice>(interface.Get(1))->GetQueue();

				//RX Link
				current_total_load =MeasureInterfaceLoad(queue_rx,
						next_schedule, host_name.str() + "_rx", linkBandwidth);

				capacity_difference= current_total_load - linkToPreviousLoad[host_name.str() + "->" + router_name.str() + "_rx"];
				linkToPreviousLoad[host_name.str() + "->" + router_name.str() + "_rx"] = current_total_load;

				//Ignore counter warps
				if (capacity_difference < 0){
					capacity_difference = current_total_load;
				}
				sum_of_capacities += capacity_difference;


				//TX Link
				current_total_load =MeasureInterfaceLoad(queue_tx,
						next_schedule, host_name.str() + "_tx", linkBandwidth);

				capacity_difference= current_total_load - linkToPreviousLoad[host_name.str() + "->" + router_name.str() + "_tx"];
				linkToPreviousLoad[host_name.str() + "->" + router_name.str() + "_tx"] = current_total_load;

				//wrap
				if (capacity_difference < 0){
					capacity_difference = current_total_load;
				}
				sum_of_capacities += capacity_difference;

				interfaces_count +=2;

				host_name.str("");
				router_name.str("");
			}
		}
	}

	//compute average network usage
	double average_usage = sum_of_capacities / interfaces_count;

	NS_LOG_UNCOND("Netowork Load: " <<  " " << average_usage);

	if (average_usage >= load_data.stopThreshold)
	{
		double now = Simulator::Now().GetSeconds();
		*(load_data.startTime) = now;
		NS_LOG_DEBUG("Above threshold: Account Flows From: " << now);
		return;
	}
	else
	{
		//Reschedule the function until it reaches a certain load
		Simulator::Schedule(Seconds(next_schedule), &MeasureInOutLoad, links,linkToPreviousLoad, metadata, next_schedule, load_data);

	}
}

void
allocateNodesFatTree(int k){


//  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel>();
//  GetNode(node_name)->AggregateObject(loc);
//	loc->SetPosition(Vector(2,5,0));

	//Locate edge and agg
	Vector host_pos(0,30,0);
	Vector edge_pos(0,25,0);
	Vector agg_pos (0,19.5,0);

	for (int pod= 0; pod < k ; pod++){

		for (int pod_router = 0; pod_router < k/2; pod_router++){

			//Allocate edges
			Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel>();

  		std::stringstream router_name;
  		router_name << "r_" << pod << "_e" << pod_router;

  		//Update edge pos
  		edge_pos.x = (edge_pos.x + 3);

  		GetNode(router_name.str())->AggregateObject(loc);
  		loc->SetPosition(edge_pos);
  		NS_LOG_DEBUG("Pos: " << router_name.str() << " " << edge_pos);


//  		Allocate hosts
  		for (int host_i= 0; host_i < k/2 ; host_i++){
  			Ptr<ConstantPositionMobilityModel> loc1 = CreateObject<ConstantPositionMobilityModel>();

    		std::stringstream host_name;
    		host_name << "h_" << pod << "_" << (host_i + (k/2*pod_router));

    		double hosts_distance = 1.6;
    		host_pos.x = edge_pos.x -(hosts_distance/2) + (host_i * (hosts_distance/((k/2)-1)));

    		GetNode(host_name.str())->AggregateObject(loc1);
    		loc1->SetPosition(host_pos);
    		NS_LOG_DEBUG("Pos: " << host_name.str() << " " << host_pos);

  		}

			//Allocate aggregations
			Ptr<ConstantPositionMobilityModel> loc2 = CreateObject<ConstantPositionMobilityModel>();

  		router_name.str("");
  		router_name << "r_" << pod << "_a" << pod_router;

  		//Update edge pos
  		agg_pos.x = (agg_pos.x + 3);

  		GetNode(router_name.str())->AggregateObject(loc2);
  		loc2->SetPosition(agg_pos);
  		NS_LOG_DEBUG("Pos: " << router_name.str() << " " << agg_pos);

		}
		edge_pos.x = edge_pos.x + 3;
		agg_pos.x = agg_pos.x + 3;
	}

	//Allocate Core routers
	int num_cores = (k/2) * (k/2);
	int offset = 6;
	double distance = (edge_pos.x -offset*2);
	double step = distance/(num_cores-1);
	Vector core_pos_i(offset,10,0);
	Vector core_pos(offset,10,0);


	for (int router_i = 0; router_i < num_cores; router_i++){

		Ptr<ConstantPositionMobilityModel> loc3 = CreateObject<ConstantPositionMobilityModel>();

		std::stringstream router_name;
		router_name << "r_c" << router_i;

		//Update edge pos
		core_pos.x =core_pos_i.x + (router_i * step);

		GetNode(router_name.str())->AggregateObject(loc3);
		loc3->SetPosition(core_pos);
		NS_LOG_DEBUG("Pos: " << router_name.str() << " " << core_pos);


	}

}

}
