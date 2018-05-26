/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/csma-module.h>
#include "utils.h"
#include "flow-error-model.h"

NS_LOG_COMPONENT_DEFINE ("utils");

namespace ns3 {

/* ... */

Ptr<UniformRandomVariable> random_variable = CreateObject<UniformRandomVariable> ();


long double GetMemoryUsed(){

	struct sysinfo memInfo;

	sysinfo (&memInfo);
	long long totalPhysMem = memInfo.totalram;
	totalPhysMem *= memInfo.mem_unit;

	long long physMemUsed = memInfo.totalram - memInfo.freeram;
	//Multiply in next statement to avoid int overflow on right hand side...
	physMemUsed *= memInfo.mem_unit;

	long double percentageUsed = (long double)physMemUsed / totalPhysMem;

	return percentageUsed;
}


Ipv4Address
GetNodeIp(std::string node_name)
{

	Ptr<Node> node = GetNode(node_name);

  Ptr<Ipv4> ip = node->GetObject<Ipv4> ();

  ObjectVectorValue interfaces;
  NS_ASSERT(ip !=0);
  ip->GetAttribute("InterfaceList", interfaces);
  for(ObjectVectorValue::Iterator j = interfaces.Begin(); j != interfaces.End (); j ++)
  {
    Ptr<Ipv4Interface> ipIface = (*j).second->GetObject<Ipv4Interface> ();
    NS_ASSERT(ipIface != 0);
    Ptr<NetDevice> device = ipIface->GetDevice();
    NS_ASSERT(device != 0);
    Ipv4Address ipAddr = ipIface->GetAddress (0).GetLocal();

    // ignore localhost interface...
    if (ipAddr == Ipv4Address("127.0.0.1")) {
      continue;
    }
    else {
      return ipAddr;
    }
  }

  return Ipv4Address("127.0.0.1");
}

//* Returns the amount of bytes needed to send at dataRate during time seconds.
uint64_t BytesFromRate(DataRate dataRate, double time){

		double bytes = ((double)dataRate.GetBitRate()/8) * time;
		//NS_LOG_DEBUG("Bytes to send: " << bytes);
		return bytes;
}

uint64_t HashString(std::string message){
  Hasher hasher;
  hasher.clear();
  return hasher.GetHash64(message);

}

std::vector< std::pair<double,uint64_t>> GetDistribution(std::string distributionFile) {

  std::vector< std::pair<double,uint64_t>> cumulativeDistribution;
  std::ifstream infile(distributionFile);

  NS_ASSERT_MSG(infile, "Please provide a valid file for reading the flow size distribution!");
  double cumulativeProbability;
  double size;
  while (infile >> size >> cumulativeProbability)
  {
    cumulativeDistribution.push_back(std::make_pair(cumulativeProbability, int(size)));
  }

  return cumulativeDistribution;
}

//*
//Get size from cumulative traffic distribution
//

uint64_t GetFlowSizeFromDistribution(std::vector< std::pair<double,uint64_t>> distribution, double uniformSample){

  NS_ASSERT_MSG(uniformSample <= 1.0, "Provided sampled number is bigger than 1.0!");

  uint64_t size =  50; //at least 1 packet

  uint64_t previous_size = 0;
  double previous_prob = 0.0;

  for (uint32_t i= 0; i < distribution.size(); i++){
//  	NS_LOG_UNCOND(uniformSample << " " << distribution[i].first);

  	if (uniformSample <= distribution[i].first){
  		//compute the proportional size depending on the position
  		if (i > 0){
  			previous_size = distribution[i-1].second;
  			previous_prob = distribution[i-1].first;
  		}
  		double relative_distance = (uniformSample - previous_prob)/(distribution[i].first - previous_prob);
//  		NS_LOG_UNCOND(relative_distance << " " << uniformSample << " " << previous_prob << " " << distribution[i].first << " " << previous_size);
  		size = previous_size + (relative_distance * (distribution[i].second - previous_size));

  		break;
  	}
  }

  //avoid setting a size of 0
  //set packet size to 50 at least..
  if (size == 0){
  	size = 50;
  }
  return size;
}

Ipv4Address
GetNodeIp(Ptr<Node> node)
{

  Ptr<Ipv4> ip = node->GetObject<Ipv4> ();

  ObjectVectorValue interfaces;
  NS_ASSERT(ip !=0);
  ip->GetAttribute("InterfaceList", interfaces);
  for(ObjectVectorValue::Iterator j = interfaces.Begin(); j != interfaces.End(); j ++)
  {
    Ptr<Ipv4Interface> ipIface = (*j).second->GetObject<Ipv4Interface> ();
    NS_ASSERT(ipIface != 0);
    Ptr<NetDevice> device = ipIface->GetDevice();
    NS_ASSERT(device != 0);
    Ipv4Address ipAddr = ipIface->GetAddress (0).GetLocal();

    // ignore localhost interface...
    if (ipAddr == Ipv4Address("127.0.0.1")) {
      continue;
    }
    else {
      return ipAddr;
    }
  }

  return Ipv4Address("127.0.0.1");
}

