#ifndef CROSSFLOWS_EVAL_H
#define CROSSFLOWS_EVAL_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/stats-module.h"
#include "../Neat/network.h"
#include "ns3/wifi-module.h"

#include <iostream>
using namespace ns3;


std::vector<double> Eval (int runNum, bool neat, NEAT::Network* ntw, bool meta);


#endif
