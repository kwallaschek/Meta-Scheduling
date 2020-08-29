# Translator
# 
# Translates NEAT Neural Network into ProgMP Schedulers
#


import csv
import argparse


class Node(object):
	"""docstring for Node"""
	def __init__(self, num, outNode):
		super(Node, self).__init__()
		self.num = num
		self.incoming = []

		
class Link(object):
	"""docstring for Link"""
	def __init__(self, outNode, inNode, weight):
		super(Link, self).__init__()
		self.inN = None
		self.outN = None
		self.outNode = outNode
		self.inNode = inNode
		self.weight = weight
		self.recursive = False
		if (outNode == inNode):
			self.recursive = True
			addRecursive(outNode)



class Net(object):

	inputNodes = []
	outputNodes = []
	hiddenNodes = []
	links = []
	"""docstring for Net"""
	def __init__(self):
		super(Net, self).__init__()
	

"""Connect the Network

Connects all nodes with the corresponding links
"""
def build(n):
	# Add all input nodes first
	for link in n.links:
		for node in n.inputNodes:
			if (link.inNode == node.num):
				link.inN = node
		for node in n.outputNodes:
			if (link.inNode == node.num):
				link.inN = node
		for node in n.hiddenNodes:
			if (link.inNode == node.num):
				link.inN = node
	# Then add the output nodes and directly reference them in the nodes themselves
	# Note: inputNodes are not necessary here since they have no input from links
	for link in n.links:
		for node in n.outputNodes:
			if (link.outNode == node.num):
				link.outN = node
			#	print(str(node.num) + ": " +str(link.inN.num))
				node.incoming.append((link.inN,link))
		for node in n.hiddenNodes:
			if (link.outNode == node.num):
				link.outN = node
			#	print(str(node.num) + ": " +str(link.inN.num))
				node.incoming.append((link.inN,link))
	



net = Net()
recursiveRegister = {}

def addRecursive(num):
	if len(recursiveRegister) == 0:
		recursiveRegister[num] = "R1"
	elif len(recursiveRegister) == 1:
		recursiveRegister[num] = "R5"
	else:
		raise ValueError('Too Many Recursive Links for ProgMP')


