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
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-classifier.h"
#include "ns3/flow-probe.h"
#include "ns3/histogram.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-flow-probe.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample");


int config = 0;

static void 
CwndTracer (uint32_t oldval, uint32_t newval)
{
  std::cout << "Moving cwnd from " << oldval << " to " << newval << std::endl;
}

std::vector<std::vector<int64_t>> rtt_vars;
ApplicationContainer sourceApps;

int
main (int argc, char *argv[])
{

RngSeedManager::SetRun (1);

  bool tracing = false;
  uint32_t maxBytes = 0;
  auto time = 10.0;
  // Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue(true));
  // LogComponentEnable("MpTcpSocketBase", LOG_INFO);
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  //Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
  Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("UNCOUPLED")); 
  //Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Min_Rtt"));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(10));
  // Error model // Set a few attributes
  //Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
  //Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  //Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.01));
  //Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));

//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (4);

//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (nodes);
//      c h1
//     / \
// s0 a   b s1
//     \ /
//      d h2

//  NodeContainer ab = NodeContainer(nodes.Get(0), nodes.Get(1));
  NodeContainer ac = NodeContainer(nodes.Get(0), nodes.Get(2));
  NodeContainer cb = NodeContainer(nodes.Get(2), nodes.Get(1));
  NodeContainer ad = NodeContainer(nodes.Get(0), nodes.Get(3));
  NodeContainer bd = NodeContainer(nodes.Get(1), nodes.Get(3));

  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;
  NetDeviceContainer nodeAC;
  NetDeviceContainer nodeAD;
  NetDeviceContainer nodeCB;
  NetDeviceContainer nodeBD;

  // Path over A
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));
  
  nodeAC = pointToPoint.Install(ac);
  nodeAD = pointToPoint.Install(ad);
  
  // Path over B
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(1)));
  
  nodeCB = pointToPoint.Install(cb);
  nodeBD = pointToPoint.Install(bd);


//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
 // Ipv4InterfaceContainer iAB = ipv4.Assign (devices);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iAC = ipv4.Assign (nodeAC);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iCB = ipv4.Assign (nodeCB);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer iAD = ipv4.Assign (nodeAD);
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer iBD = ipv4.Assign (nodeBD);


  
  NS_LOG_INFO ("Create Applications.");

  Ipv4GlobalRoutingHelper g;
  g.PopulateRoutingTables ();
//
// Create a BulkSendApplication and install it on node 0
//
  uint16_t port = 9;  // well-known echo port number

  
//
// Create a BulkSendApplication and install it on node C
//
  MpTcpBulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (iAD.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  sourceApps = source.Install (nodes.Get (2));
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (time-1));

//
// Create a PacketSinkApplication and install it on node D
//
  MpTcpPacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (3));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (time-1));

//
// Create TCP Crossflows
// 

  // Crossflow over B
  MpTcpBulkSendHelper source2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (iBD.GetAddress (1), 11));
  source2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps2 = source2.Install (nodes.Get (2));
  ApplicationContainer::Iterator j;
  for (j = sourceApps2.Begin(); j != sourceApps2.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    // DisableMpTcp to mimic normal TCP flows
    p->DisableMpTcp();
  }
  sourceApps2.Start (Seconds (1.0));
  sourceApps2.Stop (Seconds (time-1));

   MpTcpPacketSinkHelper sink2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (iBD.GetAddress (1), 11));
  ApplicationContainer sinkApps2 = sink2.Install (nodes.Get (3));
  sinkApps2.Start (Seconds (1.0));
  sinkApps2.Stop (Seconds (time-1));

  // Crossflow over A
  MpTcpBulkSendHelper source3 ("ns3::TcpSocketFactory",
                         InetSocketAddress (iAD.GetAddress (1), 12));
  source3.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps3 = source3.Install (nodes.Get (2));
  for (j = sourceApps3.Begin(); j != sourceApps3.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    // DisableMpTcp to mimic normal TCP flows
    p->DisableMpTcp();
  }
  sourceApps3.Start (Seconds (1.0));
  sourceApps3.Stop (Seconds (time-1));

  MpTcpPacketSinkHelper sink3 ("ns3::TcpSocketFactory",
                         InetSocketAddress (iAD.GetAddress (1), 12));
  ApplicationContainer sinkApps3 = sink3.Install (nodes.Get (3));
  sinkApps3.Start (Seconds (1.0));
  sinkApps3.Stop (Seconds (time-1));

// Set Error model onto sinkapp 
// ObjectFactory factory;
// factory.SetTypeId ("ns3::RateErrorModel");
// Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
// nodeAD.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
// nodeBD.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

//
// Flow Monitor
// 
  Ptr<FlowMonitor> monitor;
  FlowMonitorHelper flowmon_helper;
  monitor = flowmon_helper.InstallAll();
  //Simulator::Schedule (Seconds (1), &GetRttVar);
  
//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (time));
  Simulator::Run ();

//
// Get Delay stats
// 
  monitor->SerializeToXmlFile ("results.xml" , true, true );
  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon_helper.GetClassifier ());
  std::map< FlowId, FlowMonitor::FlowStats > stats = monitor->GetFlowStats ();
  double Thrput=0.0;
  double transmit_packets=0.0;
  double recieve_packets=0.0;
  double delaySum = 0.0;
  double lostPackets= 0.0;
  double meanDelay = 0.0;
  int counter = 0;
   
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
  {
  // Check if port is 9 with t.destinationPort
  // get delaySum and divide by receivedPkts -> meanDelay
  // Also jitterSum possible -> make as small as possible
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
  //std::cout<< t.destinationPort<< ": " <<iter->second.delaySum.GetSeconds () << std::endl;
  //std::cout<< t.destinationPort<< ": " <<iter->second.delaySum.GetSeconds ()/iter->second.rxPackets << std::endl;
  //transmit_packets=transmit_packets+iter->second.txPackets;
  //recieve_packets=recieve_packets+iter->second.rxPackets;
  //delaySum+=iter->second.delaySum.GetSeconds();
  //lostPackets+=iter->second.lostPackets;
    if (t.destinationPort == 9) {
      counter++;
      meanDelay += iter->second.delaySum.GetMilliSeconds ()/iter->second.rxPackets;
    }
  }
  meanDelay/=2; // Devide by the number of subflows
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  Ptr<MpTcpPacketSink> sink_2 = DynamicCast<MpTcpPacketSink> (sinkApps2.Get (0));
  Ptr<MpTcpPacketSink> sink_3 = DynamicCast<MpTcpPacketSink> (sinkApps3.Get (0));
  float mptcp = (sink1->GetTotalRx ()*8)/(1000000*(time-2));
  float cbd = (sink_2->GetTotalRx ()*8)/(1000000*(time-2));
  float cad = (sink_3->GetTotalRx ()*8)/(1000000*(time-2));
  std::cout << "Total Bytes Received: \n" << "mptcp: " << mptcp << "Mbps"  << std::endl;
  std::cout << "Mean Delay: " << meanDelay << " ms"<< std::endl;
  std::cout << "C->B->D: " << cbd << "Mbps" << std::endl;
  std::cout << "C->A->D: " << cad << "Mbps"  << std::endl;
  std::cout << "Sum: " << mptcp + cbd + cad << "Mbps" << std::endl;
  float nominator = pow(mptcp + cbd + cad, 2);
  float denominator = 3* (pow(mptcp,2)+pow(cbd,2)+ pow(cad,2)); 
  std::cout << "Fairness: " << nominator/denominator<< std::endl;
  
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  
}
