//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "../GeneticProgramingBrain/GeneticProgramingBrain.h"

//shared_ptr<ParameterLink<double>> GeneticProgramingBrain::valueMinPL = Parameters::register_parameter("BRAIN_CONSTANT-valueMin", 0.0, "Minmum value that brain will deliver");
//shared_ptr<ParameterLink<double>> GeneticProgramingBrain::valueMaxPL = Parameters::register_parameter("BRAIN_CONSTANT-valueMax", 100.0, "Maximum value that brain will deliver");
//shared_ptr<ParameterLink<int>> GeneticProgramingBrain::valueTypePL = Parameters::register_parameter("BRAIN_CONSTANT-valueType", 0, "0 = int, 1 = double");
//shared_ptr<ParameterLink<int>> GeneticProgramingBrain::samplesPerValuePL = Parameters::register_parameter("BRAIN_CONSTANT-samplesPerValue", 1, "for each brain value, this many samples will be taken from genome and averaged");

//shared_ptr<ParameterLink<bool>> GeneticProgramingBrain::initializeUniformPL = Parameters::register_parameter("BRAIN_CONSTANT-initializeUniform", false, "Initialize genome randomly, with all samples having same value");
//shared_ptr<ParameterLink<bool>> GeneticProgramingBrain::initializeConstantPL = Parameters::register_parameter("BRAIN_CONSTANT-initializeConstant", false, "If true, all values in genome will be initialized to initial constant value.");
//shared_ptr<ParameterLink<int>> GeneticProgramingBrain::initializeConstantValuePL = Parameters::register_parameter("BRAIN_CONSTANT-initializeConstantValue", 0, "If initialized constant, this value is used to initialize entire genome.");

shared_ptr<Abstract_MTree> GeneticProgramingBrain::makeTree(vector<string> nodeTypes, int depth, int maxDepth) {
	if (depth < maxDepth) {
		shared_ptr<Abstract_MTree> op = stringToMTree(nodeTypes[Random::getIndex(nodeTypes.size())]); // get a random node from nodeTypes
		vector<shared_ptr<Abstract_MTree>> branches;
		int numBranches = (op->numBranches() > 0) ? op->numBranches() : Random::getInt(2, 4);
		for (int branchCount = 0; branchCount < numBranches; branchCount++) { // determine number of branches and make a tree for each branch
			branches.push_back(makeTree(nodeTypes, depth + 1, maxDepth));
		}
		shared_ptr<Abstract_MTree> newOp = op->makeCopy(branches);
		for (auto b : newOp->branches) {
			b->parent = newOp;
		}
		return newOp;
	}
	else { // depth = maxDepth, get a CONST
		return make_shared<CONST_MTree>(Random::getDouble(1000));
	}
}
GeneticProgramingBrain::GeneticProgramingBrain(int _nrInNodes, int _nrOutNodes, shared_ptr<ParametersTable> _PT) :
		AbstractBrain(_nrInNodes, _nrOutNodes, _PT) {
	//valueMin = (PT == nullptr) ? valueMinPL->lookup() : PT->lookupDouble("BRAIN_CONSTANT-valueMin");
	//valueMax = (PT == nullptr) ? valueMaxPL->lookup() : PT->lookupDouble("BRAIN_CONSTANT-valueMax");
	//valueType = (PT == nullptr) ? valueTypePL->lookup() : PT->lookupInt("BRAIN_CONSTANT-valueType");
	//samplesPerValue = (PT == nullptr) ? samplesPerValuePL->lookup() : PT->lookupInt("BRAIN_CONSTANT-samplesPerValue");

	//initializeUniform = (PT == nullptr) ? initializeUniformPL->lookup() : PT->lookupBool("BRAIN_CONSTANT-initializeUniform");
	//initializeConstant = (PT == nullptr) ? initializeConstantPL->lookup() : PT->lookupBool("BRAIN_CONSTANT-initializeConstant");
	//initializeConstantValue = (PT == nullptr) ? initializeConstantValuePL->lookup() : PT->lookupInt("BRAIN_CONSTANT-initializeConstantValue");
	
	//cout << "init" << endl;
	
	initialTreeDepth = 3;
	nodeTypes = { "SUM","MULT","SUBTRACT","DIVIDE","SIN","COS","VECT" };

	vector<string> leafNodeTypes = { "CONST" };

	nrHiddenValues = 2;
	readFromOutputs = true;
	useHidden = true;
	nrHiddenValues = (useHidden) ? nrHiddenValues : 0;
	
	magnitudeMax = 1000000000; // values with abs > are set to this value (but keep sign)
	magnitudeMin = 0; // values with abs < are set to this value (but keep sign)

	int nrInputTotal = nrInputValues + ((readFromOutputs) ? nrOutputValues : 0) + nrHiddenValues;
	int nrOutTotal = nrOutputValues + nrHiddenValues;
	//cout << "nrInputTotal = " << nrInputTotal << "    nrOutTotal = " << nrOutTotal << endl;
	readFromValues.resize(nrInputTotal,0);
	writeToValues.resize(nrOutTotal, 0);

	for (int o = 0; o < nrOutTotal; o++) {
		// make a tree from each output in trees
		////cout << " making tree #" << o << endl;
		trees.push_back(makeTree(nodeTypes, 0, initialTreeDepth));
		//trees.back()->show();
		////cout << "---------------------------------" << endl;
		////cout << trees.back()->getFormula() << endl;
	}
// columns to be added to ave file
	popFileColumns.clear();
	popFileColumns.push_back("geneticProgramingBrainNodesCount");
	//for (int i = 0; i < nrOutputValues; i++) {
	//	popFileColumns.push_back("brainValue" + to_string(i));
	//}

	////cout << "done init" << endl;
	//int c = 0;
	//for (auto t : trees) {
	//	cout << "tree #" << c << endl;
	//	cout << "----------------------------------------------------------------" << endl;
	//	cout << t->getFormula() << endl;
	//	c++;
	//}
}


