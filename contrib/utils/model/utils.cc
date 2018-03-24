/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "utils.h"

NS_LOG_COMPONENT_DEFINE ("utils");


namespace ns3 {

/* ... */

Ptr<UniformRandomVariable> random_variable = CreateObject<UniformRandomVariable> ();

/*
 * Nodes Usage Object
 */

void NodesUsage::update_rx_receivers(Ptr<Node> node, uint64_t flow_size){

	std::string node_name = GetNodeName(node);

	if (this->receivers.count(node_name) > 0){
		this->receivers[node_name].rx_bytes += flow_size;
		this->receivers[node_name].rx_flows += 1;
	}
	else{
		this->receivers[node_name].rx_bytes = flow_size;
		this->receivers[node_name].rx_flows = 1;
		this->receivers[node_name].ipAddr = ipv4AddressToString(GetNodeIp(node));
		this->receivers[node_name].name = node_name;
	}
}

void NodesUsage::update_tx_senders(Ptr<Node> node, uint64_t flow_size){

	std::string node_name = GetNodeName(node);

	if (this->senders.count(node_name) > 0){
		this->senders[node_name].tx_bytes += flow_size;
		this->senders[node_name].tx_flows += 1;
	}
	else{
		this->senders[node_name].tx_bytes = flow_size;
		this->senders[node_name].tx_flows = 1;
		this->senders[node_name].ipAddr = ipv4AddressToString(GetNodeIp(node));
		this->senders[node_name].name = node_name;
	}
}

	std::vector<usage_struct> NodesUsage::getReceiversVector(void){
		return receivers_vector;
	}


	void NodesUsage::map_to_vector(std::unordered_map<std::string, usage_struct> &map, std::vector<usage_struct> &vector){

        std::unordered_map<std::string, usage_struct>::iterator it = map.begin();
        while (it != map.end()){
            vector.push_back(it->second);
            it++;
	}
}

void NodesUsage::sort_vector(std::vector<usage_struct> &vector, std::string sort_by){

	if (sort_by == "tx_flows")
	{
		std::sort(vector.begin(), vector.end(), less_tx_flows());
	}
	else if (sort_by == "rx_flows")
	{
		std::sort(vector.begin(), vector.end(), less_rx_flows());
	}
	else if (sort_by == "tx_bytes")
	{
		std::sort(vector.begin(), vector.end(), less_tx_bytes());
	}
	else if (sort_by == "rx_bytes")
	{
		std::sort(vector.begin(), vector.end(), less_rx_bytes());
	}
}


NodesUsage::NodesUsage(void){
 /*nothing*/
}

void NodesUsage::update(Ptr<Node> src, Ptr<Node> dst, uint64_t flow_size){
	update_tx_senders(src, flow_size);
	update_rx_receivers(dst, flow_size);
}


void NodesUsage::save_vector_in_file(std::string output_file, std::vector<usage_struct> vector){

	std::ofstream out_file(output_file);
	for (const auto &e : vector){
		out_file << e.name << " ";
		out_file << e.ipAddr << " ";
		out_file << e.tx_flows << " ";
		out_file << e.rx_flows << " ";
		out_file << e.tx_bytes << " ";
		out_file << e.rx_bytes << "\n";
	}
}

void NodesUsage::print(std::vector<usage_struct> vector){

	for (const auto &e : vector){
		std::cout << e.name << " ";
		std::cout << e.ipAddr << " ";
		std::cout << e.tx_flows << " ";
		std::cout << e.rx_flows << " ";
		std::cout << e.tx_bytes << " ";
		std::cout << e.rx_bytes << "\n";
	}
	std::cout << "\n";
}

