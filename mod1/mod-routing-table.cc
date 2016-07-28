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

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "mod-routing-table.h"
#include "ns3/mobility-model.h"
#include <vector>
#include <boost/lexical_cast.hpp>

using namespace std;

NS_LOG_COMPONENT_DEFINE ("ModRoutingTable");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ModRoutingTable);

TypeId
ModRoutingTable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ModRoutingTable")
    .SetParent<Object> ()
    .AddConstructor<ModRoutingTable> ()
    ;
  return tid;
}

ModRoutingTable::ModRoutingTable ()
{
  m_modNext = 0;
  m_modDist = 0;
  m_txRange = 0;
}
ModRoutingTable::~ModRoutingTable ()
{}

void 
ModRoutingTable::AddRoute (Ipv4Address srcAddr, Ipv4Address relayAddr, Ipv4Address dstAddr)
{
  NS_LOG_FUNCTION (srcAddr << relayAddr << dstAddr);
  ModtableEntry se;
  se.srcAddr = srcAddr;
  se.relayAddr = relayAddr;
  se.dstAddr = dstAddr;
  
  m_modtable.push_back (se);
}

void
ModRoutingTable::AddNode (Ptr<Node> node, Ipv4Address addr)
{
  NS_LOG_FUNCTION ("node: " << node << " addr: " << addr);
  ModNodeEntry sn;
  sn.node = node;
  sn.addr = addr;
  m_nodeTable.push_back (sn);
}

Ipv4Address
ModRoutingTable::LookupRoute (Ipv4Address srcAddr, Ipv4Address dstAddr)
{
    uint16_t i = 0, j = 0;
    Ipv4Address relay;
    std::vector<ModNodeEntry>::iterator it = m_nodeTable.begin ();
    
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == srcAddr)
          {
            break;
          }
        i++;
      }
    it = m_nodeTable.begin ();
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == dstAddr)
          {
            break;
          }
        j++;
      }
    
    uint16_t k;
    do
      {
        k = j;
        j = m_modNext [i * m_nodeTable.size() + j];
        NS_LOG_INFO ("@@ " << i << " " << j << " " << k);
      }
    while (i != j);
    
    if (DistFromTable (i, k) > m_txRange)
      {
        NS_LOG_DEBUG ("No Path Exists!");
        return srcAddr;
      }
    
    ModNodeEntry se = m_nodeTable.at (k);
    return se.addr;
}

// Find shortest paths for all pairs using Floyd-Warshall algorithm 
void 
ModRoutingTable::UpdateRoute (double txRange)
{
  NS_LOG_FUNCTION ("");

  m_txRange = txRange;
  uint16_t n = m_nodeTable.size(); // number of nodes
  uint16_t i, j, k; //loop counters
  double distance;

  //initialize data structures
  if (m_modNext != 0)
    {
      delete m_modNext;
    }
  if (m_modDist != 0)
    {
      delete m_modDist;
    }

  double* dist = new double [n * n];
  uint16_t* pred = new uint16_t [n * n];

  //algorithm initialization
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
        {
          if (i == j)
            {
              dist [i * n + j] = 0;
            }
          else
            {
              distance = DistFromTable (i, j);
              if (distance > 0 && distance <= txRange)
                {
	                //dist [i * n + j] = distance; // shortest distance
                  dist [i * n + j] = 1; // shortest hop
                }
              else
                {
	                dist [i * n + j] = HUGE_VAL;
                }
              pred [i * n + j] = i;
            }
        }
    }
    
  // Main loop of the algorithm
  for (k = 0; k < n; k++)
    {
      for (i = 0; i < n; i++)
        {
          for (j = 0; j < n; j++)
            {
              distance = std::min (dist[i * n + j], dist[i * n + k] + dist[k * n + j]);
              if (distance != dist[i * n + j])
                {
                  dist[i * n + j] = distance;
                  pred[i * n + j] = pred[k * n + j];
                }
            }
        }
    }
    
  m_modNext = pred; // predicate matrix, useful in reconstructing shortest routes
  m_modDist = dist;
  
  string str;
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
      {
        str.append (boost::lexical_cast<string>( pred[i * n + j] ));
        str.append (" ");
      }
      NS_LOG_INFO (str);
      str.erase (str.begin(), str.end());
    }
}

// Get direct-distance between two nodes
double
ModRoutingTable::GetDistance (Ipv4Address srcAddr, Ipv4Address dstAddr)
{
    uint16_t i = 0, j = 0;
    uint16_t n = m_nodeTable.size(); // number of nodes
    Ipv4Address relay;
    std::vector<ModNodeEntry>::iterator it = m_nodeTable.begin ();
    
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == srcAddr)
          {
            break;
          }
        i++;
      }
    it = m_nodeTable.begin ();
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == dstAddr)
          {
            break;
          }
        j++;
      }
    
    return m_modDist [i * n + j];
}

double 
ModRoutingTable::DistFromTable (uint16_t i, uint16_t j)
{
  Vector pos1, pos2;
  ModNodeEntry se;
  double dist;
  
  se = m_nodeTable.at (i);
  pos1 = (se.node)->GetObject<MobilityModel> ()->GetPosition ();
  se = m_nodeTable.at (j);
  pos2 = (se.node)->GetObject<MobilityModel> ()->GetPosition ();
  
  dist = pow (pos1.x - pos2.x, 2.0) + pow (pos1.y - pos2.y, 2.0) + pow (pos1.z - pos2.z, 2.0);
  return sqrt (dist);
}

void
ModRoutingTable::Print (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION ("");
  // not implemented
}

} // namemodace ns3
