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
	int numSimulations = 100;

	std::ofstream outFile0("results3/minRtt30_0.txt");
	outFile0<<"minRtt30 0\n";
	outFile0<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile1("results3/minRtt30_1.txt");
	outFile1<<"minRtt30 1\n";
	outFile1<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile2("results3/minRtt30_2.txt");
	outFile2<<"minRtt30 2\n";
	outFile2<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile3("results3/minRtt30_3.txt");
	outFile3<<"minRtt30 c\n";
	outFile3<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile4("results3/minRtt30_4.txt");
	outFile3<<"minRtt30 fc\n";
	outFile3<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	for(int run = 0; run < numSimulations; run++){
		std::cout<<"\r Start Simulation "<< run << "/"<<numSimulations<<std::flush;
		std::vector<double> p;
		if(run!=39){
			// Config 0
			p = Eval (run, neat, ntw, false);
 			outFile0<< "\n" << run << " ";
 			for (const auto &e : p) outFile0 << e << " ";
 			// Config 1
 			p = Eval_Dumbbell (run, neat, ntw, 1,false);
 			outFile1<< "\n" << run << " ";
 			for (const auto &e : p) outFile1 << e << " ";
 			// Config 2
 			p = Eval_Dumbbell (run, neat, ntw, 2, false);
 			outFile2<< "\n" << run << " ";
 			for (const auto &e : p) outFile2 << e << " ";
 			// Config 3
 			p = Eval_Dumbbell (run, neat, ntw , 3, false);
 			outFile3<< "\n" << run << " ";
 			for (const auto &e : p) outFile3 << e << " ";
 			// Config 4
 			p = Eval_Dumbbell (run, neat, ntw , 4, false);
 			outFile4<< "\n" << run << " ";
 			for (const auto &e : p) outFile4 << e << " ";
		}
 		
	} 
		std::cout<<"Done"<<std::endl;
}

void meta(NEAT::Network* ntw)
{
	int numSimulations = 100;

	std::ofstream outFile0("metaResults/meta_double2_0.txt");
	outFile0<<"meta_double_0\n";
	outFile0<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile1("metaResults/meta_double2_1.txt");
	outFile1<<"meta_double_1\n";
	outFile1<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile2("metaResults/meta_double2_2.txt");
	outFile2<<"meta_double_2\n";
	outFile2<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile3("metaResults/meta_double2_c.txt");
	outFile3<<"meta_double_c\n";
	outFile3<<"run Sum_Thr mpTcp_Thr cad_thr cbd_thr mpTcp_delay cad_delay cbd_delay avg_delay mpTcp_lstPkts cad_lstPkts cbd_lstPkts fairness";

	std::ofstream outFile4("metaResults/meta_double2_fc.txt");
	outFile3<<"meta_double_fc\n";
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
 			// Config 4
 			p = Eval_Dumbbell (run, false, ntw , 4, true);
 			outFile4<< "\n" << run << " ";
 			for (const auto &e : p) outFile4 << e << " ";
		}
 		
	} 
}
int main(int argc, char *argv[]){

	/** Initialize NEAT-Network neat_18**/
/*	NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
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
*/
	/** Initialize META NEAT-Network Double**/
	NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
	NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
	NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
	NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::SENSOR, 4, NEAT::nodeplace::INPUT);

	NEAT::NNode* node13 = new NEAT::NNode(NEAT::nodetype::NEURON, 13, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node14 = new NEAT::NNode(NEAT::nodetype::NEURON, 14, NEAT::nodeplace::OUTPUT);

	NEAT::NNode* node25 = new NEAT::NNode(NEAT::nodetype::NEURON, 25, NEAT::nodeplace::HIDDEN);
	NEAT::NNode* node30 = new NEAT::NNode(NEAT::nodetype::NEURON, 30, NEAT::nodeplace::HIDDEN);
	NEAT::NNode* node34 = new NEAT::NNode(NEAT::nodetype::NEURON, 34, NEAT::nodeplace::HIDDEN);
	
	node13->add_incoming(node2, 0.251599);
	node13->add_incoming(node4, 0.251599);
	node14->add_incoming(node1, 0.251599);
	node14->add_incoming(node2, 0.251599);
	node14->add_incoming(node3, 0.251599);
	node14->add_incoming(node13, 0.251599);
	node25->add_incoming(node3, 0.251599);
	node13->add_incoming(node25, 0.251599);
	node30->add_incoming(node4, 0.251599);
	node14->add_incoming(node30, 0.251599);
	node34->add_incoming(node1, 0.251599);
	node13->add_incoming(node34, 0.251599);
	// Fill Arrays with the corresponding Nodes
 	std::vector<NEAT::NNode*> allNodes;
	std::vector<NEAT::NNode*> inputNodes;
	std::vector<NEAT::NNode*> outputNodes;
	allNodes.push_back(node1);
	allNodes.push_back(node2);
	allNodes.push_back(node3);
	allNodes.push_back(node4);
	allNodes.push_back(node13);
	allNodes.push_back(node14);
	allNodes.push_back(node25);
	allNodes.push_back(node30);
	allNodes.push_back(node34);
	
	inputNodes.push_back(node1);
	inputNodes.push_back(node2);
	inputNodes.push_back(node3);
	inputNodes.push_back(node4);
	outputNodes.push_back(node13);
	outputNodes.push_back(node14);

	// Build Network
	NEAT::Network* meta_ntw = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);

	/** Initialize META NEAT-Network**/
