#ifndef ENGINE_H
#define ENGINE_H

#include "stdafx.h"
#include "Basics.h"

using namespace std;
namespace SimSimba {

	class Engine {
	public:
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

