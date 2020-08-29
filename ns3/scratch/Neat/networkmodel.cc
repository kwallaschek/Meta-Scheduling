/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// This program configures a grid (default 5x5) of nodes on an
// 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to node 1.
//
// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc-grid --help"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the ns-3 documentation.
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when distance increases beyond
// the default of 500m.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --distance=500"
// ./waf --run "wifi-simple-adhoc --distance=1000"
// ./waf --run "wifi-simple-adhoc --distance=1500"
//
// The source node and sink node can be changed like this:
//
// ./waf --run "wifi-simple-adhoc --sourceNode=20 --sinkNode=10"
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc-grid --verbose=1"
//
// By default, trace file writing is off-- to enable it, try:
// ./waf --run "wifi-simple-adhoc-grid --tracing=1"
//
// When you are done tracing, you will notice many pcap trace files
// in your directory.  If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-grid-0-0.pcap -nn -tt
//

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/energy-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-classifier.h"
#include "ns3/flow-probe.h"
#include "ns3/histogram.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-flow-probe.h"
#include "../Neat/network.h"
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "../Neat/networkmodel.h"


#include <iostream>
#include <fstream>
#include <vector>
#include <string>

////NS_LOG_COMPONENT_DEFINE ("NeatNetwork2");

using namespace ns3;




int nodeNums;

double meanThroughput;

double totalThroughput;

double runTimer;


//std::vector<double> Initialize (int nodeNum, int runNum, int runTime, bool neat, NEAT::Network *ntw)
double Initialize_thr_1 (int nodeNum, int runNum, int runTime, bool neat, NEAT::Network *ntw, int config)
{
  RngSeedManager::SetRun (runNum);
  
  bool tracing = true;
  uint32_t maxBytes = 0;
  auto time = 10.0;
 // Config::SetDefault ("ns3::TcpSocketBase::EnableMpTcp", BooleanValue(true));
 // LogComponentEnable("MpTcpSocketBase", LOG_INFO);
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
  Config::SetDefault("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(100));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
  Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Double_Out"));


//
// Explicitly create the nodes required by the topology (shown above).
//
  //NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (4);

//      c
//     / \
//    a---b
//     \ /
//      d

  NodeContainer ab = NodeContainer(nodes.Get(0), nodes.Get(1));
  NodeContainer ac = NodeContainer(nodes.Get(0), nodes.Get(2));
  NodeContainer cb = NodeContainer(nodes.Get(2), nodes.Get(1));

  NodeContainer ad = NodeContainer(nodes.Get(0), nodes.Get(3));
  NodeContainer bd = NodeContainer(nodes.Get(1), nodes.Get(3));

  //NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;
  NetDeviceContainer devices;
  //devices = pointToPoint.Install (nodes);
  //devices = pointToPoint.Install(ab);

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(10)));
  NetDeviceContainer nodeAC;
  NetDeviceContainer nodeAD;
//  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(10)));
  NetDeviceContainer nodeCB;
  NetDeviceContainer nodeBD;

  nodeAC = pointToPoint.Install(ac);
  nodeAD = pointToPoint.Install(ad);
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));
  nodeCB = pointToPoint.Install(cb);
  nodeBD = pointToPoint.Install(bd);
//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (nodes);

//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  //NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iAB = ipv4.Assign (devices);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iAC = ipv4.Assign (nodeAC);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iCB = ipv4.Assign (nodeCB);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer iAD = ipv4.Assign (nodeAD);
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer iBD = ipv4.Assign (nodeBD);
  //NS_LOG_INFO ("Create Applications.");

  Ipv4GlobalRoutingHelper g;
//   Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>
//("dynamic-global-routing.routes", std::ios::out);
//   g.PrintRoutingTableAllAt (Seconds (12), routingStream);
  g.PopulateRoutingTables ();
//
// Create a BulkSendApplication and install it on node 0
//
  uint16_t port = 9;  // well-known echo port number


  MpTcpBulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (iAD.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps = source.Install (nodes.Get (2));
  ApplicationContainer::Iterator j;
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    p->SetNeat(ntw,NULL);
  }
  
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (time));

//
// Create a PacketSinkApplication and install it on node 1
//
  MpTcpPacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (3));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (time));
//  if (config == 1) {
//    Simulator::Schedule (Seconds(time), Config::Set, "/ChannelList/3/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(10)));
//    Config::Set("/NodeList/3/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("50Mbps") );
//  }



// Now, do the actual simulation.
//

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (time));

  Simulator::Run ();
  
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  //std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
  Simulator::Destroy ();
  //NS_LOG_INFO ("Done.");
  // need to return fitness here
  return (sink1->GetTotalRx ()*8)/(1000000*(time-2));
}

double Initialize_thr_2 (int runNum, NEAT::Network *ntw, int config)
{
  RngSeedManager::SetRun (runNum);
  

  bool tracing = false;
  uint32_t maxBytes = 0;
  auto time = 10.0;
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
  Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Triple_Out"));

  // Error model // Set a few attributes
  Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));


