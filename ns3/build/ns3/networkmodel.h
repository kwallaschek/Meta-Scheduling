#ifndef NETWORKMODEL_H
#define NETWORKMODEL_H


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/stats-module.h"
#include "ns3/wifi-module.h"
#include "scratch/Neat/network.h"


#include <iostream>
using namespace ns3;


//std::vector<double> Initialize (int nodeNum, int runNum, int runTime, bool neat, NEAT::Network *ntw);
double Initialize_thr_1 (int nodeNum, int runNum, int runTime, bool neat, NEAT::Network *ntw, int config);
double Initialize_thr_2 (int runNum, NEAT::Network *ntw, int config);
std::vector<double> Initialize_fairness_1 (int runNum, NEAT::Network *ntw, int config);
std::vector<double> Initialize_fairness_dd (int runNum, NEAT::Network *ntw, int config);

#endif
