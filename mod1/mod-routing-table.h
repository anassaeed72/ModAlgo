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

#ifndef MOD_ROUTING_TABLE_H
#define MOD_ROUTING_TABLE_H

#include "ns3/node.h"
#include "ns3/ipv4-route.h"
#include "ns3/output-stream-wrapper.h"
#include <list>
#include <vector>

namespace ns3 {

class ModRoutingTable : public Object
{
public:

  ModRoutingTable ();
  virtual ~ModRoutingTable ();

  static TypeId GetTypeId ();
  void AddRoute (Ipv4Address srcAddr, Ipv4Address relayAddr, Ipv4Address dstAddr);
  void AddNode (Ptr<Node> node, Ipv4Address addr);
  Ipv4Address LookupRoute (Ipv4Address srcAddr, Ipv4Address dstAddr);
  void UpdateRoute (double txRange);
  double GetDistance (Ipv4Address srcAddr, Ipv4Address dstAddr);

  void Print (Ptr<OutputStreamWrapper> stream) const;

private:
  typedef struct
    {
      Ipv4Address srcAddr;
      Ipv4Address relayAddr;
      Ipv4Address dstAddr;
    }
  ModtableEntry;
  
  typedef struct
    {
      Ptr<Node> node;
      Ipv4Address addr;
    } 
  ModNodeEntry;
  
  double DistFromTable (uint16_t i, uint16_t j);
  
  std::list<ModtableEntry> m_modtable;
  std::vector<ModNodeEntry> m_nodeTable;
  
  uint16_t* m_modNext;
  double*   m_modDist;
  
  double    m_txRange;
};

}

#endif // SHORTEST_PATH_ROUTING_TABLE_H
