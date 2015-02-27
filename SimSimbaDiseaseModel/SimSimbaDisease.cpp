// SimSimbaDisease.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Engine.h"

using namespace SimSimba;

// syntax: simsimba2007 inpop timesteps [-pparam] [-ooutpop] [-sstats] [-rreplicates] [-t]
// inpop = input population file
// param = parameters file; if not specified = "Parameters.txt"
// outpop = output population file
// stats = statistics file
// timesteps = number of timesteps
// replicates = number of replicates (default 1)
// -t = write to transcript file

int main(int argc, char* argv[])
{

	// seed the random number generator
	time_t t = time(0);
	srand((unsigned)t); 

	// get additional command line args
	if (argc<3) {
		cout << "simsimba inpop timesteps [-pparam] [-ooutpop] [-sstats] [-rreplicates] [-t]\n";
		return 0;
	}

	string infile(argv[1]);
	int steps;

	steps = atoi(argv[2]);

	int i;
	string outpop,stats,paramfile;
	int repl=1;
	bool trans = false;
	for (i=3;i<argc;i++) {
		string a(argv[i]);

		if (a.length()<2 || a[0]!='-' || 
			(a[1]!='o' && a[1]!='s' && a[1]!='r' && a[1]!='t' && a[1]!='p')) {
			cout << "simsimba inpop timesteps [-pparam] [-ooutpop] [-sstats] [-rreplicates] [-t]\n";
			return 0;
		}
		if ((a[1]=='o' || a[1]=='s' || a[1]=='r' || a[1]=='p') && a.length()<3) {
			cout << "simsimba inpop timesteps [-pparam] [-ooutpop] [-sstats] [-rreplicates] [-t]\n";
			return 0;
		}

		if (a[1]=='o')
			outpop = a.substr(2);
		else if (a[1]=='s')
			stats = a.substr(2);
		else if (a[1]=='r') 
			repl = atoi((a.substr(2)).c_str());
		else if (a[1]=='t')
			trans = true;
		else if (a[1]=='p')
			paramfile = a.substr(2);
	}

	// should pass in the name of the file to load at start up
	Engine* eng = new Engine(paramfile,infile,steps,outpop,stats,repl,trans);
	eng->Run();
	delete eng;

	return 0;
}

