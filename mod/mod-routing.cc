/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Arizona
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Junseok Kim <junseok@email.arizona.edu> <engr.arizona.edu/~junseok>
 */

#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/pointer.h"
#include "ns3/double.h"
#include "ns3/ipv4-static-routing.h"
#include "mod-routing.h"
#include "MyTag.h"
 #include <list>
#include <map>
#include <vector>
#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/ipv4.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
 #include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/object-vector.h"
#include "ns3/ipv4-header.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/traffic-control-layer.h"
// #include "ns3/ip"
// #include "arp-l3-protocol.h"
// #include "ipv4-l3-protocol.h"
// #include "icmpv4-l4-protocol.h"
// #include "ipv4-interface.h"
// #include "ipv4-raw-socket-impl.h"
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>
#include <vector>
NS_LOG_COMPONENT_DEFINE ("ModRouting");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ModRouting);

TypeId
ModRouting::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ModRouting")
    .SetParent<Ipv4RoutingProtocol> ()
    .AddConstructor<ModRouting> ()
    .AddAttribute ("RoutingTable", "Pointer to Routing Table.",
                   PointerValue (),
                   MakePointerAccessor (&ModRouting::SetRtable),
                   MakePointerChecker<ModRoutingTable> ())
    ;
  return tid;
}


ModRouting::ModRouting () 
{
  NS_LOG_FUNCTION_NOARGS ();
}

ModRouting::~ModRouting () 
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ptr<Ipv4Route>
ModRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, enum Socket::SocketErrno &sockerr)
{
  Ipv4Address relay = m_rtable->LookupRoute (m_address, header.GetDestination ());
  NS_LOG_FUNCTION (this << header.GetSource () << "->" << relay << "->" << header.GetDestination ());
  NS_LOG_INFO ("Relay to " << relay);
  if (m_address == relay)
    {
      NS_LOG_DEBUG ("Can't find route!!");
    }
  
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetGateway (relay);
  route->SetSource (m_address);
  route->SetDestination (header.GetDestination ());
  route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));
  
  sockerr = Socket::ERROR_NOTERROR;
  
  MyTag tagForPacket;
  int numberOfSwitches = 10;
  std::vector<uint8_t> vectorForPacket(numberOfSwitches);
  for (int i = 0; i < numberOfSwitches; ++i)
  {
    /* code */
    vectorForPacket.push_back(0x00);
  }
  tagForPacket.SetSimpleValue(vectorForPacket);
  // p->AddPacketTag(tagForPacket);
  return route;
}

bool 
ModRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev, 
                             UnicastForwardCallback ucb, MulticastForwardCallback mcb, 
                             LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (header.GetDestination ());
  MyTag tagForPacket;
  p->PeekPacketTag(tagForPacket);
  if (header.GetDestination () == m_address)
    {
      NS_LOG_DEBUG ("I'm the destination");
      lcb (p, header, m_ipv4->GetInterfaceForDevice (idev));
      return true;
    }
  else if (header.GetDestination () == m_broadcast)
    {
      NS_LOG_DEBUG ("It's broadcast");
      return true;
    }
  else if (tagForPacket.GetSimpleValueByIndex(idev->GetNode()->GetId()) == 0x00)
    {
      Ipv4Address relay = m_rtable->LookupRoute (m_address, header.GetDestination ());
      NS_LOG_FUNCTION (this << m_address << "->" << relay << "->" << header.GetDestination ());
      NS_LOG_DEBUG ("Relay to " << relay);
      if (m_address == relay)
        {
          NS_LOG_DEBUG ("Can't find a route!!");
        }
      Ptr<Ipv4Route> route = Create<Ipv4Route> ();
      route->SetGateway (relay);
      route->SetSource (header.GetSource ());
      route->SetDestination (header.GetDestination ());
      route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));
      ucb (route, p, header);
      return true;
    }
    else{

      int tagForPacketValue = tagForPacket.GetSimpleValueByIndex(idev->GetNode()->GetId());
      int maxDevices = idev->GetNode()->GetNDevices();
      int indexOfOutputDevice = (tagForPacketValue%maxDevices) +1;
      Ptr<NetDevice>outputDevice = idev->GetNode()->GetDevice(indexOfOutputDevice);
      tagForPacket.SetSimpleValueByIndex(idev->GetNode()->GetId(),(uint8_t)indexOfOutputDevice);
      while (outputDevice->IsLinkUp() == false){
        tagForPacketValue = tagForPacket.GetSimpleValueByIndex(idev->GetNode()->GetId());
        maxDevices = idev->GetNode()->GetNDevices();
        indexOfOutputDevice = (tagForPacketValue%maxDevices) +1;
        Ptr<NetDevice>outputDevice = idev->GetNode()->GetDevice(indexOfOutputDevice);
        tagForPacket.SetSimpleValueByIndex(idev->GetNode()->GetId(),(uint8_t)indexOfOutputDevice);
      }
      lcb(p,header,m_ipv4->GetInterfaceForDevice (outputDevice));
    }
  return false;
}

void 
ModRouting::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
}
void 
ModRouting::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
}
void 
ModRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION(this << interface << address << m_rtable);
  m_ifaceId = interface;
  m_address = address.GetLocal ();
  m_broadcast = address.GetBroadcast ();
}
void 
ModRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION(this << interface << address);
}
void 
ModRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION(this << ipv4);
  m_ipv4 = ipv4;
}
void
ModRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
  *stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId ();
  m_rtable->Print (stream);
}
void
ModRouting::SetRtable (Ptr<ModRoutingTable> p)
{
  NS_LOG_FUNCTION(p);
  m_rtable = p;
}

} // namespace ns3