void GeneticProgramingBrain::resetBrain() {
	//cout << "in reset Brain" << endl;
	fill(inputValues.begin(), inputValues.end(), 0);
	fill(outputValues.begin(), outputValues.end(), 0);
	fill(readFromValues.begin(), readFromValues.end(), 0);
	fill(writeToValues.begin(), writeToValues.end(), 0);
}

void GeneticProgramingBrain::update() {
	//cout << "in update GP" << endl;
	int index = 0;
	//cout << "nrInputValues = " << nrInputValues << "    nrOutputValues = " << nrOutputValues << "   nrHiddenValues = " << nrHiddenValues << endl;
	//cout << "readFromValues.size() = " << readFromValues.size() << endl;
	//cout << "writeToValues.size() = " << writeToValues.size() << endl;
	//cout << "outputValues.size() = " << outputValues.size() << endl;
	for (; index < nrInputValues; index++) { // copy input values into readFromValues
		//cout << "input : " << index << " from inputValues[" << index << "] = " << inputValues[index] << endl;
		readFromValues[index] = inputValues[index];
	}
	if (readFromOutputs) { // if readFromOutputs, then add last outputs to readFromValues
		for (; index < nrInputValues + nrOutputValues; index++) {
			//cout << "out : " << index << " from  writeToValues[" << index - nrInputValues << "] = " << writeToValues[index - nrInputValues]  << endl;
			readFromValues[index] = writeToValues[index - nrInputValues];
		}
	}
	for (; index < nrInputValues + ((readFromOutputs)?nrOutputValues:0) + nrHiddenValues; index++) { // add hidden values to readFromValues
		//cout << "hidden : " << index << " from  writeToValues[" << (index - nrInputValues) + ((readFromOutputs) ? 0 : nrOutputValues) << "] = " << writeToValues[(index - nrInputValues) + ((readFromOutputs) ? 0 : nrOutputValues)] << endl;
		readFromValues[index] = writeToValues[(index - nrInputValues) + ((readFromOutputs) ? 0 : nrOutputValues)];
	}

	index = 0;
	DataMap dataMap;
	vector<vector<double>> values;
	values.push_back(readFromValues);
	//if (Global::update > 1125) {
	//		cout << "  In GeneticProgramingBrain::eval  ::  looking at values for VECT" << endl;
	//		cout << "   ";
	//		for (auto a : values) {
	//			for (auto b : a) {
	//				cout << b << " ";
	//			}
	//			cout << endl;
	//		}
	//}
	for (auto t : trees) {
		//if (Global::update > 1125) {
		//	cout << "t" << endl;
		//	cout << t->getFormula() << endl;
		//	cout << t->getFormula() << " = " << t->eval(dataMap, PT, values)[0] << endl;
		//}
		double newVal = t->eval(dataMap, PT, values)[0];
		if (abs(newVal) > magnitudeMax) {
			newVal = (newVal < 0) ? -1.0 * magnitudeMax : magnitudeMax;
		}
		if (abs(newVal) < magnitudeMin) {
			newVal = (newVal < 0) ? -1.0 * magnitudeMin : magnitudeMin;
		}
		writeToValues[index] = newVal;
		//cout << "post t" << endl;
		if (index < nrOutputValues) {
			outputValues[index] = writeToValues[index];
		}
		index++;
	}

	//cout << "done update GP" << endl;

}

