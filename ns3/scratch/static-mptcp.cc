/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("test");

int 
main (int argc, char *argv[])
{
 /* Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));

  


  
  uint16_t port = 9;   
  MpTcpBulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (ifInAddrC1.GetLocal(), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (nA);
  ApplicationContainer::Iterator j;
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
   // p->SetNeat(ntw);
  }
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));

  MpTcpPacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nC);
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));

  Simulator::Run ();
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx ()/(10*1000000)<<" Mbps" << std::endl;

  Simulator::Destroy ();
*/
  return 0;
}
