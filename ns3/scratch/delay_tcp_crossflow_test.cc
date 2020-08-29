 /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//       n0 ----------- n1
//            500 Kbps
//             5 ms
//
// - Flow from n0 to n1 using BulkSendApplication.
// - Tracing of queues and packet receptions to file "tcp-bulk-send.tr"
//   and pcap tracing available when tracing is turned on.

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include <stdlib.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample");

int config = 0;


std::vector<std::vector<int64_t>> rtt_vars;
ApplicationContainer sourceApps;
void
GetRttVar(){
  ApplicationContainer::Iterator j;
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    std::vector<int64_t> new_rtt_vars = p->GetAllRttVar();
    //rtt_vars.insert(rtt_vars.end(), new_rtt_vars.begin(), new_rtt_vars.end());
    rtt_vars.push_back(new_rtt_vars);
  }
  for(int i = 0; i<rtt_vars.size();++i){
    for(int j = 0; j<rtt_vars[i].size();++j){
      std::cout << rtt_vars[i][j]<<", ";
    }
    std::cout << std::endl;
  }
  Simulator::Schedule (Seconds(1), Config::Set, "/ChannelList/1/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(rand() % 100)));
  Simulator::Schedule (Seconds(1), Config::Set, "/ChannelList/2/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(rand() % 100)));
  Simulator::Schedule (Seconds(1), Config::Set, "/ChannelList/3/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(rand() % 100)));
  Simulator::Schedule (Seconds (0.1), &GetRttVar);
}
int
main (int argc, char *argv[])
{
RngSeedManager::SetRun (34);

  bool tracing = true;
  uint32_t maxBytes = 0;
 // Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue(true));
 // LogComponentEnable("MpTcpSocketBase", LOG_INFO);
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(3)); // Sink
  Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(false));
  Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Round_Robin"));
 // Error model // Set a few attributes
  Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.001));
 Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));


//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (5); // a=0; b=1: c=2; d=3; e=4

//        d (dest)
//       /|\
//      / | \       e-a-d: 10 Mbps
//     /  |  \      e-b-d: 5  Mbps
//    a   b   c     e-c-d: 5  Mbps
//     \  |  /
//      \ | /
//       \|/
//        e (source)

  // e-a-d
  NodeContainer ad = NodeContainer(nodes.Get(0), nodes.Get(3));
  NodeContainer ae = NodeContainer(nodes.Get(0), nodes.Get(4));
  // e-b-d
  NodeContainer bd = NodeContainer(nodes.Get(1), nodes.Get(3));
  NodeContainer be = NodeContainer(nodes.Get(1), nodes.Get(4));
  // e-c-d
  NodeContainer cd = NodeContainer(nodes.Get(2), nodes.Get(3));
  NodeContainer ce = NodeContainer(nodes.Get(2), nodes.Get(4));

  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(30)));
  NetDeviceContainer e_a_container = pointToPoint.Install(ae);;
  NetDeviceContainer a_d_container = pointToPoint.Install(ad);
  

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  NetDeviceContainer b_d_container = pointToPoint.Install(bd);
  NetDeviceContainer e_b_container = pointToPoint.Install(be);
  NetDeviceContainer c_d_container = pointToPoint.Install(cd);
  NetDeviceContainer e_c_container = pointToPoint.Install(ce);
//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (nodes);

//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iAD = ipv4.Assign (a_d_container);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iEA = ipv4.Assign (e_a_container);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iBD = ipv4.Assign (b_d_container);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer iEB = ipv4.Assign (e_b_container);
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer iCD = ipv4.Assign (c_d_container);
  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer iEC = ipv4.Assign (e_c_container);
  NS_LOG_INFO ("Create Applications.");

  Ipv4GlobalRoutingHelper g;
  g.PopulateRoutingTables ();
//
// Create a BulkSendApplication and install it
//
  uint16_t port = 9;  // well-known echo port number


  MpTcpBulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (iAD.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  sourceApps = source.Install (nodes.Get (4));
  ApplicationContainer::Iterator j;
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
  //  p->SetNeat(ntw);
  }
  
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));

//
// Create a PacketSinkApplication and install it on node 1
//
  MpTcpPacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (3));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));
  //
// Set Error model onto sinkapp 
  ObjectFactory factory;
  factory.SetTypeId ("ns3::RateErrorModel");
  Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
  a_d_container.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  b_d_container.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  c_d_container.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));



// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Schedule (Seconds (1), &GetRttVar);
  Simulator::Schedule (Seconds(1), Config::Set, "/ChannelList/3/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(rand() % 100)));
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  std::vector<int64_t> sendLog;
  
  
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    sendLog = p->GetTimeLog();
    //rtt_vars = p->GetAllRttVar();
  }
  std::vector<int64_t> receivedLog = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0))->GetTimeLog ();

  std::cout << "send: " << sendLog.size () << std::endl;
  std::cout << "rcvd: " << receivedLog.size () << std::endl;
  
  std::cout << std::endl;
  
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx ()/1000 << std::endl;
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  
}