 std::string Ipv4AddressToString(Ipv4Address address){

	 std::stringstream ip;
	 ip << address;
	 return ip.str();

//  	return ipToString((address.m_address & 0xff000000) >> 24, (address.m_address & 0x00ff0000) >> 16, (address.m_address & 0x0000ff00) >> 8, address.m_address & 0x000000ff);
 }

IpMask GetIpMask(std::string prefix){

	std::string buff("");
	IpMask address;
	for (auto chr: prefix){
		if (chr == '/'){
			address.ip = buff;
			buff = "";
		}
		else{
			buff +=chr;
		}
	}

	address.mask = "/" + buff;
	return address;
}


//Returns node if added to the name system , 0 if it does not exist
Ptr<Node> GetNode(std::string name){
	return Names::Find<Node>(name);
}

std::string GetNodeName(Ptr<Node> node){
	return Names::FindName(node);
}


double MeasureInterfaceLoad(Ptr<Queue<Packet>> q,  double next_schedule, std::string name, DataRate linkBandwidth){

	uint32_t current_counter = q->GetTotalReceivedBytes();
	double total_load = double(current_counter)/BytesFromRate(DataRate(linkBandwidth), next_schedule);
	return total_load;
}


void PrintNow(double delay){
	NS_LOG_UNCOND("Simulation Time: " << Simulator::Now().GetSeconds());
	Simulator::Schedule (Seconds(delay), &PrintNow, delay);
}

void PrintMemUsage(double delay){
	NS_LOG_UNCOND("Memory Used: " << GetMemoryUsed());
	Simulator::Schedule (Seconds(delay), &PrintMemUsage, delay);
}

void PrintNowMem(double delay, std::clock_t starting_time){

	std::cout  <<"Simulation Time: " << Simulator::Now().GetSeconds() << "(" << (float(clock() - starting_time) / CLOCKS_PER_SEC) << ")"
									  << "\t Memory Used: " << GetMemoryUsed() <<"\n";
	Simulator::Schedule (Seconds(delay), &PrintNowMem, delay, starting_time);
}

void PrintNowMemStop(double delay, std::clock_t starting_time, double stop_after){

	std::cout  <<"Simulation Time: " << Simulator::Now().GetSeconds() << "(" << (float(clock() - starting_time) / CLOCKS_PER_SEC) << ")"
						 << "\t Memory Used: " << GetMemoryUsed() <<"\n";

	if ((float(clock() - starting_time) / CLOCKS_PER_SEC) > stop_after){
		std::cout << "Forcing Simulation to Stop Since it exceeded the running time of " << stop_after << " Seconds\n";
		Simulator::Stop(Seconds(0));
	}
	else {
		Simulator::Schedule(Seconds(delay), &PrintNowMemStop, delay, starting_time, stop_after);
	}
}

void SaveNow(double delay, Ptr<OutputStreamWrapper> file){

	*(file->GetStream()) << Simulator::Now().GetSeconds() << "\n";
	(file->GetStream())->flush();

	Simulator::Schedule (Seconds(delay), &SaveNow, delay, file);
}

void PrintQueueSize(Ptr<Queue<Packet>> q){
	uint32_t size = q->GetNPackets();
	if (size > 0){
		NS_LOG_UNCOND(Simulator::Now().GetSeconds() << " " <<  size);
	}
	Simulator::Schedule(Seconds(0.001), &PrintQueueSize, q);
}

Ptr<Queue<Packet>> GetPointToPointNetDeviceQueue(PointToPointNetDevice netDevice){

	//DynamicCast<PointToPointNetDevice>(netDevice)
	Ptr<PointToPointNetDevice> p2pNetDevice = netDevice.GetObject<PointToPointNetDevice>();
	//alternative
	//PointerValue tmp;
	//p2pNetDevice->GetAttribute("TxQueue", tmp);
	// return tmp.GetObject<Queue<Packet>>();
	return p2pNetDevice->GetQueue();

}

/* Link failure utils  */

/*Completely fails link and dropped packets are traced by TX traces*/

void LinkUp(NetDeviceContainer link)
{
	NS_LOG_FUNCTION_NOARGS();
	DynamicCast<PointToPointNetDevice>(link.Get(0))->LinkUp();
	DynamicCast<PointToPointNetDevice>(link.Get(1))->LinkUp();
}

void LinkDown(NetDeviceContainer link)
{
	NS_LOG_FUNCTION_NOARGS();
	DynamicCast<PointToPointNetDevice>(link.Get(0))->LinkDown();
	DynamicCast<PointToPointNetDevice>(link.Get(1))->LinkDown();
}

void FailLink(NetDeviceContainer link_to_fail)
{

	LinkDown(link_to_fail);
}

void RecoverLink(NetDeviceContainer link_to_recover)
{
	LinkUp(link_to_recover);
}

void SetUniformDropRate(NetDeviceContainer link_to_change, double drop_rate)
{
	Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
	em->SetAttribute ("ErrorRate", DoubleValue (drop_rate));
	em->SetAttribute ("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));
	link_to_change.Get(0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
	link_to_change.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
}

void SetFlowErrorModel(NetDeviceContainer link)
{
  Ptr<FlowErrorModel> em = CreateObject<FlowErrorModel>();
  em->Disable();
  Ptr<FlowErrorModel> em1 = CreateObject<FlowErrorModel>();
  em->Disable();
  link.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  link.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em1));
  //Alternative way of setting
  // DynamicCast<PointToPointNetDevice>(link2.Get (1))->SetReceiveErrorModel(em);
}

void ClearFlowErrorModel(NetDeviceContainer link)
{
	ChangeFlowErrorDropRate(link, 0);
	SetFlowErrorNormalDropRate(link, 0);
	PointerValue emp;
	link.Get(1)->GetAttribute("ReceiveErrorModel", emp);
	Ptr<FlowErrorModel> em = emp.Get<FlowErrorModel>();
	em->Disable();
}

void SetFlowErrorNormalDropRate(NetDeviceContainer link, double drop_rate)
{
	PointerValue emp;
	link.Get(0)->GetAttribute("ReceiveErrorModel", emp);
	Ptr<FlowErrorModel> em = emp.Get<FlowErrorModel>();
	em->SetNormalErrorModelAttribute("ErrorRate", DoubleValue(drop_rate));
	em->Enable();
}

void SetFlowErrorNormalBurstSize(NetDeviceContainer link, uint16_t min, uint16_t max)
{
	PointerValue emp;
	link.Get(0)->GetAttribute("ReceiveErrorModel", emp);
	Ptr<FlowErrorModel> em = emp.Get<FlowErrorModel>();
	Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
	rand->SetAttribute("Min", DoubleValue(min));
	rand->SetAttribute("Max", DoubleValue(max));
	em->SetNormalErrorModelAttribute("BurstSize", PointerValue(rand));
}

void ChangeFlowErrorDropRate(NetDeviceContainer link, double drop_rate)
{
	PointerValue emp;
	link.Get(1)->GetAttribute("ReceiveErrorModel", emp);
	Ptr<FlowErrorModel> em = emp.Get<FlowErrorModel>();
	em->SetAttribute("FlowErrorRate", DoubleValue(drop_rate));
	em->Reset();
	em->Enable();
}

void SetFlowErrorModelFromFeatures(NetDeviceContainer link, double flow_drop_rate, double normal_drop_rate,
																	 uint16_t minBurst, uint16_t maxBurst)
{
	Ptr<FlowErrorModel> em = CreateObject<FlowErrorModel>();
	//only for one direction
	em->SetAttribute("FlowErrorRate", DoubleValue(flow_drop_rate));
	Ptr<FlowErrorModel> em1 = CreateObject<FlowErrorModel>();

	if (flow_drop_rate == 0 and normal_drop_rate == 0)
	{
		em->Disable();
		em1->Disable();
	}

	link.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
	link.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em1));


	//uniform loss
	if (minBurst == 1 and maxBurst == 1)
	{
		Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
		rem->SetAttribute ("ErrorRate", DoubleValue (normal_drop_rate));
		rem->SetAttribute ("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));

		//em->SetNormalErrorModel(rem);
		em1->SetNormalErrorModel(rem);
	}
	//default normal model is a Burst
	else if (minBurst >= 1 and maxBurst > 1){
		//em->SetNormalErrorModelAttribute("ErrorRate", DoubleValue(normal_drop_rate));
		em1->SetNormalErrorModelAttribute("ErrorRate", DoubleValue(normal_drop_rate));

		Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
		rand->SetAttribute("Min", DoubleValue(minBurst));
		rand->SetAttribute("Max", DoubleValue(maxBurst));
		//em->SetNormalErrorModelAttribute("BurstSize", PointerValue(rand));
		em1->SetNormalErrorModelAttribute("BurstSize", PointerValue(rand));
	}
}

/* Misc */

uint64_t LeftMostPowerOfTen(uint64_t number){
	uint64_t leftMost = 0;
	uint64_t power_of_10 = 0;

	//Handle an exception
	if (number == 0){
		return 0;
	}

	while(number != 0)
	{
		leftMost = number;
		power_of_10++;
		number /=10;
	}
	return leftMost * std::pow(10, power_of_10 -1);
}

double FindClosest(std::vector<double> vect, double value){

	auto it = std::lower_bound(vect.begin(), vect.end(), value);

	if (it == vect.begin()){
		return vect[0];
	}
	else if (it == vect.end()){
		return vect.back();
	}
	else{
		//NS_LOG_DEBUG(*(it-1) << " " << std::abs(value - *(it-1)) << " " << *it << " " << std::abs(value - *(it)) << "\n");
		if (std::abs(value - *(it-1)) <= std::abs( value - *it)){
			return *(it-1);
		}
		else{
			return *it;
		}
	}
}


bool file_exists (const std::string& name){
	std::ifstream f(name.c_str());
	return f.good();
}


}
