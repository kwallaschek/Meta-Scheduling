#include "crossflows_eval.h"
#include "triple_flow_test.h"
#include "dumbbell_crossflows_eval.h"
#include "../Neat/network.h"
#include "../Neat/nnode.h"




#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("evaluation");
void triple()
{
	std::ofstream outFile("Triple_MinRtt_50_results");
	
	int numSimulations = 10;
	int counter = 0;
	
	outFile<<"MinRtt_50:\n";
	outFile<<"run Thr Delay lstPkts";

	for(int i = 0; i<numSimulations/10; i++)
	for(int run = 0; run < 10; run++){
		std::cout<<"\r Start Simulation "<< counter << "/"<<numSimulations<<std::flush;
		counter++;
		std::vector<double> p;
		if(counter!=39)
			p = Eval_Triple (run);
 		outFile<< "\n" << counter << ": ";
 		for (const auto &e : p) outFile << e << " ";
	} 
	std::cout<<"Done"<<std::endl;
}

void crossflows(bool neat, NEAT::Network* ntw)
{
	int numSimulations = 10;

	std::ofstream outFile0("aa_neat_conf0.txt");
	outFile0<<"neat 0\n";
	outFile0<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile1("aa_neat_conf1_3.txt");
	outFile1<<"neat 1\n";
	outFile1<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile2("aa_neat_conf2_3.txt");
	outFile2<<"neat 2\n";
	outFile2<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile3("aa_neat_confc_3.txt");
	outFile3<<"neat c\n";
	outFile3<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	for(int run = 0; run < numSimulations; run++){
		std::cout<<"\r Start Simulation "<< run << "/"<<numSimulations<<std::flush;
		std::vector<double> p;
		if(run!=39){
			// Config 0
			//p = Eval (1, neat, ntw, false);
 			outFile0<< "\n" << 1 << " ";
 			for (const auto &e : p) outFile0 << e << " ";
 			// Config 1
 			p = Eval_Dumbbell (1, neat, ntw, 1,false);
 			outFile1<< "\n" << 1 << " ";
 			for (const auto &e : p) outFile1 << e << " ";
 			// Config 2
 			p = Eval_Dumbbell (1, neat, ntw, 2, false);
 			outFile2<< "\n" << 1 << " ";
 			for (const auto &e : p) outFile2 << e << " ";
 			// Config 3
 			p = Eval_Dumbbell (1, neat, ntw , 3, false);
 			outFile3<< "\n" << 1 << " ";
 			for (const auto &e : p) outFile3 << e << " ";
		}
 		
	} 
		std::cout<<"Done"<<std::endl;
}

void meta(NEAT::Network* ntw)
{
	int numSimulations = 100;

	std::ofstream outFile0("aa_meta_conf0.txt");
	outFile0<<"meta0\n";
	outFile0<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile1("aa_meta_conf1.txt");
	outFile1<<"meta1\n";
	outFile1<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile2("aa_meta_conf2.txt");
	outFile2<<"meta2\n";
	outFile2<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile3("aa_meta_confc.txt");
	outFile3<<"meta_c\n";
	outFile3<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	for(int run = 0; run < numSimulations; run++){
		std::cout<<"\r Start Simulation "<< run << "/"<<numSimulations<<std::flush;
		std::vector<double> p;
		if(run!=39){
			// Config 0
			p = Eval (run, false, ntw, true);
 			outFile0<< "\n" << run << " ";
 			for (const auto &e : p) outFile0 << e << " ";
 			// Config 1
 			p = Eval_Dumbbell (run, false, ntw, 1, true);
 			outFile1<< "\n" << run << " ";
 			for (const auto &e : p) outFile1 << e << " ";
 			// Config 2
 			p = Eval_Dumbbell (run, false, ntw , 2, true);
 			outFile2<< "\n" << run << " ";
 			for (const auto &e : p) outFile2 << e << " ";
 			// Config 3
 			p = Eval_Dumbbell (run, false, ntw , 3, true);
 			outFile3<< "\n" << run << " ";
 			for (const auto &e : p) outFile3 << e << " ";
		}
 		
	} 
}
int main(int argc, char *argv[]){

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
	NEAT::Network* ntw = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);

	/** Initialize META NEAT-Network**/
	/*NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
	NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
	NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
	NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::SENSOR, 4, NEAT::nodeplace::INPUT);
	NEAT::NNode* node5 = new NEAT::NNode(NEAT::nodetype::SENSOR, 5, NEAT::nodeplace::INPUT);
	NEAT::NNode* node13 = new NEAT::NNode(NEAT::nodetype::NEURON, 13, NEAT::nodeplace::INPUT);
	NEAT::NNode* node14 = new NEAT::NNode(NEAT::nodetype::NEURON, 14, NEAT::nodeplace::INPUT);
	NEAT::NNode* node15 = new NEAT::NNode(NEAT::nodetype::NEURON, 15, NEAT::nodeplace::INPUT);
	NEAT::NNode* node20 = new NEAT::NNode(NEAT::nodetype::NEURON, 20, NEAT::nodeplace::OUTPUT);
	

	
	// Add Links to Nodes
	node13->add_incoming(node1,  -1.40435);
	//node13->add_incoming(node2, -1.78895);
	node13->add_incoming(node3,  0.143054);
	node13->add_incoming(node4, -0.932684);
	node13->add_incoming(node5, 2.20985);
	node14->add_incoming(node1, -1.34639);
	node14->add_incoming(node2,-2.11967);
	node14->add_incoming(node3, 0.0146001);
	node14->add_incoming(node4, 0.501258);
	node14->add_incoming(node5,-4.09365);
	node15->add_incoming(node1, -0.487846);
	node15->add_incoming(node2,  0.552143);
	node15->add_incoming(node3,  0.225703);
	node15->add_incoming(node4, -0.338482);
	node15->add_incoming(node5,  0.144472);
	node20->add_incoming(node2, -1.07554);
	node13->add_incoming(node20, 0.868377);
	node20->add_incoming(node3, 1.66999);
	node20->add_incoming(node15, 0.723105);
	

	std::vector<NEAT::NNode*> allNodes;
	std::vector<NEAT::NNode*> inputNodes;
	std::vector<NEAT::NNode*> outputNodes;

	// Fill Arrays with the corresponding Nodes
	allNodes.push_back(node1);
	allNodes.push_back(node2);
	allNodes.push_back(node3);
	allNodes.push_back(node4);
	allNodes.push_back(node5);
	allNodes.push_back(node13);
	allNodes.push_back(node14);
	allNodes.push_back(node15);
	allNodes.push_back(node20);

	inputNodes.push_back(node1);
	inputNodes.push_back(node2);
	inputNodes.push_back(node3);
	inputNodes.push_back(node4);
	inputNodes.push_back(node5);

	outputNodes.push_back(node13);
	outputNodes.push_back(node14);
	outputNodes.push_back(node15);

	// Build Network
	NEAT::Network* meta_ntw = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);
	*/
	//triple();
	crossflows(false, ntw);
	//meta(meta_ntw);
}