    uint64_t NodesUsage::get_total_rx(void){
        int64_t sum = 0;
        for (const auto &e: receivers_vector){
            sum += e.rx_bytes;
        }
        return sum;
    }

void NodesUsage::save(std::string output_file, bool sorted_by_flows, bool sorted_by_bytes){

	map_to_vector(receivers, receivers_vector);
	map_to_vector(senders, senders_vector);

	if (sorted_by_bytes | sorted_by_flows){
		if (sorted_by_bytes){
			sort_vector(receivers_vector, "rx_bytes");
			sort_vector(senders_vector, "tx_bytes");
		}
		else if (sorted_by_flows){
			sort_vector(receivers_vector, "rx_flows");
			sort_vector(senders_vector, "tx_flows");
		}
	}

	save_vector_in_file(output_file + "-tx.txt", senders_vector);
	save_vector_in_file(output_file + "-rx.txt", receivers_vector);
}

/*
 *
 */

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

uint64_t hash_string(std::string message){
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

 std::string ipv4AddressToString(Ipv4Address address){

	 std::stringstream ip;
	 ip << address;
	 return ip.str();

//  	return ipToString((address.m_address & 0xff000000) >> 24, (address.m_address & 0x00ff0000) >> 16, (address.m_address & 0x0000ff00) >> 8, address.m_address & 0x000000ff);
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

//TRACE SINKS
  void
  CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
  {
  //  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
    *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
  }

   void
  TracePcap (Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
  {
    file->Write (Simulator::Now (), packet);
  }

   void
  RxDropAscii (Ptr<OutputStreamWrapper> file, Ptr<const Packet> packet)
  {
  //	Ptr<PcapFileWrapper> file OLD VERSION
    //NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());

  	Ptr<Packet> p = packet->Copy();

  	PppHeader ppp_header;
  	p->RemoveHeader(ppp_header);

  	Ipv4Header ip_header;
  	p->RemoveHeader(ip_header);


    std::ostringstream oss;
    oss << Simulator::Now().GetSeconds() << " "
    		<< ip_header.GetSource() << " "
        << ip_header.GetDestination() << " "
        << int(ip_header.GetProtocol()) << " ";

  	if (ip_header.GetProtocol() == uint8_t(17)){ //udp
      UdpHeader udpHeader;
      p->PeekHeader(udpHeader);
      oss << int(udpHeader.GetSourcePort()) << " "
          << int(udpHeader.GetDestinationPort()) << " ";

  	}
  	else if (ip_header.GetProtocol() == uint8_t(6)) {//tcp
      TcpHeader tcpHeader;
      p->PeekHeader(tcpHeader);
      oss << int(tcpHeader.GetSourcePort()) << " "
          << int(tcpHeader.GetDestinationPort()) << " ";
  	}

  	oss << packet->GetSize() << "\n";
  	*(file->GetStream()) << oss.str();
    (file->GetStream())->flush();

  //  file->Write (Simulator::Now (), p);
  }

  void
  TxDrop (std::string s, Ptr<const Packet> p){
  	static int counter = 0;
  	NS_LOG_UNCOND (counter++ << " " << s << " at " << Simulator::Now ().GetSeconds ()) ;
  }


void printNow(double delay){
	NS_LOG_UNCOND("Simulation Time: " << Simulator::Now().GetSeconds());
	Simulator::Schedule (Seconds(delay), &printNow, delay);
}

void printMemUsage(double delay){
	NS_LOG_UNCOND("Memory Used: " << GetMemoryUsed());
	Simulator::Schedule (Seconds(delay), &printMemUsage, delay);
}

void printNowMem(double delay){

	std::cout  <<"Simulation Time: " << Simulator::Now().GetSeconds()
									  << "\t Memory Used: " << GetMemoryUsed() <<"\n";
	Simulator::Schedule (Seconds(delay), &printNowMem, delay);
}

void saveNow(double delay, Ptr<OutputStreamWrapper> file){

	*(file->GetStream()) << Simulator::Now().GetSeconds() << "\n";
	(file->GetStream())->flush();

	Simulator::Schedule (Seconds(delay), &saveNow, delay, file);
}

void PrintQueueSize(Ptr<Queue<Packet>> q){
	uint32_t size = q->GetNPackets();
	if (size > 0){
		NS_LOG_UNCOND(Simulator::Now().GetSeconds() << " " <<  size);
	}
	Simulator::Schedule(Seconds(0.001), &PrintQueueSize, q);
}

void ChangeLinkDropRate(NetDeviceContainer link_to_change, double drop_rate){
	Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
	em->SetAttribute ("ErrorRate", DoubleValue (drop_rate));
	em->SetAttribute ("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));

	link_to_change.Get(0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
	link_to_change.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
}

void FailLink(NetDeviceContainer link_to_fail){

    NS_LOG_DEBUG("Failing prefix ");
	ChangeLinkDropRate(link_to_fail, 1);
}

void RecoverLink(NetDeviceContainer link_to_recover){
	ChangeLinkDropRate(link_to_recover, 0);
}

uint64_t leftMostPowerOfTen(uint64_t number){
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

double find_closest(std::vector<double> vect, double value){

	auto it = std::lower_bound(vect.begin(), vect.end(), value);

	if (it == vect.begin()){
		return vect[0];
	}
	else if (it == vect.end()){
		return vect.back();
	}
	else{
		NS_LOG_DEBUG(*(it-1) << " " << std::abs(value - *(it-1)) << " " << *it << " " << std::abs(value - *(it)) << "\n");
		if (std::abs(value - *(it-1)) <= std::abs( value - *it)){
			return *(it-1);
		}
		else{
			return *it;
		}
	}
}

}
