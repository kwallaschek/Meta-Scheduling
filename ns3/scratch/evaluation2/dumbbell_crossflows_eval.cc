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

NS_LOG_COMPONENT_DEFINE ("Dumbbell21e1");



std::vector<double> 
Eval_Dumbbell (int runNum, bool neat, NEAT::Network* ntw, int config, bool meta)
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
  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(false));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(20));
  //Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
  Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("UNCOUPLED")); 
 // if(neat){  
    //Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
 //   Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Double_Out"));
 // }else{
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Min_Rtt"));
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(20)); 
 /* }
  if(meta){
    Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta"));
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(50)); 
  }
  
 /*if (config == 1){
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta")); 
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(50)); 
  }
  if (config == 2){
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta"));  
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
  nodes.Create (10);

//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (nodes);
//  A 			H
//  	D---F
//  B   		I
//  	E---G
//  C 			J

// LeftSide
  NodeContainer AD = NodeContainer(nodes.Get(0), nodes.Get(3));
  NodeContainer BD = NodeContainer(nodes.Get(1), nodes.Get(3));
  NodeContainer BE = NodeContainer(nodes.Get(1), nodes.Get(4));
  NodeContainer CE = NodeContainer(nodes.Get(2), nodes.Get(4));
// Middle
  NodeContainer DF = NodeContainer(nodes.Get(3), nodes.Get(5));
  NodeContainer EG = NodeContainer(nodes.Get(4), nodes.Get(6));
// Right
  NodeContainer FH = NodeContainer(nodes.Get(5), nodes.Get(7));
  NodeContainer FI = NodeContainer(nodes.Get(5), nodes.Get(8));
  NodeContainer GI = NodeContainer(nodes.Get(6), nodes.Get(8));
  NodeContainer GJ = NodeContainer(nodes.Get(6), nodes.Get(9));


  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;
  NetDeviceContainer nodeAD;
  NetDeviceContainer nodeBD;
  NetDeviceContainer nodeBE;
  NetDeviceContainer nodeCE;
  NetDeviceContainer nodeDF;
  NetDeviceContainer nodeEG;
  NetDeviceContainer nodeFH;
  NetDeviceContainer nodeFI;
  NetDeviceContainer nodeGI;
  NetDeviceContainer nodeGJ;

  // Path topside
  if (config == 1){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Kbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(2)));  
  }
  if (config == 2 || config == 3){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Kbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));  
  }
  nodeAD = pointToPoint.Install(AD);
  nodeBD = pointToPoint.Install(BD);
  nodeDF = pointToPoint.Install(DF);
  nodeFH = pointToPoint.Install(FH);
  nodeFI = pointToPoint.Install(FI);
  
  // Path bottomside
  if (config == 1){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Kbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(20)));  
  }
  if (config == 2 || config == 3){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Kbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(1)));  
  }
  nodeBE = pointToPoint.Install(BE);
  nodeCE = pointToPoint.Install(CE);
  nodeEG = pointToPoint.Install(EG);
  nodeGI = pointToPoint.Install(GI);
  nodeGJ = pointToPoint.Install(GJ);

  if(config == 3){

    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/5/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/6/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/7/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/8/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/9/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/5/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/6/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/7/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/8/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/9/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
  }
//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  // leftside
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iAD = ipv4.Assign (nodeAD);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iBD = ipv4.Assign (nodeBD);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iBE = ipv4.Assign (nodeBE);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer iCE = ipv4.Assign (nodeCE);
  // middle
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer iDF = ipv4.Assign (nodeDF);
  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer iEG = ipv4.Assign (nodeEG);
  
  // rightside
  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer iFH = ipv4.Assign (nodeFH);
  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer iFI = ipv4.Assign (nodeFI);
  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer iGI = ipv4.Assign (nodeGI);
  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer iGJ = ipv4.Assign (nodeGJ);


  
  NS_LOG_INFO ("Create Applications.");

  Ipv4GlobalRoutingHelper g;
  g.PopulateRoutingTables ();
//
// Create a BulkSendApplication and install it on node 0
//
  uint16_t port = 9;  // well-known echo port number

  
//
// Create a BulkSendApplication and install it on node B
//
  MpTcpBulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (iFI.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps = source.Install (nodes.Get (1));
  for (auto j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    if(meta){
      Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
      p->SetNeat(neat_18,ntw);
    }else{
      if(neat){
        Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
        p->SetNeat(ntw, NULL);
      }
    }
  }
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (time-1));

//
// Create a PacketSinkApplication and install it on node D
//
  MpTcpPacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (iFI.GetAddress (1), port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (8));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (time-1));

//
// Create TCP Crossflows
// 

  // Crossflow A -> H
  MpTcpBulkSendHelper source2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (iFH.GetAddress (1), 11));
  source2.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps2 = source2.Install (nodes.Get (0));
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
                         InetSocketAddress (iFH.GetAddress (1), 11));
  ApplicationContainer sinkApps2 = sink2.Install (nodes.Get (7));
  sinkApps2.Start (Seconds (1.0));
  sinkApps2.Stop (Seconds (time-1));

  // Crossflow C -> J
  MpTcpBulkSendHelper source3 ("ns3::TcpSocketFactory",
                         InetSocketAddress (iGJ.GetAddress (1), 12));
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
                         InetSocketAddress (iGJ.GetAddress (1), 12));
  ApplicationContainer sinkApps3 = sink3.Install (nodes.Get (9));
  sinkApps3.Start (Seconds (1.0));
  sinkApps3.Stop (Seconds (time-1));

// Set Error model onto sinkapp 
 ObjectFactory factory;
 factory.SetTypeId ("ns3::RateErrorModel");
 Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
 nodeAD.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeBE.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeCE.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeBD.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeDF.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeEG.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeFH.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeFI.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeGI.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeGJ.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

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
  
  monitor->SerializeToXmlFile ("results1.xml" , true, true );
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
  double cj = 0.0;
  double ah = 0.0;
  //if(sink1->GetTotalRx () > 0)
  mptcp = (sink1->GetTotalRx ()*8)/(1000*(time-2));
  //if(sink_2->GetTotalRx () > 0)
  cj = (sink_2->GetTotalRx ()*8)/(1000*(time-2));
  //if(sink_3->GetTotalRx () > 0)
  ah = (sink_3->GetTotalRx ()*8)/(1000*(time-2));
  //std::cout << "Total Bytes Received: \n" << "mptcp: " << mptcp << "Mbps"  << std::endl;
  
  //std::cout << "Mean Delay: " << meanDelayMp << " ms"<< std::endl;
  
  //std::cout << "C->J: " << cj << "Mbps" << std::endl;
  
  //std::cout << "A->H: " << ah << "Mbps"  << std::endl;
  
  //std::cout << "Sum: " << mptcp + cj + ah << "Mbps" << std::endl;

  
  double nominator = pow(mptcp + cj + ah, 2);
  double denominator = 4* (pow(mptcp/2,2)+pow(mptcp/2,2)+pow(cj,2)+ pow(ah,2)); 
  //std::cout << "Fairness: " << nominator/denominator<< std::endl;
  
  std::vector<double> results;
  results.push_back(mptcp+cj+ah);
  results.push_back(mptcp);
  results.push_back(ah);
  results.push_back(cj);
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