"""Prints sensor nodes

Instead of using a Variable for a sensor node,
this function prints the input itself
"""
def printSensorNodes(num, model):
	if model == 'single':
		dictionary={
			1: "sbf.LOST_SKBS",
			2: "sbf.RTT/8000",
			3: "sbf.RTT_VAR/8000",
			4: "sbf.CWND"
		}
		out.write(dictionary.get(num, "error"))
		return
	if model == 'meta':
		dictionary={
			1: "SUBFLOWS.MIN(sbf => sbf.RTT).RTT/SUBFLOWS.MAX(sbf => sbf.RTT).RTT",
			2: "SUBFLOWS.MIN(sbf => sbf.RTT_VAR).RTT_VAR/SUBFLOWS.MAX(sbf => sbf.RTT_VAR).RTT_VAR",
			3: "SUBFLOWS.MIN(sbf => sbf.CWND).CWND/SUBFLOWS.MAX(sbf => sbf.CWND).CWND",
			4: "SUBFLOWS.COUNT"
		}
		out.write(dictionary.get(num, "error"))
		return
	else:
		# There is no n0 so this works
		if num%4 == 0:
			out.write("SUBFLOWS.GET({}).CWND".format(num/4 - 1))
		else:
			if num%4 == 1:
				out.write("SUBFLOWS.GET({}).LOST_SKBS".format(num//4))
			if num%4 == 2:
				out.write("SUBFLOWS.GET({}).RTT/8000".format(num//4))
			if num%4 == 3:
				out.write("SUBFLOWS.GET({}).RTT_VAR/8000".format(num//4))




def printNet(out, model):
	visited = net.inputNodes[:]
	indent = '\t'
	outnode = []
	if (model == 'single'):
		indent = '\t\t'
		out.write("\tFOREACH (VAR sbf IN SUBFLOWS) {\n")


	# Print hidden and output nodes
	while (not all(node in visited for node in net.hiddenNodes) or not all(node in visited for node in net.outputNodes)):
		# Hidden nodes
		notYetPrinted = [n for n in net.hiddenNodes if n not in visited]
		readyToBePrinted = [n for n in notYetPrinted if all(node in visited for node in [i[0] for i in n.incoming if i[0].num != n.num])]
		for n in readyToBePrinted:
			# Variable to check if we have to set Register for a recursive link
			recursive = False
			toPrint = None
			# Create two lists of input: one with positive weights and one with negative weights
			pos = []
			neg = []
			for s in n.incoming:
				if (s[1].weight>0):
					pos.append(s)
				else:
					neg.append(s)
			# Positive and Negative weights
			if len(neg) > 0 and len(pos) > 0:
				out.write("{}VAR pos{} = ".format(indent,n.num))
				first = True
				for t in pos:
					if first:
						if t[0] in net.inputNodes:
							printSensorNodes(t[0].num, model)
							out.write(" * {}".format(t[1].weight))
						elif t[1].recursive:
							recursive = True
							toPrint = recursiveRegister.get(n.num)
							out.write("{} * {}".format(toPrint, t[1].weight))
						else:
							out.write("n{} * {}".format(t[0].num, t[1].weight))
						first = False
					else:
						if t[0] in net.inputNodes:
							out.write(" + ")
							printSensorNodes(t[0].num, model)
							out.write(" * {}".format(t[1].weight))
						elif t[1].recursive:
							recursive = True
							toPrint = recursiveRegister.get(n.num)
							out.write(" + {} * {}".format(toPrint, t[1].weight))
						else:
							out.write(" + n{} * {}".format(t[0].num, t[1].weight))
				out.write("; \n")
				out.write("{}VAR neg{} = ".format(indent,n.num))
				first = True
				for t in neg:
					if first:
						if t[0] in net.inputNodes:
							printSensorNodes(t[0].num, model)
							out.write(" * {}".format(t[1].weight*(-1)))
						elif t[1].recursive:
							recursive = True
							toPrint = recursiveRegister.get(n.num)
							out.write("{} * {}".format(toPrint, t[1].weight*(-1)))
						else:
							out.write("n{} * {}".format(t[0].num, t[1].weight*(-1)))
						first = False
					else:
						if t[0] in net.inputNodes:
							out.write(" + ")
							printSensorNodes(t[0].num, model)
							out.write(" * {}".format(t[1].weight*(-1)))
						elif t[1].recursive:
							recursive = True
							toPrint = recursiveRegister.get(n.num)
							out.write(" + {} * {}".format(toPrint, t[1].weight*(-1)))
						else:
							out.write(" + n{} * {}".format(t[0].num, t[1].weight*(-1)))
				out.write("; \n")
				out.write("{}IF (neg{} > pos{}) {{".format(indent,n.num, n.num))
				out.write("\n{}\tSET(R3,0);\n".format(indent))
				out.write("{}}} ELSE {{ \n".format(indent))
				out.write("{}\tSET(R3, pos{} - neg{});".format(indent,n.num,n.num))
				out.write("\n{}}}\n".format(indent))
				out.write("{}VAR n{} = R3;\n".format(indent,n.num))
				if recursive:
					out.write("{}SET({},n{});\n".format(indent,toPrint,n.num))
					recursive = False
				out.write("\n")
			# Only negative weights -> can only be 0
			elif len(pos) == 0:
				out.write("{}VAR n{} = 0;\n\n".format(indent,n.num))
			# Only positive weights -> sum of positive weighted nodes
			elif len(neg) == 0:
				out.write("{}VAR n{} = ".format(indent,n.num))
				first = True
				for t in pos:
					if first:
						if t[0] in net.inputNodes:
							printSensorNodes(t[0].num, model)
							out.write(" * {}".format(t[1].weight))
						elif t[1].recursive:
							recursive = True
							toPrint = recursiveRegister.get(n.num)
							out.write("{} * {}".format(toPrint, t[1].weight))
						else:
							out.write("n{} * {} ".format(t[0].num, t[1].weight))
						first = False
					else:
						if t[0] in net.inputNodes:
							out.write(" + ")
							printSensorNodes(t[0].num, model)
							out.write(" * {}".format(t[1].weight))
						elif t[1].recursive:
							recursive = True
							toPrint = recursiveRegister.get(n.num)
							out.write("{} * {}".format(toPrint, t[1].weight))
						else:
							out.write("+ n{} * {}".format(t[0].num, t[1].weight))
				out.write(";\n")
				if recursive:
					out.write("{}SET({},n{});\n".format(indent,toPrint,n.num))
					recursive = False
				out.write("\n")
			visited.append(n)
		# Output nodes
		print(len(net.outputNodes))
		notYetPrinted = [n for n in net.outputNodes if n not in visited]
		print(len(notYetPrinted))
		while len(outnode) < len(notYetPrinted):
			readyToBePrinted = [n for n in notYetPrinted if all(node in visited for node in [i[0] for i in n.incoming if i[0].num != n.num]) and not(n in visited)]
			for n in readyToBePrinted:
				outnode.append(n)
				# Variable to check if we have to set Register for a recursive link
				recursive = False
				toPrint = None
				# Create two lists of input: one with positive weights and one with negative weights
				pos = []
				neg = []
				for s in n.incoming:
					if (s[1].weight>0):
						pos.append(s)
					else:
						neg.append(s)
				# Positive and Negative weights
				if len(neg) > 0 and len(pos) > 0:
					out.write("{}VAR pos{} = ".format(indent,n.num))
					first = True
					for t in pos:
						if first:
							if t[0] in net.inputNodes:
								printSensorNodes(t[0].num, model)
								out.write(" * {}".format(t[1].weight))
							elif t[1].recursive:
								recursive = True
								toPrint = recursiveRegister.get(n.num)
								out.write("{} * {}".format(toPrint, t[1].weight))
							else:
								out.write("n{} * {}".format(t[0].num, t[1].weight))
							first = False
						else:
							if t[0] in net.inputNodes:
								out.write(" + ")
								printSensorNodes(t[0].num, model)
								out.write(" * {}".format(t[1].weight))
							elif t[1].recursive:
								recursive = True
								toPrint = recursiveRegister.get(n.num)
								out.write(" + {} * {}".format(toPrint, t[1].weight))
							else:
								out.write(" + n{} * {}".format(t[0].num, t[1].weight))
					out.write("; \n")
					out.write("{}VAR neg{} = ".format(indent,n.num))
					first = True
					for t in neg:
						if first:
							if t[0] in net.inputNodes:
								printSensorNodes(t[0].num, model)
								out.write(" * {}".format(t[1].weight*(-1)))
							elif t[1].recursive:
								recursive = True
								toPrint = recursiveRegister.get(n.num)
								out.write("{} * {}".format(toPrint, t[1].weight*(-1)))
							else:
								out.write("n{} * {}".format(t[0].num, t[1].weight*(-1)))
							first = False
						else:
							if t[0] in net.inputNodes:
								out.write(" + ")
								printSensorNodes(t[0].num, model)
								out.write(" * {}".format(t[1].weight*(-1)))
							elif t[1].recursive:
								recursive = True
								toPrint = recursiveRegister.get(n.num)
								out.write(" + {} * {}".format(toPrint, t[1].weight*(-1)))
							else:
								out.write(" + n{} * {}".format(t[0].num, t[1].weight*(-1)))
					out.write("; \n")
					out.write("{}IF (neg{} > pos{}) {{".format(indent,n.num, n.num))
					out.write("\n{}\tSET(R3,0);\n".format(indent))
					out.write("{}}} ELSE {{ \n".format(indent))
					out.write("{}\tSET(R3, pos{} - neg{});".format(indent,n.num,n.num))
					out.write("\n{}}}\n".format(indent))
					out.write("{}VAR n{} = R3;\n".format(indent,n.num))
					if recursive:
						out.write("{}SET({},n{});\n".format(indent,toPrint,n.num))
						recursive = False
					out.write("\n")
				# Only negative weights -> can only be 0
				elif len(pos) == 0:
					out.write("{}VAR n{} = 0;\n\n".format(indent,n.num))
				# Only positive weights -> sum of positive weighted nodes
				elif len(neg) == 0:
					out.write("{}VAR n{} = ".format(indent,n.num))
					first = True
					for t in pos:
						if first:
							if t[0] in net.inputNodes:
								printSensorNodes(t[0].num, model)
								out.write(" * {}".format(t[1].weight))
							elif t[1].recursive:
								recursive = True
								toPrint = recursiveRegister.get(n.num)
								out.write("{} * {}".format(toPrint, t[1].weight))
							else:
								out.write("n{} * {} ".format(t[0].num, t[1].weight))
							first = False
						else:
							if t[0] in net.inputNodes:
								out.write(" + ")
								printSensorNodes(t[0].num, model)
								out.write(" * {}".format(t[1].weight))
							elif t[1].recursive:
								recursive = True
								toPrint = recursiveRegister.get(n.num)
								out.write("{} * {}".format(toPrint, t[1].weight))
							else:
								out.write("+ n{} * {}".format(t[0].num, t[1].weight))
					out.write(";\n")
					if recursive:
						out.write("{}SET({},n{});\n".format(indent,toPrint,n.num))
						recursive = False
						out.write("\n")
				visited.append(n)
	# Print switching
		print(len(outnode))
		if model == 'meta':
			out.write("\tIF (n{}>n{}){{\n".format(outnode[0].num,outnode[1].num))
			out.write("\t\tSET(R6,SUBFLOWS.FILTER(sbf => sbf.HAS_WINDOW_FOR(Q.TOP)).MIN(sbf => sbf.RTT).ID);\n")
			out.write("\t\tSUBFLOWS.GET(R6).PUSH(Q.POP());\n")
			out.write("\t} ELSE {\n")	
			out.write("\t\tSET(R6, R6+1);\n")
			out.write("\t\tIF (R6+1 > SUBFLOWS.COUNT){\n")
			out.write("\t\t\tSET(R6,0);\n \t\t}\n")
			out.write("\t\tSUBFLOWS.GET(R6).PUSH(Q.POP());\n")
			out.write("\t}\n")	
		else:	
			out.write("\tIF (n{}>n{}){{\n".format(outnode[0].num,outnode[1].num))
			out.write("\t\tIF (n{}>n{}){{\n".format(outnode[0].num,outnode[2].num))
			out.write("\t\t\tSUBFLOWS.GET(0).PUSH(Q.POP());\n")
			out.write("\t\t} ELSE {\n")	
			out.write("\t\t\tSUBFLOWS.GET(2).PUSH(Q.POP());\n")
			out.write("\t\t}\n")
			out.write("\t} ELSE {\n")	
			out.write("\t\tIF (n{}>n{}){{\n".format(outnode[1].num,outnode[2].num))
			out.write("\t\t\tSUBFLOWS.GET(1).PUSH(Q.POP());\n")
			out.write("\t\t} ELSE {\n")	
			out.write("\t\t\tSUBFLOWS.GET(2).PUSH(Q.POP());\n")
			out.write("\t\t}\n")
			out.write("\t}\n")





if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument("-nn-f", "--nnFile", help="The file which contains the NEAT Neural Network", required=True)
	parser.add_argument("-out-f", "--outFile", help="The output file name for the ProgMP Scheduler", required=True)
	parser.add_argument("-model", "--model", help="The model of the NN (single/double/triple)", required=True)

	args = parser.parse_args()
	
	with open(args.nnFile, "r") as file:
		readCSV = csv.reader(file, delimiter=" ", quotechar=None)

		for row in readCSV:
			if (row[0] == 'node' and row[4] == '1'):
				net.inputNodes.append(Node(int(row[1]), False))
			if (row[0] == 'node' and row[4] == '2'):
				net.outputNodes.append(Node(int(row[1]), True))
			if (row[0] == 'node' and row[4] == '0'):
				net.hiddenNodes.append(Node(int(row[1]), False))
			if (row[0] == 'gene' and row[8] == '1'):
				net.links.append(Link(int(row[3]), int(row[2]), int(float(row[4])*100)))

	if (len(net.outputNodes) + len(net.hiddenNodes) > 22):
		print("Too many nodes! ProgMP is not capable of using that many variables")
		exit()

	build(net)

	with open(args.outFile, "w") as out:
		out.write("SCHEDULER {};\n".format(args.outFile[:-7]))
		out.write("\n")
		out.write("IF (R5 == 4){\n")
		out.write("\t SET(R5, 0);\n")
		out.write("}\n")
		out.write("IF (R6 == 5){\n")
		out.write("\t SET(R6, 0);\n")
		out.write("}\n")
		out.write("SET(R2,0);\n")
		out.write("SET(R3,0);\n")
		out.write("SET(R4,0);\n\n\n")
		# Retransmissions
		out.write("VAR sbfCandidates = SUBFLOWS.FILTER(sbf => sbf.CWND > sbf.SKBS_IN_FLIGHT + sbf.QUEUED AND !sbf.THROTTLED AND !sbf.LOSSY);\n\n")
		out.write("IF (!RQ.EMPTY) {\n")
		out.write("		IF (!SUBFLOWS.FILTER(sbf => !RQ.TOP.SENT_ON(sbf)).EMPTY) {\n")
		out.write("   		DROP( RQ.POP());\n")
		out.write("    		RETURN;}\n")
		out.write("		ELSE {\n")
		out.write("    		VAR retransmissionCandidates = sbfCandidates.FILTER(sbf => sbf.HAS_WINDOW_FOR(RQ.TOP) AND !RQ.TOP.SENT_ON(sbf));\n")
		out.write("    		IF(!retransmissionCandidates.EMPTY) {\n")
		out.write("        		FOREACH(VAR sbf IN retransmissionCandidates) {\n")
		out.write("         		sbf.PUSH(RQ.TOP);\n")
		out.write("				}\n")
		out.write("				RETURN;\n")
		out.write("    		}\n")
		out.write("		}\n")
		out.write("}\n\n\n")
		out.write("IF (!Q.EMPTY) {\n")
		printNet(out,args.model)
		out.write("}")













