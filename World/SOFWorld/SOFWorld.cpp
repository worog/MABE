//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "SOFWorld.h"

shared_ptr<ParameterLink<string>> SOFWorld::scoreMapFilenamePL = Parameters::register_parameter("WORLD_SOF-scoreMapFilename", (string)"World/SOFWorld/scoreMap_20x20_2peaks.txt", "name of file containing score map.");
shared_ptr<ParameterLink<string>> SOFWorld::groupNamePL = Parameters::register_parameter("WORLD_SOF_NAMES-groupName", (string)"root", "name of group to be evaluated");
shared_ptr<ParameterLink<string>> SOFWorld::brainNamePL = Parameters::register_parameter("WORLD_SOF_NAMES-brainName", (string)"root", "name of brains used to control organisms\nroot = use empty name space\nGROUP:: = use group name space\n\"name\" = use \"name\" namespace at root level\nGroup::\"name\" = use GROUP::\"name\" name space");


SOFWorld::SOFWorld(shared_ptr<ParametersTable> _PT) :
AbstractWorld(_PT) {
	x = y = 0;

	groupName = (PT == nullptr) ? groupNamePL->lookup() : PT->lookupString("WORLD_SOF_NAMES-groupName");
	brainName = (PT == nullptr) ? brainNamePL->lookup() : PT->lookupString("WORLD_SOF_NAMES-brainName");

	// columns to be added to ave file
	popFileColumns.clear();
	popFileColumns.push_back("score");
	popFileColumns.push_back("x");
	popFileColumns.push_back("y");
	
	ifstream scoreMap;
	string rawLine;
	int temp;
	
	string filename = (PT == nullptr) ? scoreMapFilenamePL->lookup() : PT->lookupString("WORLD_SOF-scoreMapFilename");

	scoreMap.open(filename);
	
	if(scoreMap.is_open())
	{
		getline(scoreMap, rawLine);
		std::stringstream ss(rawLine);
		ss >> x;
		ss >> y;
		
		for(int i=0;i<y;i++){
			getline(scoreMap, rawLine);
			std::stringstream ss(rawLine);
			for(int j=0;j<x;j++){
				ss >> temp;
				
				//cout << temp << endl;
				scoreMatrix.push_back(temp);
			}
			
		}//close for loop
		
	} else{
		cout << "ERROR: could not find WORLD_SOF-scoreMapFilename with value \"" << filename << "\"." << endl;
		exit(1);
	}
	
}

// score is number of outputs set to 1 (i.e. output > 0) squared
void SOFWorld::evaluateSolo(shared_ptr<Organism> org, int analyse, int visualize, int debug) {
	
	auto brain = org->brains[brainName];
	brain->resetBrain();
	brain->setInput(0,1); // give the brain a constant 1 (for wire brain)
	brain->update();
	//double score = 0.0;
	
	int local_x = (int)brain->readOutput(0);
	int local_y = (int)brain->readOutput(1);
	
	if(local_x < 0){
		local_x = 0;
	}
	if(local_x >= x){
		local_x = x-1;
	}
	
	if(local_y < 0){
		local_y = 0;
	}
	if(local_y >= y){
		local_y = y-1;
	}
	
	int key = (x*local_y) + local_x;
	
	double score = (double)scoreMatrix[key];

	if(debug){
		cout << local_x << "," << local_y << "  :  " << score << endl;
	}

	//org->score = score;
	org->dataMap.Append("score", score);
	org->dataMap.Append("x", local_x);
	org->dataMap.Append("y", local_y);
}


