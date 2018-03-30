/**
	Parser.cpp
	Parses the parameters file and saves the values to a Param object.
	
	@author mkosmala
*/

#include "stdafx.h"
#include "Parser.h"

using namespace SimSimba;

Parser::~Parser() {}

/**
	Parses the parameters file and saves the values in a Param object
	@param fname name of the parameters file
	@return Param object containing the parameters needed for simulation
*/
Param Parser::Parse(string fname) {

	fstream fs;
	string line,token,value;
	string::size_type eq, s1, s2;

	fs.open(fname.c_str(), ios::in);
	if (!fs.is_open()) {
		cout << "Unable to open Parameters file: " << fname << endl;
		return param;
	}
	else {    
		getline (fs,line);
		if (line != "SimSimba2007 Parameters") {
			cout << fname << " is in the wrong format" << endl;
			return param;
		}
	}

	Prefill();

	while (!fs.eof()) {
		getline(fs,line);
  
		// parse each line if it's not empty or a comment
		if (!line.empty() && line.substr(0,1)!="/") {
			eq = line.find("=",0);

			if (eq != string::npos) {

				token = line.substr(0,eq);	
				// remove trailing spaces
				s1 = token.find_last_not_of(" ");
				token = token.substr(0,s1+1);

				value = line.substr(eq+1);
				// remove leading spaces
				s2 = value.find_first_not_of(" ");
				value = value.substr(s2);

				SetGlobalVariable(token,value);
			}
			else if (line == "Fight Matrix") {
				list<string> fightnums;
				for (int i=0;i<10;i++) {
					getline(fs,line);
					fightnums.push_back(line);				
				}
				ParseFightTable(fightnums);
			}
			else {
				cout << "Syntax error in Parameters file: " << fname << endl << line << endl;
			}
		}
	}

	fs.close();

	CalculateLifeExpectancy();

	return param;
}