//
// Explicitly create the nodes required by the topology (shown above).
//
  //NS_LOG_INFO ("Create nodes.");
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

  //NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(10)));
  NetDeviceContainer e_a_container = pointToPoint.Install(ae);;
  NetDeviceContainer a_d_container = pointToPoint.Install(ad);
  

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));
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
  //NS_LOG_INFO ("Assign IP Addresses.");
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
  //NS_LOG_INFO ("Create Applications.");

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
  ApplicationContainer sourceApps = source.Install (nodes.Get (4));
  ApplicationContainer::Iterator j;
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    p->SetNeat(ntw,NULL);
  }
  
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (time-1));

//
// Create a PacketSinkApplication and install it on node 1
//
  MpTcpPacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (3));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (time-1));
  
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
  Simulator::Stop (Seconds (time));

  Simulator::Run ();
  
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  //std::cout << "Total Bytes Received: " << sink1->GetTotalRx ()/1000 << std::endl;
  Simulator::Destroy ();
  //NS_LOG_INFO ("Done.");
  // need to return fitness here
  return (sink1->GetTotalRx ()*8)/(1000000*(time-2));
}



std::vector<double>
Initialize_fairness_1(int runNum, NEAT::Network *ntw, int config)
{
  RngSeedManager::SetRun (runNum);
  RngSeedManager::SetSeed (1);
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
  Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  if(config == 0){
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Double_Out"));  
  } else {
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta"));
  }
  
  Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(20));
  // Error model // Set a few attributes
  Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.01));
  Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));