/*	NEAT::NNode* node1 = new NEAT::NNode(NEAT::nodetype::SENSOR, 1, NEAT::nodeplace::INPUT);
	NEAT::NNode* node2 = new NEAT::NNode(NEAT::nodetype::SENSOR, 2, NEAT::nodeplace::INPUT);
	NEAT::NNode* node3 = new NEAT::NNode(NEAT::nodetype::SENSOR, 3, NEAT::nodeplace::INPUT);
	NEAT::NNode* node4 = new NEAT::NNode(NEAT::nodetype::SENSOR, 4, NEAT::nodeplace::INPUT);

	NEAT::NNode* node13 = new NEAT::NNode(NEAT::nodetype::NEURON, 13, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node14 = new NEAT::NNode(NEAT::nodetype::NEURON, 14, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node15 = new NEAT::NNode(NEAT::nodetype::NEURON, 15, NEAT::nodeplace::OUTPUT);

	NEAT::NNode* node17 = new NEAT::NNode(NEAT::nodetype::NEURON, 17, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node67 = new NEAT::NNode(NEAT::nodetype::NEURON, 67, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node85 = new NEAT::NNode(NEAT::nodetype::NEURON, 85, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node113 = new NEAT::NNode(NEAT::nodetype::NEURON, 113, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node201 = new NEAT::NNode(NEAT::nodetype::NEURON, 201, NEAT::nodeplace::OUTPUT);
	NEAT::NNode* node340 = new NEAT::NNode(NEAT::nodetype::NEURON, 340, NEAT::nodeplace::OUTPUT);
	

	
	// Add Links to Nodes

	node14->add_incoming(node1, 0.251599);
	node14->add_incoming(node3,0.0142729);
	node14->add_incoming(node4,0.29044);
	node15->add_incoming(node1, -0.295134);
	
	node15->add_incoming(node13, -0.119054);
	node14->add_incoming(node17, -0.520723);
	node17->add_incoming(node15, -0.141049);
	node17->add_incoming(node4, 0.0430487);

	node13->add_incoming(node67, 0.207304);
	node85->add_incoming(node1, 0.0287051);
	node17->add_incoming(node113, -0.576504);
	node85->add_incoming(node2, 0.21187);

	node113->add_incoming(node15, -0.59942);
	node67->add_incoming(node2,  0.0283757);
	node67->add_incoming(node4,  0.574815);
	
	node14->add_incoming(node113, -0.388239);
	node340->add_incoming(node15,  0.527952);
	node17->add_incoming(node340, -0.188695);

	node13->add_incoming(node113, 0.266008);
	node113->add_incoming(node1, 0.0194203);
	node67->add_incoming(node113, 0.614516);

	std::vector<NEAT::NNode*> allNodes;
	std::vector<NEAT::NNode*> inputNodes;
	std::vector<NEAT::NNode*> outputNodes;

	// Fill Arrays with the corresponding Nodes
	allNodes.push_back(node1);
	allNodes.push_back(node2);
	allNodes.push_back(node3);
	allNodes.push_back(node4);
	allNodes.push_back(node13);
	allNodes.push_back(node14);
	allNodes.push_back(node15);
	
	allNodes.push_back(node17);
	allNodes.push_back(node67);
	allNodes.push_back(node85);
	allNodes.push_back(node113);
	allNodes.push_back(node201);
	allNodes.push_back(node340);
	
	inputNodes.push_back(node1);
	inputNodes.push_back(node2);
	inputNodes.push_back(node3);
	inputNodes.push_back(node4);
	outputNodes.push_back(node13);
	outputNodes.push_back(node14);
	outputNodes.push_back(node15);

	// Build Network
	NEAT::Network* meta_ntw = new NEAT::Network(inputNodes, outputNodes, allNodes, 1);
	*/
	//triple();
	crossflows(false,NULL);
	//meta(meta_ntw);
}

