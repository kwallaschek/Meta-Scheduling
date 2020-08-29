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
#include <iostream>
#include "../Neat/network.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample21e1e");


int config = 0;

std::vector<double> 
Eval (int runNum, bool neat, NEAT::Network* ntw, bool meta)
{
RngSeedManager::SetRun (runNum);
  /** Initialize NEAT-Network**/
  NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
  NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
  NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
  NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::SENSOR, 4, NEAT::nodeplace::INPUT);
  NEAT::NNode* node5 = new NEAT::NNode(NEAT::nodetype::SENSOR, 5, NEAT::nodeplace::INPUT);
  NEAT::NNode* node6 = new NEAT::NNode(NEAT::nodetype::SENSOR, 6, NEAT::nodeplace::INPUT);
  NEAT::NNode* node7 = new NEAT::NNode(NEAT::nodetype::SENSOR, 7, NEAT::nodeplace::INPUT);
  NEAT::NNode* node8 = new NEAT::NNode(NEAT::nodetype::SENSOR, 8, NEAT::nodeplace::INPUT);
  NEAT::NNode* node9 = new NEAT::NNode(NEAT::nodetype::NEURON, 9, NEAT::nodeplace::OUTPUT);
  NEAT::NNode* node10 = new NEAT::NNode(NEAT::nodetype::NEURON, 10, NEAT::nodeplace::OUTPUT);

  
  // Add Links to Nodes
  node9->add_incoming(node1,  0.771147);
  node9->add_incoming(node2, -0.944704);
  node9->add_incoming(node3,  0.335433);
  node9->add_incoming(node4, -0.200188);
  node9->add_incoming(node5,-0.722862);
  node9->add_incoming(node6, -0.652788);
  node9->add_incoming(node7, -0.377467);
  node9->add_incoming(node8,  -0.67096);
  node10->add_incoming(node1,   0.604871);
  node10->add_incoming(node2,  0.260299);
  node10->add_incoming(node3,  0.647212);
  node10->add_incoming(node4, -0.209242);
  node10->add_incoming(node5,-0.440218);
  node10->add_incoming(node6,  0.782647);
  node10->add_incoming(node7, -0.223213);
  node10->add_incoming(node8, -0.782433);

  std::vector<NEAT::NNode*> allNodes;
  std::vector<NEAT::NNode*> inputNodes;
  std::vector<NEAT::NNode*> outputNodes;

  // Fill Arrays with the corresponding Nodes
  allNodes.push_back(node1);
  allNodes.push_back(node2);
  allNodes.push_back(node3);
  allNodes.push_back(node4);
  allNodes.push_back(node5);
  allNodes.push_back(node6);
  allNodes.push_back(node7);
  allNodes.push_back(node8);
  allNodes.push_back(node9);
  allNodes.push_back(node10);

  inputNodes.push_back(node1);
  inputNodes.push_back(node2);
  inputNodes.push_back(node3);
  inputNodes.push_back(node4);
  inputNodes.push_back(node5);
  inputNodes.push_back(node6);
  inputNodes.push_back(node7);
  inputNodes.push_back(node8);

  outputNodes.push_back(node9);
  outputNodes.push_back(node10);

  // Build Network
  NEAT::Network* neat_18 = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);
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
  //if(neat){  
    Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  //  Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Double_Out"));
 // }else{
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Double_Out"));
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(20));
 // }
  /*if(meta){
    Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta"));
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(50)); 
  }*/
  // Error model // Set a few attributes
  Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));

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
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Kbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));
  
  nodeAC = pointToPoint.Install(ac);
  nodeAD = pointToPoint.Install(ad);
  
  // Path over B
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Kbps"));
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
  ApplicationContainer sourceApps = source.Install (nodes.Get (2));
  for (auto j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    if(!meta){
      if(neat){
        p->SetNeat(ntw,NULL);
      }
      
    }
    else 
      p->SetNeat(neat_18,ntw);
  }
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
  
  monitor->SerializeToXmlFile ("results1231.xml" , true, true );
  // Print per flow statistics
  
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon_helper.GetClassifier ());
  std::map< FlowId, FlowMonitor::FlowStats > stats = monitor->GetFlowStats ();
  double Thrput=0.0;
  double transmit_packets=0.0;
  double recieve_packets=0.0;
  double delaySum = 0.0;
  double lostPacketsMp = 0.0;
  double lostPacketsCad = 0.0;
  double lostPacketsCbd = 0.0;
  double meanDelayMp = 0.0;
  double meanDelayCad = 0.0;
  double meanDelayCbd = 0.0;
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
      if(iter->second.rxPackets>0){
        counter++;
        meanDelayMp += iter->second.delaySum.GetMilliSeconds ()/iter->second.rxPackets;
        lostPacketsMp += iter->second.lostPackets;
      }
    } else if (t.destinationPort == 11){
      if(iter->second.rxPackets>0){
        meanDelayCbd += iter->second.delaySum.GetMilliSeconds ()/iter->second.rxPackets;
        lostPacketsCbd += iter->second.lostPackets;
      }
    } else if (t.destinationPort == 12){
      if(iter->second.rxPackets>0){
        meanDelayCad += iter->second.delaySum.GetMilliSeconds ()/iter->second.rxPackets;
        lostPacketsCad += iter->second.lostPackets;
      }
    }
  }
  Simulator::Destroy ();
  if(counter != 0){
    meanDelayMp/=counter; // Devide by the number of subflows
    lostPacketsMp/=counter;
  }
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  Ptr<MpTcpPacketSink> sink_2 = DynamicCast<MpTcpPacketSink> (sinkApps2.Get (0));
  Ptr<MpTcpPacketSink> sink_3 = DynamicCast<MpTcpPacketSink> (sinkApps3.Get (0));
  double mptcp = 0.0;
  double cbd = 0.0;
  double cad = 0.0;
  //if(sink1->GetTotalRx () > 0)
  mptcp = (sink1->GetTotalRx ()*8)/(1000*(time-2));
  //if(sink_2->GetTotalRx () > 0)
  cbd = (sink_2->GetTotalRx ()*8)/(1000*(time-2));
  //if(sink_3->GetTotalRx () > 0)
  cad = (sink_3->GetTotalRx ()*8)/(1000*(time-2));
  //std::cout << "Total Bytes Received: \n" << "mptcp: " << mptcp << "Mbps"  << std::endl;
  
  //std::cout << "Mean Delay: " << meanDelayMp << " ms"<< std::endl;
  
  //std::cout << "C->B->D: " << cbd << "Mbps" << std::endl;
  
  //std::cout << "C->A->D: " << cad << "Mbps"  << std::endl;
  
  //std::cout << "Sum: " << mptcp + cbd + cad << "Mbps" << std::endl;
  
  double nominator = pow(mptcp + cbd + cad, 2);
  double denominator = 4* (pow(mptcp/2,2)+pow(mptcp/2,2)+pow(cbd,2)+ pow(cad,2)); 
  //std::cout << "Fairness: " << nominator/denominator<< std::endl;
  
  std::vector<double> results;
  results.push_back(mptcp+cbd+cad);
  results.push_back(mptcp);
  results.push_back(cad);
  results.push_back(cbd);
  results.push_back(meanDelayMp);
  results.push_back(meanDelayCad);
  results.push_back(meanDelayCbd);
  results.push_back((meanDelayMp+meanDelayCad+meanDelayCbd)/3);
  results.push_back(lostPacketsMp);
  results.push_back(lostPacketsCad);
  results.push_back(lostPacketsCbd);
  results.push_back(nominator/denominator);

  
  NS_LOG_INFO ("Done.");
  return results;
}