//
// Explicitly create the nodes required by the topology (shown above).
//
  //NS_LOG_INFO ("Create nodes.");
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

  //NS_LOG_INFO ("Create channels.");

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
  //NS_LOG_INFO ("Assign IP Addresses.");
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
    if(config == 0)
      p->SetNeat(ntw,NULL);
    if(config == 1)
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
 ObjectFactory factory;
 factory.SetTypeId ("ns3::RateErrorModel");
 Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
 nodeAD.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeBD.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

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
  
  //monitor->SerializeToXmlFile ("results_fairness8.xml" , true, true );
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
  mptcp = (sink1->GetTotalRx ()*8)/(1000000*(time-2));
  //if(sink_2->GetTotalRx () > 0)
  cbd = (sink_2->GetTotalRx ()*8)/(1000000*(time-2));
  //if(sink_3->GetTotalRx () > 0)
  cad = (sink_3->GetTotalRx ()*8)/(1000000*(time-2));
  double nominator = pow(mptcp + cbd + cad, 2);
  double denominator = 4* (pow(mptcp/2,2)+pow(mptcp/2,2)+pow(cbd,2)+ pow(cad,2)); 
  std::vector<double> results;
  results.push_back(mptcp+cbd+cad);
  results.push_back((meanDelayMp+meanDelayCad+meanDelayCbd)/3);
  results.push_back(nominator/denominator);
  return results;

}
std::vector<double>
Initialize_fairness_dd(int runNum, NEAT::Network *ntw, int config)
{
  RngSeedManager::SetRun (runNum);
  RngSeedManager::SetSeed (1);
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
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(50));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1400));
  //Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(0));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(MpTcpSocketBase::GetTypeId()));
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
  Config::SetDefault("ns3::MpTcpSocketBase::CongestionControl", StringValue("UNCOUPLED")); 
  Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  if (config == 0 || config == 4){
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Double_Out"));  
  }
  if (config == 1){
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta")); 
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(50)); 
  }
  if (config == 2){
    Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Neat_Meta"));  
    Config::SetDefault("ns3::MpTcpSocketBase::MaxUnprobedMinRtt", UintegerValue(50)); 
  }

  
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
//  A       H
//    D---F
//  B       I
//    E---G
//  C       J

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
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(2)));  
  }
  if (config == 2){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));  
  }
  if (config == 4){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));   
  }
  nodeAD = pointToPoint.Install(AD);
  nodeBD = pointToPoint.Install(BD);
  nodeDF = pointToPoint.Install(DF);
  nodeFH = pointToPoint.Install(FH);
  nodeFI = pointToPoint.Install(FI);
  
  // Path bottomside
  if (config == 1){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(20)));  
  }
  if (config == 2 ){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(1)));  
  }
  if (config == 4){
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(5)));   
  }
  nodeBE = pointToPoint.Install(BE);
  nodeCE = pointToPoint.Install(CE);
  nodeEG = pointToPoint.Install(EG);
  nodeGI = pointToPoint.Install(GI);
  nodeGJ = pointToPoint.Install(GJ);

  if(config == 4){
    Simulator::Schedule (Seconds(4), Config::Set, "/ChannelList/5/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(50)));
    Simulator::Schedule (Seconds(4), Config::Set, "/ChannelList/6/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(50)));
    Simulator::Schedule (Seconds(4), Config::Set, "/ChannelList/7/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(50)));
    Simulator::Schedule (Seconds(4), Config::Set, "/ChannelList/8/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(50)));
    Simulator::Schedule (Seconds(4), Config::Set, "/ChannelList/9/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(50)));
    Simulator::Schedule (Seconds(4), Config::Set, "/NodeList/5/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("13Mbps"));
    Simulator::Schedule (Seconds(4), Config::Set, "/NodeList/6/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("13Mbps"));
    Simulator::Schedule (Seconds(4), Config::Set, "/NodeList/7/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("13Mbps"));
    Simulator::Schedule (Seconds(4), Config::Set, "/NodeList/8/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("13Mbps"));
    Simulator::Schedule (Seconds(4), Config::Set, "/NodeList/9/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("13Mbps"));

    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/5/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/6/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/7/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/8/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/ChannelList/9/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(25)));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/5/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("1Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/6/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("1Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/7/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("1Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/8/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("1Mbps"));
    Simulator::Schedule (Seconds(5), Config::Set, "/NodeList/9/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("1Mbps"));

    Simulator::Schedule (Seconds(6), Config::Set, "/ChannelList/5/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(40)));
    Simulator::Schedule (Seconds(6), Config::Set, "/ChannelList/6/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(40)));
    Simulator::Schedule (Seconds(6), Config::Set, "/ChannelList/7/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(40)));
    Simulator::Schedule (Seconds(6), Config::Set, "/ChannelList/8/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(40)));
    Simulator::Schedule (Seconds(6), Config::Set, "/ChannelList/9/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(40)));
    Simulator::Schedule (Seconds(6), Config::Set, "/NodeList/5/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("5Mbps"));
    Simulator::Schedule (Seconds(6), Config::Set, "/NodeList/6/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("5Mbps"));
    Simulator::Schedule (Seconds(6), Config::Set, "/NodeList/7/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("5Mbps"));
    Simulator::Schedule (Seconds(6), Config::Set, "/NodeList/8/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("5Mbps"));
    Simulator::Schedule (Seconds(6), Config::Set, "/NodeList/9/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("5Mbps"));

    Simulator::Schedule (Seconds(7), Config::Set, "/ChannelList/5/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(7), Config::Set, "/ChannelList/6/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(7), Config::Set, "/ChannelList/7/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(7), Config::Set, "/ChannelList/8/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(7), Config::Set, "/ChannelList/9/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(7), Config::Set, "/NodeList/5/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(7), Config::Set, "/NodeList/6/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(7), Config::Set, "/NodeList/7/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(7), Config::Set, "/NodeList/8/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));
    Simulator::Schedule (Seconds(7), Config::Set, "/NodeList/9/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps"));

    Simulator::Schedule (Seconds(8), Config::Set, "/ChannelList/5/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(8), Config::Set, "/ChannelList/6/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(8), Config::Set, "/ChannelList/7/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(8), Config::Set, "/ChannelList/8/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(8), Config::Set, "/ChannelList/9/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(5)));
    Simulator::Schedule (Seconds(8), Config::Set, "/NodeList/5/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("10Mbps"));
    Simulator::Schedule (Seconds(8), Config::Set, "/NodeList/6/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("10Mbps"));
    Simulator::Schedule (Seconds(8), Config::Set, "/NodeList/7/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("10Mbps"));
    Simulator::Schedule (Seconds(8), Config::Set, "/NodeList/8/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("10Mbps"));
    Simulator::Schedule (Seconds(8), Config::Set, "/NodeList/9/DeviceList/0/$ns3::PointToPointNetDevice/DataRate", StringValue("10Mbps"));
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
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
    if(config == 0 || config == 4)
      p->SetNeat(ntw,NULL);
    if(config == 1 || config == 2)
      p->SetNeat(neat_18,ntw);
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
// nodeAD.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
// nodeBE.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
// nodeDF.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
// nodeEG.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeFH.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeFI.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeGI.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
 nodeGJ.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

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
  
  //monitor->SerializeToXmlFile ("results8.xml" , true, true );
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
  mptcp = (sink1->GetTotalRx ()*8)/(1000000*(time-2));
  //if(sink_2->GetTotalRx () > 0)
  cbd = (sink_2->GetTotalRx ()*8)/(1000000*(time-2));
  //if(sink_3->GetTotalRx () > 0)
  cad = (sink_3->GetTotalRx ()*8)/(1000000*(time-2));
  double nominator = pow(mptcp + cbd + cad, 2);
  double denominator = 4* (pow(mptcp/2,2)+pow(mptcp/2,2)+pow(cbd,2)+ pow(cad,2)); 
  std::vector<double> results;
  results.push_back(mptcp+cbd+cad);
  results.push_back((meanDelayMp+meanDelayCad+meanDelayCbd)/3);
  results.push_back(nominator/denominator);
  return results;

}