shared_ptr<AbstractBrain> GeneticProgramingBrain::makeBrain(unordered_map<string, shared_ptr<AbstractGenome>>& _genomes) {
	shared_ptr<GeneticProgramingBrain> newBrain = make_shared<GeneticProgramingBrain>(nrInputValues, nrOutputValues);
	return newBrain;
}

shared_ptr<AbstractBrain> GeneticProgramingBrain::makeBrainFrom(shared_ptr<AbstractBrain> parent, unordered_map<string, shared_ptr<AbstractGenome>>& _genomes) {
	//cout << "  in makeMutatedBrainFrom" << endl;
	shared_ptr<GeneticProgramingBrain> newBrain = dynamic_pointer_cast<GeneticProgramingBrain>(parent->makeCopy());
	// select a node in the newBrain
	//count all nodes
	for (int treeIndex = 0; treeIndex < (int)trees.size(); treeIndex++){
	if (Random::P(.1)){
			vector<shared_ptr<Abstract_MTree>> nodesList;
			trees[treeIndex]->explode(trees[treeIndex], nodesList);
			// select a node in nodesList
			int pickID = Random::getIndex(nodesList.size());
			auto pickNode = nodesList[pickID];
			// mutate to a new op!

			if (pickNode->parent == nullptr) { // the root was picked
				trees[treeIndex] = stringToMTree(nodeTypes[Random::getIndex(nodeTypes.size())]); // get a random node from nodeTypes
				trees[treeIndex]->branches = pickNode->branches;
				int numBranches = (trees[treeIndex]->numBranches() > 0) ? trees[treeIndex]->numBranches() : Random::getInt(2, 4);
				while ((int)trees[treeIndex]->branches.size() < numBranches) { // we need more branches
					trees[treeIndex]->branches.push_back(makeTree(nodeTypes, 0, initialTreeDepth - 1));
				}
				while ((int)trees[treeIndex]->branches.size() > numBranches) { // we need less branches
					trees[treeIndex]->branches.pop_back();
				}
				for (auto b : trees[treeIndex]->branches) {
					b->parent = trees[treeIndex];
				}
			}
			else {
				if (pickNode->type() != "CONST") {
					shared_ptr<Abstract_MTree> op = stringToMTree(nodeTypes[Random::getIndex(nodeTypes.size())]); // get a random node from nodeTypes
					op->branches = pickNode->branches;

					int numBranches = (op->numBranches() > 0) ? op->numBranches() : Random::getInt(2, 4);
					while ((int)op->branches.size() < numBranches) { // we need more branches
						op->branches.push_back(makeTree(nodeTypes, 0, initialTreeDepth - 1));
					}
					while ((int)op->branches.size() > numBranches) { // we need less branches
						op->branches.pop_back();
					}
					for (auto b : op->branches) {
						b->parent = op;
					}
				}
				else { // this is a const node, just change value
					shared_ptr<CONST_MTree> castNode = dynamic_pointer_cast<CONST_MTree>(pickNode);
					castNode->value = Random::getDouble(1000);
				}




				//vector<shared_ptr<Abstract_MTree>> branches;
				//int numBranches = (op->numBranches() > 0) ? op->numBranches() : Random::getInt(2, 4);
				//for (int branchCount = 0; branchCount < numBranches; branchCount++) { // determine number of branches and make a tree for each branch
				//	branches.push_back(makeTree(nodeTypes, depth + 1, maxDepth));
				//}
				//return(op->makeCopy(branches));



			}

		}
	}
	return newBrain;
}

