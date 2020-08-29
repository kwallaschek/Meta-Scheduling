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
  //Simulator::Schedule (Seconds(1), Config::Set, "/ChannelList/3/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(20 + (rand() % static_cast<int>(100 - 20 + 1)))));
  //Simulator::Schedule (Seconds(1), Config::Set, "/ChannelList/1/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(20 + (rand() % static_cast<int>(100 - 20 + 1)))));
  Simulator::Schedule (Seconds (0.1), &GetRttVar);
}

int
main (int argc, char *argv[])
{

  /* Init NEAT-NN */
  NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
  NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
  NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
  NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::NEURON, 4, NEAT::nodeplace::OUTPUT);
  NEAT::NNode* node5 = new NEAT::NNode(NEAT::nodetype::NEURON, 5, NEAT::nodeplace::HIDDEN);

  node4->add_incoming(node1, -0.75);
  node4->add_incoming(node2, 0.04);
  node4->add_incoming(node5, 2,7);
  node5->add_incoming(node3, 0.71);

  std::vector<NEAT::NNode*> allNodes;
  std::vector<NEAT::NNode*> inputNodes;
  std::vector<NEAT::NNode*> outputNodes;

  allNodes.push_back(node1);
  allNodes.push_back(node2);
  allNodes.push_back(node3);
  allNodes.push_back(node4);
  allNodes.push_back(node5);

  inputNodes.push_back(node1);
  inputNodes.push_back(node2);
  inputNodes.push_back(node3);

  outputNodes.push_back(node4);

  // Build Network
  NEAT::Network* ntw = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);

  bool tracing = false;
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
  Config::SetDefault("ns3::MpTcpSocketBase::MaxSubflows", UintegerValue(2)); // Sink
//  Config::SetDefault("ns3::MpTcpBulkSendApplication::Neat", BooleanValue(true));
  Config::SetDefault("ns3::MpTcpSocketBase::SchedulingAlgorithm", StringValue("Min_Rtt"));
//
// Allow the user to override any of the defaults at
// run-time, via command-line arguments
//
  CommandLine cmd;
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("maxBytes",
                "Total number of bytes for application to send", maxBytes);
  cmd.Parse (argc, argv);

//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (4);

//      c h1
//     / \
// s0 a---b s1
//     \ /
//      d h2

  NodeContainer ab = NodeContainer(nodes.Get(0), nodes.Get(1));
  NodeContainer ac = NodeContainer(nodes.Get(0), nodes.Get(2));
  NodeContainer cb = NodeContainer(nodes.Get(2), nodes.Get(1));

  NodeContainer ad = NodeContainer(nodes.Get(0), nodes.Get(3));
  NodeContainer bd = NodeContainer(nodes.Get(1), nodes.Get(3));

  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint;
//  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
 // pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));

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
  nodeCB = pointToPoint.Install(cb);
  nodeAD = pointToPoint.Install(ad);
  nodeBD = pointToPoint.Install(bd);
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
  Ipv4InterfaceContainer iAB = ipv4.Assign (devices);
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


  MpTcpBulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (iAD.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  sourceApps = source.Install (nodes.Get (2));
  ApplicationContainer::Iterator j;
  for (j = sourceApps.Begin(); j != sourceApps.End(); ++j)
  {
    Ptr<MpTcpBulkSendApplication> p = DynamicCast<MpTcpBulkSendApplication> (*j);
   // p->SetNeat(ntw);
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

Simulator::Schedule (Seconds (1), &GetRttVar);

//
// Set up tracing if enabled
//
  if (tracing)
    {
      AsciiTraceHelper ascii;
      pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
      pointToPoint.EnablePcapAll ("tcp-bulk-send", false);
    }

/**
 * 

Similarly for the Channel use the config path
"/ChannelList/[i]/$ns3::PointToPointChannel/Delay"
 */
  if (config == 1) {
    Simulator::Schedule (Seconds(10.0), Config::Set, "/ChannelList/3/$ns3::PointToPointChannel/Delay", TimeValue (MilliSeconds(10)));
    Config::Set("/NodeList/3/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("50Mbps") );
  }
//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  /*ApplicationContainer::Iterator i;
  for (i = sinkApps.Begin(); i != sinkApps.End(); ++i)
  {
    Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (*i);
    std::cout << sink1 -> GetTotalRx() << std::endl;
  }
  */
  Ptr<MpTcpPacketSink> sink1 = DynamicCast<MpTcpPacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx ()/(10*1000000)<<" Mbps" << std::endl;
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  
}
