#include "stdafx.h"
#include "Engine.h"
#include "Parser.h"
#include "Population.h"

using namespace SimSimba;

Engine::~Engine() {}

void Engine::Run() {

	Parser* parser = new Parser();

	string pn = "Parameters.txt"; // default
	if (paramname!="")
		pn = paramname;			  // passed in
	param = parser->Parse(pn);

	bool dostats = true;
	if (statsname=="")
		dostats = false;
	
	if (dostats) {
		stats.open(statsname.c_str(), ios::out);
		stats2.open("deaths.csv", ios::out);
	}

	// simulations are each of X steps
	for (int i=1;i<=replicates;i++) {
		OneSimulation();
		if (dostats) {
			stats << endl;
			stats2 << endl;
		}
		cout << "replicate " << i << " complete.\n";
	}

	if (dostats) {
		stats.close();
		stats2.close();
	}
}

void Engine::OneSimulation() {

	bool dostats = true;
	if (statsname=="")
		dostats = false;

	Population* pop = new Population(filename,param,dostats,stats,stats2,transcript);
	for (int i=0;i<timesteps;i++) 
		pop->Step();

	if (outpopname!="") 
		pop->Save(outpopname);
	
	delete pop;
}