shared_ptr<AbstractBrain> GeneticProgramingBrain::makeBrainFromMany(vector<shared_ptr<AbstractBrain>> parents, unordered_map<string, shared_ptr<AbstractGenome>>& _genomes) {
	//cout << "  in makeMutatedBrainFromMany" << endl;
	return parents[0]->makeCopy();
}


string GeneticProgramingBrain::description() {
	string S = "Genetic Programing Brain";
	for (int i = 0; i < (int)trees.size(); i++) {
		S = S + "\n  formula " + to_string(i) + ": " + trees[i]->getFormula();
	}
	return S;
}

DataMap GeneticProgramingBrain::getStats(string& prefix) {
	DataMap dataMap;
	int nodesCount = 0;
	for (auto t : trees) {
		vector<shared_ptr<Abstract_MTree>> nodesList;
		t->explode(t, nodesList);
		nodesCount = (int)nodesList.size();
	}
	dataMap.Append(prefix+"geneticProgramingBrainNodesCount", nodesCount);
	return (dataMap);
}

DataMap GeneticProgramingBrain::serialize(string& name) {
	DataMap serialDataMap;
	int numFormula = (int)trees.size();
	serialDataMap.Set(name + "_numFormula", numFormula);
	string formula = "\"[";
	for (int i = 0; i < numFormula; i++) {
		formula = formula + trees[i]->getFormula() + "::";
	}
	formula += "]\"";
	serialDataMap.Set(name + "_formulas", formula);
	return serialDataMap;
}

void GeneticProgramingBrain::deserialize(shared_ptr<ParametersTable> PT, unordered_map<string, string>& orgData, string& name) {
	trees.clear();
	if (orgData.find("BRAIN_" + name + "_numFormula") == orgData.end() || orgData.find("BRAIN_" + name + "_formulas") == orgData.end()) {
		cout << "  In GeneticProgramingBrain::deserialize :: can not find either BRAIN_" + name + "_numFormula or BRAIN_" + name + "_formulas.\n  exiting" << endl;
		exit(1);
	}

	int numFormula;
	load_value(orgData["BRAIN_" + name + "_numFormula"], numFormula);
	stringstream ss(orgData["BRAIN_" + name + "_formulas"]);

	char nextChar;
	// skip first two chars and '['
	ss >> nextChar;
	// load first char
 	ss >> nextChar;
	string nextString;
	
	for (int i = 0; i < numFormula; i++) {
		nextString = "";
		while (nextChar != ':') {
			nextString += nextChar;
			ss >> nextChar;
		}
		//cout << "converting i = " << i << "  " << nextString << endl;
		trees.push_back(stringToMTree(nextString));
		ss >> nextChar; // skip second ':'
		ss >> nextChar; // load next char
	}
}


shared_ptr<AbstractBrain> GeneticProgramingBrain::makeCopy(shared_ptr<ParametersTable> _PT) {
	//cout << "   start Make Copy" << endl;
	shared_ptr<GeneticProgramingBrain> newBrain = make_shared<GeneticProgramingBrain>(nrInputValues, nrOutputValues, _PT);

	newBrain->trees.clear();
	for (auto t : trees) {
		newBrain->trees.push_back(t->makeCopy());
	}
	//cout << "   done Make Copy" << endl;
	return newBrain;
}