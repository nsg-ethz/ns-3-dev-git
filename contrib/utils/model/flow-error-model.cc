//
// Created by edgar costa molero on 04.05.18.
//

#include <cmath>
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "flow-error-model.h"

//Import headers
#include "ns3/ppp-header.h"
#include "ns3/ethernet-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/ipv4-header.h"
//#include "ns3/ipv6-header.h" FUTURE

namespace ns3 {

//
// FlowErrorModel
//

NS_OBJECT_ENSURE_REGISTERED (FlowErrorModel);

TypeId FlowErrorModel::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::FlowErrorModel")
            .SetParent<ErrorModel> ()
            .SetGroupName("Network")
            .AddConstructor<FlowErrorModel> ()
            .AddAttribute ("FlowLayer", "The error unit",
                           EnumValue (L4_LAYER),
                           MakeEnumAccessor (&FlowErrorModel::m_layer),
                           MakeEnumChecker (L3_LAYER, "L3_LAYER",
                                            L4_LAYER, "L4_LAYER"))
            .AddAttribute ("ErrorRate", "The error rate.",
                           DoubleValue (0.0),
                           MakeDoubleAccessor (&FlowErrorModel::m_rate),
                           MakeDoubleChecker<double> ())
            .AddAttribute ("RanVar", "The decision variable attached to this error model.",
                           StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1.0]"),
                           MakePointerAccessor (&FlowErrorModel::m_ranvar),
                           MakePointerChecker<RandomVariableStream> ())
    ;
    return tid;
}

FlowErrorModel::FlowErrorModel ()
{
    NS_LOG_FUNCTION (this);
}

FlowErrorModel::~FlowErrorModel ()
{
    NS_LOG_FUNCTION (this);
}

FlowErrorModel::FlowLayer
FlowErrorModel::GetLayer (void) const
{
    NS_LOG_FUNCTION (this);
    return m_layer;
}

void
FlowErrorModel::SetLayer (enum FlowLayer flow_layer)
{
    NS_LOG_FUNCTION (this << flow_layer);
    m_layer = flow_layer;
}

double
FlowErrorModel::GetRate (void) const
{
    NS_LOG_FUNCTION (this);
    return m_rate;
}

void
FlowErrorModel::SetRate (double rate)
{
    NS_LOG_FUNCTION (this << rate);
    m_rate = rate;
}

void
FlowErrorModel::SetRandomVariable (Ptr<RandomVariableStream> ranvar)
{
    NS_LOG_FUNCTION (this << ranvar);
    m_ranvar = ranvar;
}

int64_t
FlowErrorModel::AssignStreams (int64_t stream)
{
    NS_LOG_FUNCTION (this << stream);
    m_ranvar->SetStream (stream);
    return 1;
}

bool
FlowErrorModel::DoCorrupt (Ptr<Packet> p)
{
    NS_LOG_FUNCTION (this << p);
    if (!IsEnabled ())
    {
        return false;
    }

    /*Check if the flow was tagged to be failed in the past */
    uint64_t flow_id = GetHeaderHash(p);

    /* Check if flow_id is already known: */
    if (IsRed(flow_id))
    {
        return true;
    }

    if (IsGreen(flow_id))
    {
        //apply the burst error model
        return false;
    }

    /* New Flow: check if it has to be corrupted */


    return false;
}

uint64_t
FlowErrorModel::GetHeaderHash(Ptr<Packet> packet){

    Ptr<Packet> p = packet->Copy();

    /* Assumes that the packet is a ppp */
    PppHeader ppp_header;
    p->RemoveHeader(ppp_header);

    /* Assumes IPv4 header */
    Ipv4Header ip_header;
    p->RemoveHeader(ip_header);

    uint8_t ip_protocol = ip_header.GetProtocol();
    uint16_t src_port = 0;
    uint16_t dst_port = 0;

    switch (m_layer)
    {
        case L3_LAYER:
            break;
        case L4_LAYER:

            if (ip_protocol == uint8_t(17)){//udp
                UdpHeader udp_header;
                p->RemoveHeader(udp_header);
                src_port = udp_header.GetSourcePort();
                dst_port = udp_header.GetDestinationPort();
            }
            else if (ip_protocol ==  uint8_t(6)) {//tcp
                TcpHeader tcp_header;
                p->RemoveHeader(tcp_header);
                src_port = tcp_header.GetSourcePort();
                dst_port = tcp_header.GetDestinationPort();
            }
        default:
            NS_ASSERT_MSG(false, "Layer not supported");
            break;
    }

    uint8_t buf[13];
    ip_header.GetSource().Serialize(buf);
    ip_header.GetDestination().Serialize(buf+4);
    buf[8] = ip_protocol;
    buf[9] = (src_port >> 8) & 0xff;
    buf[10] = src_port & 0xff;
    buf[11] = (dst_port >> 8) & 0xff;
    buf[12] = dst_port & 0xff;

    uint64_t hash = Hash64((char*) buf, 13);
    NS_LOG_DEBUG("Flow Error Model Packet Hash" << hash);
    return hash;
}

bool
FlowErrorModel::IsGreen(uint64_t flow_id)
{
    NS_LOG_FUNCTION(this << flow_id);
    return m_greenFlows.find(flow_id) != m_greenFlows.end();
}

bool
FlowErrorModel::IsRed(uint64_t flow_id)
{
    NS_LOG_FUNCTION(this << flow_id);
    return m_redFlows.find(flow_id) != m_redFlows.end();
}

bool
FlowErrorModel::DoCorruptFlow (uint64_t flow_id)
{
    NS_LOG_FUNCTION (this << flow_id);
    bool to_corrupt = (m_ranvar->GetValue () < m_rate);
    if (to_corrupt)
    {
        m_redFlows.insert(flow_id);
    }
    else
    {
        m_greenFlows.insert(flow_id);
    }
    return to_corrupt;
}

void
FlowErrorModel::DoReset (void)
{
    NS_LOG_FUNCTION (this);
    /* re-initialize any state; no-op for now */
}

} // namespace ns3

