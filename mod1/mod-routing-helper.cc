
#include "ns3/pointer.h"
#include "mod-routing-helper.h"

namespace ns3
{
ModRoutingHelper::ModRoutingHelper ()
 : Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::ModRouting");
}

ModRoutingHelper* 
ModRoutingHelper::Copy (void) const 
{
  return new ModRoutingHelper (*this);
}

Ptr<Ipv4RoutingProtocol> 
ModRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<ModRouting> agent = m_agentFactory.Create<ModRouting> ();
  node->AggregateObject (agent);
  return agent;
}

void 
ModRoutingHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

}
