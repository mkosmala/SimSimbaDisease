#ifndef ENGINE_H
#define ENGINE_H

#include "stdafx.h"
#include "Basics.h"

using namespace std;
namespace SimSimba {

	class Engine {
	public:
		/**
		Constructor
		@param pname name of the parameter file
		@param fname name of the input population file
		@param steps number of steps (half-years) to run each simulation
		@param outpop name of the output population file
		@param fstat name of the output statistics file
		@param repl number of replicates (simulations) to run
		@param trans whether or not to create a transcript of the simulation (warning: large!)
		*/
		Engine(string pname,string fname,int steps,string outpop,string fstat,int repl,bool trans) { 
			paramname = pname;
			filename = fname; 
			timesteps = steps;
			outpopname = outpop;
			statsname = fstat;
			replicates = repl;
			transcript = trans;
		}
		~Engine();

		void Run();

	private:
		string paramname;
		string filename;
		int timesteps;
		string outpopname;
		string statsname;
		int replicates;
		bool transcript;

		Param param;
		fstream stats;
		fstream stats2;

		void OneSimulation();
	};
}
#endif