/**
	Sets one of the parameters for the simulation
	@param token the parameter to be set
	@param value value to which the parameter should be set
*/
void Parser::SetGlobalVariable(string token, string value) {

	if (token=="SimType") {
		if (value=="GROUP")
			param.SimulationType = LION;
		else if (value=="SOLO")
			param.SimulationType = LEOPARD;
		else
			Error(token,value);
	}

	else if (token=="Scenario")
		param.ScenarioNum = atoi(value.c_str());

	else if (token=="FemaleMax") 
		param.FemaleMax = atoi(value.c_str());

	else if (token=="MaleMax")
		param.MaleMax = atoi(value.c_str());

	else if (token=="MaxAge")
		param.MaxAge = atoi(value.c_str());

	else if (token=="FemaleReproduceAge")
		param.FemaleReproduceAge = atof(value.c_str());

	else if (token=="MaleReproduceAge")
		param.MaleReproduceAge = atof(value.c_str());

	else if (token=="Cub2Age") {
		param.CubMinAge[2] = atof(value.c_str());
		param.CubCategoriesDefined = 2;
	}

	else if (token=="Cub3Age") {
		param.CubMinAge[3] = atof(value.c_str());
		param.CubCategoriesDefined = 3;
	}
	
	else if (token=="Cub4Age") {
		param.CubMinAge[4] = atof(value.c_str());
		param.CubCategoriesDefined = 4;
	}
	
	else if (token=="SubadultMaleAge")
		param.SubadultMaleMinAge = atof(value.c_str());
	
	else if (token=="SubadultFemaleAge")
		param.SubadultFemaleMinAge = atof(value.c_str());
	
	else if (token=="AdultMaleAge")
		param.AdultMaleMinAge = atof(value.c_str());
	
	else if (token=="AdultFemaleAge")
		param.AdultFemaleMinAge = atof(value.c_str());

	else if (token=="SurviveResidentMale")
		param.SurviveResidentMale = atof(value.c_str());

	else if (token=="SurviveNomadicMale")
		param.SurviveNomadicMale = atof(value.c_str());

	else if (token=="SurviveSubadultMale")
		param.SurviveSubadultMale = atof(value.c_str());

	else if (token=="SurviveEvictedMale")
		param.SurviveEvictedMale = atof(value.c_str());

	else if (token=="SurviveSubadultFemale")
		param.SurviveSubadultFemale = atof(value.c_str());

	else if (token=="SurviveAdultFemale")
		param.SurviveAdultFemale = atof(value.c_str());

	else if (token=="SurviveHomelessFemale")
		param.SurviveHomelessFemale = atof(value.c_str());

	else if (token=="SurviveCub1")
		param.SurviveCub[1] = atof(value.c_str());

	else if (token=="SurviveCub2")
		param.SurviveCub[2] = atof(value.c_str());

	else if (token=="SurviveCub3")
		param.SurviveCub[3] = atof(value.c_str());

	else if (token=="SurviveCub4")
		param.SurviveCub[4] = atof(value.c_str());

	else if (token=="SurviveOrphan")
		param.SurviveOrphan = atof(value.c_str());

	else if (token=="SurviveTakeoverCub1")
		param.SurviveTakeoverCub[1] = atof(value.c_str());

	else if (token=="SurviveTakeoverCub2")
		param.SurviveTakeoverCub[2] = atof(value.c_str());

	else if (token=="SurviveTakeoverCub3")
		param.SurviveTakeoverCub[3] = atof(value.c_str());

	else if (token=="SurviveTakeoverCub4")
		param.SurviveTakeoverCub[4] = atof(value.c_str());

	else if (token=="SurviveDefendingFemale")
		param.SurviveDefendingFemale = atof(value.c_str());

	else if (token=="SurviveDefendingMaleWins")
		param.SurviveDefendingMaleWins = atof(value.c_str());

	else if (token=="SurviveDefendingMaleLoses")
		param.SurviveDefendingMaleLoses = atof(value.c_str());

	else if (token=="SurviveAttackingMaleLoses")
		param.SurviveAttackingMaleLoses = atof(value.c_str());

	else if (token=="SurviveAttackingMaleWins")
		param.SurviveAttackingMaleWins = atof(value.c_str());

	else if (token=="LitterSize1")
		param.LitterSize1 = atof(value.c_str());

	else if (token=="LitterSize2")
		param.LitterSize2 = atof(value.c_str());

	else if (token=="LitterSize3")
		param.LitterSize3 = atof(value.c_str());

	else if (token=="LitterSize4")
		param.LitterSize4 = atof(value.c_str());

	else if (token=="CubMale")
		param.CubMale = atof(value.c_str());

	else if (token=="CubAbandoned")
		param.CubAbandoned = atof(value.c_str());

	else if (token=="ResidentTakes") {
		istringstream iss;
		iss.str(value);
		int nummales;
		string howmany;
		int numterrs;
		double prob;
		int i;
		iss >> nummales >> howmany >> numterrs >> prob;
		if (howmany=="ONLY") {
			assert(nummales<15);
			assert(numterrs<10);
			param.ResidentTakes[nummales][numterrs] = prob;		
		}
		else if (howmany=="ORMORE") {
			for (i=nummales;i<15;i++) {
				assert(i<15);
				assert(numterrs<10);
				param.ResidentTakes[i][numterrs] = prob;		
			}
		}
		else 
			Error(token,value);
	}

	else if (token=="NomadJoins1Resident")
		param.NomadJoins1Resident = atof(value.c_str());

	else if (token=="NomadJoins1Nomad")
		param.NomadJoins1Nomad = atof(value.c_str());

	else if (token=="NomadJoins2Nomads")
		param.NomadJoins2Nomads = atof(value.c_str());

	else if (token=="NomadMoves")
		param.NomadMoves = atoi(value.c_str());

	else if (token=="SubadultMaleMoves")
		param.SubadultMaleMoves = atoi(value.c_str());

	else if (token=="FemaleMoves2")
		param.FemaleMoves2 = atof(value.c_str());

	else if (token=="FemaleMoves3")
		param.FemaleMoves3 = atof(value.c_str());

	else if (token=="FemaleMoves4")
		param.FemaleMoves4 = atof(value.c_str());

	else if (token=="FemaleMoves5")
		param.FemaleMoves5 = atof(value.c_str());

	else if (token=="TrophyMinAge") {
		param.TrophyMinAge = atof(value.c_str());
		param.TrophyMinAge1 = atof(value.c_str());
	}

	else if (token=="TrophySexError") {
		param.TrophySexError = atof(value.c_str());
		param.TrophySexError1 = atof(value.c_str());
	}

	else if (token=="TrophyQuota") {
		param.TrophyQuota = atoi(value.c_str());
		param.TrophyQuota1 = atoi(value.c_str());
	}

	else if (token=="TrophyHunt") {
		if (value=="ANNUAL") {
			param.HuntingFrequency = ANNUAL;
			param.HuntingFrequency1 = ANNUAL;
		}
		else if (value=="BIANNUAL") {
			param.HuntingFrequency = BIANNUAL;
			param.HuntingFrequency1 = BIANNUAL;
		}
		else
			Error(token,value);	
	}

	else if (token=="TrophyIgnoreQuota") {
		if (value=="TRUE") {
			param.IgnoreQuota = true;
			param.IgnoreQuota1 = true;
		}
		else if (value=="FALSE") {
			param.IgnoreQuota = false;
			param.IgnoreQuota1 = false;
		}
		else
			Error(token,value);	
	}

	else if (token=="TrophyMinAge2")
		param.TrophyMinAge2 = atof(value.c_str());

	else if (token=="TrophySexError2")
		param.TrophySexError2 = atof(value.c_str());

	else if (token=="TrophyQuota2")
		param.TrophyQuota2 = atoi(value.c_str());

	else if (token=="TrophyHunt2") {
		if (value=="ANNUAL")
			param.HuntingFrequency2 = ANNUAL;
		else if (value=="BIANNUAL")
			param.HuntingFrequency2 = BIANNUAL;
		else
			Error(token,value);	
	}

	else if (token=="TrophyIgnoreQuota2") {
		if (value=="TRUE")
			param.IgnoreQuota2 = true;
		else if (value=="FALSE")
			param.IgnoreQuota2 = false;
		else
			Error(token,value);	
	}

	else if (token=="SwitchStrategy") {
		param.SwitchHuntingStrategyAt = atof(value.c_str());
	}

	else if (token=="LandLossFraction")
		param.LandLossPercent = atof(value.c_str());

	else if (token=="LandLossTime")
		param.LandLossYears = atof(value.c_str());

	else if (token=="LandLossType") {
		if (value=="RANDOM")
			param.LandLossMethod = RANDOM;
		else if (value=="EDGES")
			param.LandLossMethod = EDGES;
		else
			Error(token,value);	
	}


	else if (token=="NoDiseaseYears")
		param.NoDiseaseYears = atof(value.c_str());

	else if (token=="EtoIRate") {
		param.lambdaE = atof(value.c_str());
	}
	else if (token=="ItoDeadRate") {
		param.lambdaI = atof(value.c_str());
	}

	else if (token=="DiseaseFucundity")
		param.InfectiousFecundityFactor = atof(value.c_str());
	else if (token=="MaternalTransmission")
		param.MaternalTransmission = atof(value.c_str());
	else if (token=="LionTransmission")
		param.LionTransmission = atof(value.c_str());
	else if (token=="OutgroupEncounter")
		param.OutgroupEncounter = atof(value.c_str());
	else if (token=="FoodTransmission")
		param.BuffaloTransmission = atof(value.c_str());
	else if (token=="FoodDiseaseProgression") {
		if (value=="CONSTANT")
			param.BuffaloDisease = CONSTANT;
		else if (value=="LOGISTIC")
			param.BuffaloDisease = LOGISTIC;
		else
			Error(token,value);	
	}


	else if (token=="StatsType") {
		if (value=="HUNTED")
			param.StatisticsType = STAT_HUNTED;
		else if (value=="RESCOALSIZE")
			param.StatisticsType = STAT_RESCOAL;
		else if (value=="PRIDESIZE")
			param.StatisticsType = STAT_PRIDE;
		else if (value=="DEMOGRAPHICS")
			param.StatisticsType = STAT_DEMOGRAPHICS;
		else if (value=="DISEASE")
			param.StatisticsType = STAT_DISEASE;
		else if (value=="FOODINFECT")
			param.StatisticsType = STAT_FOOD_INFECT;
		else if (value.substr(0,8)=="DEATHAGE") {
			param.StatisticsType = STAT_DEATH_AGES;
			param.StatisticsHuntUsed = false;
			param.StatisticsSexUsed = false;
			bool stopit = false;
			stringstream iss;
			string par;
			iss << value.substr(8);
			iss << " STOP";
			iss >> par;

			while (!stopit) {
				if (par=="STOP") {
					stopit = true;
				}
				else if (par=="M") {
					param.StatisticsSexUsed = true;
					param.StatisticsSex = MALE;
				}
				else if (par=="F") {
					param.StatisticsSexUsed = true;
					param.StatisticsSex = FEMALE;
				}
				else if (par=="H") {
					param.StatisticsHuntUsed = true;
					param.StatisticsHuntable = true;
				}
				else if (par=="P") {
					param.StatisticsHuntUsed = true;
					param.StatisticsHuntable = false;
				}
				else 
					Error(token,value);
		

				iss >> par;
			}

		}
		else if (value.substr(0,7) == "SURVIVE") {
			param.StatisticsType = STAT_SURVIVE;
			param.StatisticsAgeUsed = false;
			param.StatisticsHuntUsed = false;
			param.StatisticsSexUsed = false;

			stringstream iss;
			string par;
			bool minnum = false;
			bool maxnum = false;
			bool stopit = false;

			iss << value.substr(8);
			iss << " STOP";
			iss >> par;

			while (!stopit) {
				if (par=="STOP") {
					stopit = true;
				}
				else if (par=="M") {
					param.StatisticsSexUsed = true;
					param.StatisticsSex = MALE;
				}
				else if (par=="F") {
					param.StatisticsSexUsed = true;
					param.StatisticsSex = FEMALE;
				}
				else if (par=="H") {
					param.StatisticsHuntUsed = true;
					param.StatisticsHuntable = true;
				}
				else if (par=="P") {
					param.StatisticsHuntUsed = true;
					param.StatisticsHuntable = false;
				}
				else if (par=="MIN") {
					param.StatisticsAgeUsed = true;
					param.StatisticsAgeMin = true;
					minnum = true;
				}
				else if (par=="BTW") {
					param.StatisticsAgeUsed = true;
					param.StatisticsAgeMin = false;
					minnum = true;
					maxnum = true;
				}
				else if (minnum) {
					param.StatisticsAge1 = atof(par.c_str());
					minnum = false;
				}
				else if (maxnum) {
					param.StatisticsAge2 = atof(par.c_str());
					maxnum = false;
				}
				else {
					Error(token,value);
				}

				iss >> par;
			}
		}
		else {
			Error(token,value);
		}
	}

	else {
		Error(token,value);
	}

}

