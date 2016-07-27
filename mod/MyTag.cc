#include <Mytag.h>

uint32_t
MyTag::GetTagItems(void) const{
  std::cout << "GetTagItems " << m_items <<"\n";
  return (uint32_t)(m_items+1);
}
void
MyTag::SetTagItems(int maxItems) {
  std::cout << "SetTagItems " << maxItems <<"\n";
  m_items = maxItems;
}
TypeId 
MyTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyTag")
    .SetParent<Tag> ()
    .AddConstructor<MyTag> ()
    // .AddAttribute ("SimpleValue",
    //                "A simple value",
    //                EmptyAttributeValue (),
    //                MakeUintegerAccessor (&MyTag::GetSimpleValue),
    //                MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}
TypeId 
MyTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t 
MyTag::GetSerializedSize (void) const
{
  // SetTagItems(2);
  return GetTagItems();
}
void 
MyTag::Serialize (TagBuffer i) const
{
  std::cout <<"Serialize called\n";
  i.WriteU8((uint8_t)m_simpleValue.size());
  i.WriteVector (m_simpleValue);
  std::cout << "Serialize done\n";
}
void 
MyTag::Deserialize (TagBuffer i)
{
  std::cout <<"Deserialize called\n";
  int itemsToRead = (int) i.ReadU8();
  m_simpleValue = i.ReadVector (itemsToRead);
  std::cout <<"Deserialize done\n";
}
void 
MyTag::Print (std::ostream &os) const
{
  os << "v=";
  for (auto x : m_simpleValue){
    os << x << " ";
  }
}
void 
MyTag::SetSimpleValue (std::vector<uint8_t> value)
{
  m_simpleValue = value;
  SetTagItems(m_simpleValue.size());
}

std::vector<uint8_t> 
MyTag::GetSimpleValue (void) const
{
  return m_simpleValue;
}
void 
MyTag::SetSimpleValueByIndex(int index, uint8_t value){
  m_simpleValue[index-1] = value;
}
uint8_t MyTag::GetSimpleValueByIndex(int index) const{
  return m_simpleValue[index-1];
}
void 
MyTag::printTag(MyTag tag) const{
  std::cout << "Printing tag size = "<< tag.GetSimpleValue().size()  << "\nv= ";
  for( auto x: tag.GetSimpleValue()){
    std::cout << x << " ";
  }
  std::cout <<"\n-----------------------\n";
}