/**
	Parses the two-dimensional table of fighting odds
	@param fightnums list of rows in the fight table
*/
void Parser::ParseFightTable(list<string> fightnums) {

	list<string>::iterator iter;
	istringstream iss;
	int i,j=0;

	for (iter=fightnums.begin();iter!=fightnums.end();iter++) {
		iss.clear();
		iss.str(*iter);	

		for (i=0;i<10;i++) {
			assert(i<10);
			assert(j<10);
			iss >> param.FightNomadAttacks[i][j];
		}
		j++;
	}

}

/**
	Quits if there's an error reading the parameters file
	@param par parameter name
	@param val value given to parameter name
*/
void Parser::Error(string par, string val) {

	cout <<  "Syntax error in Parameters file: " << par << " = " << val << endl;
	exit(1);
}

/**
	Set parameters to automatic and default values
*/
void Parser::Prefill() {

	int i,k;

	param.CubMinAge[1] = 0.0;
	param.CubCategoriesDefined = 1;

	for (i=0;i<15;i++)
		for (k=0;k<10;k++) {
			assert(i<15);
			assert(k<10);
				param.ResidentTakes[i][k] = 0.0;
		}

}

/**
	Calculate the life expectancy for males and females and save as parameter values.
*/
void Parser::CalculateLifeExpectancy() {
	
	int i,j;
	int maxSteps;
	double facc, macc;
	bool mset,fset;
	double mtot,ftot;

	double femsteptable[60];
	double malsteptable[60];


	maxSteps = (int)(param.MaxAge*2);

	// create step-wise tables
	for (i=0;i<maxSteps;i++) {

		mset = false;
		fset = false;

		// cubs
		for (j=1;j<param.CubCategoriesDefined;j++) {
			assert(j+1<5);
			if (i<(int)(2*param.CubMinAge[j+1])) {
				assert(i<60);
				assert(j<5);
				femsteptable[i] = param.SurviveCub[j];
				malsteptable[i] = param.SurviveCub[j];
				j = param.CubCategoriesDefined;
				mset = true;
				fset = true;
			}
		}
		if (!fset && i<(int)(2*param.SubadultFemaleMinAge)) {
			assert(i<60);
			assert(param.CubCategoriesDefined<5);
			femsteptable[i] = param.SurviveCub[param.CubCategoriesDefined];
			fset = true;
		}
		if (!mset && i<(int)(2*param.SubadultMaleMinAge)) {
			assert(i<60);
			assert(param.CubCategoriesDefined<5);
			malsteptable[i] = param.SurviveCub[param.CubCategoriesDefined];
			mset = true;
		}

		// subadults
		if (!fset && i<(int)(2*param.AdultFemaleMinAge)) {
			assert(i<60);
			femsteptable[i] = param.SurviveSubadultFemale;
			fset = true;
		}
		if (!mset && i<(int)(2*param.AdultMaleMinAge)) {
			assert(i<60);
			malsteptable[i] = param.SurviveSubadultMale;
			mset = true;
		}

		// adults
		if (!fset) {
			assert(i<60);
			femsteptable[i] = param.SurviveAdultFemale;
		}
		if (!mset) {
			assert(i<60);
			malsteptable[i] = param.SurviveResidentMale;
		}
	}

	// for each from age
	for (i=0;i<maxSteps;i++) {

		facc = 1.0;
		macc = 1.0;
		ftot = 0;
		mtot = 0;

		// for each to age
		for (j=i;j<maxSteps;j++) {
			assert(j<60);
			facc = facc * femsteptable[j];
			ftot += facc;

			macc = macc * malsteptable[j];
			mtot += macc;
		}

		assert(i<60);
		param.LifeExpectancy[FEMALE][i] = ((double)i + ftot)/2.0;
		param.LifeExpectancy[MALE][i] = ((double)i + mtot)/2.0;
	}
		
}

