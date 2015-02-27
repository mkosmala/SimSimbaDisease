#ifndef BASICS_H
#define BASICS_H

#include "stdafx.h"
using namespace std;

namespace SimSimba {

	const double TIMESTEP = 0.5;

	enum Animal { LION, LEOPARD };
	enum Sex { MALE, FEMALE };
	enum MaleStatus { SUBADULT, NOMADIC, RESIDENT };
	enum DeathReason { BACKGROUND, ABANDONED, INFANTICIDE, FIGHT, HUNTED, LANDGONE, DISEASE };
	enum StatsType { STAT_SURVIVE, STAT_HUNTED, STAT_RESCOAL, STAT_PRIDE, STAT_DEATH_AGES, STAT_DEMOGRAPHICS, STAT_DISEASE, STAT_FOOD_INFECT };
	enum HuntFreq { ANNUAL, BIANNUAL };
	enum LandLossType { RANDOM, EDGES };
	enum DiseaseState { SUSCEPTIBLE, EXPOSED, INFECTIOUS };
	enum DiseaseSpread { CONSTANT, LOGISTIC };

	const int CUB0=0;
	const int CUB1=1;
	const int CUB2=2;
	const int CUB3=3;
	const int SUBADULT_F = 4;
	const int ADULT_F = 5;
	const int SUBADULT_M = 6;
	const int ADULT_M_NOMADIC = 7;
	const int ADULT_M_RESIDENT = 8;


	// for abandonment
	const double NEWBORN = 0.5; // under age
	const double INFANT = 1.0; // under age

	struct Param {

		// scenario number
		int ScenarioNum;

		// type
		Animal SimulationType;

		// Stats
		StatsType StatisticsType;
		bool StatisticsSexUsed;
		Sex StatisticsSex;
		bool StatisticsAgeUsed;
		bool StatisticsAgeMin; // false = between, true = minimum
		double StatisticsAge1;
		double StatisticsAge2;
		bool StatisticsHuntUsed;
		bool StatisticsHuntable; // false = park, true = hunted

		// Population
		int FemaleMax;
		int MaleMax;
		int MaxAge;

		// Age categories
		double FemaleReproduceAge;
		double MaleReproduceAge;
		double CubMinAge[5];
		int CubCategoriesDefined;
		double SubadultMaleMinAge;
		double SubadultFemaleMinAge;
		double AdultMaleMinAge;
		double AdultFemaleMinAge;

		// Survivorship 
		double SurviveResidentMale;
		double SurviveNomadicMale;
		double SurviveSubadultMale;
		double SurviveEvictedMale;
		double SurviveSubadultFemale;
		double SurviveAdultFemale;
		double SurviveCub[5];
		double SurviveOrphan;
		double SurviveHomelessFemale;

		// Fighting Survivorship 
		double SurviveTakeoverCub[5];
		double SurviveDefendingFemale;
		double SurviveDefendingMaleWins;
		double SurviveDefendingMaleLoses;
		double SurviveAttackingMaleLoses;
		double SurviveAttackingMaleWins;

		// Reproduction
		double LitterSize1;
		double LitterSize2;
		double LitterSize3;
		double LitterSize4;
		double CubMale;
		double CubAbandoned;

		// Territories
		//double Resident3MaleTakes2;
		//double Resident4MalesTakes2;
		//double Resident4MalesTakes3;
		double ResidentTakes[15][10]; // number of males, number of territories

		// Nomad Coalitions
		double NomadJoins1Resident;
		double NomadJoins1Nomad;
		double NomadJoins2Nomads;
		int NomadMoves;
		int SubadultMaleMoves;

		// Dispersal of Subadult Females
		double FemaleMoves2;
		double FemaleMoves3;
		double FemaleMoves4;
		double FemaleMoves5;

		// Trophy Hunting
		double TrophyMinAge;
		int TrophyQuota;
		HuntFreq HuntingFrequency;
		double TrophySexError;
		bool IgnoreQuota; 

		double SwitchHuntingStrategyAt;
		int HuntingStrategy;

		double TrophyMinAge1;
		int TrophyQuota1;
		HuntFreq HuntingFrequency1;
		double TrophySexError1;
		bool IgnoreQuota1; 
		double TrophyMinAge2;
		int TrophyQuota2;
		HuntFreq HuntingFrequency2;
		double TrophySexError2;
		bool IgnoreQuota2; 

		// land loss
		int OrigNumTerritories;
		double LandLossPercent;
		double LandLossYears;
		int LandLossMethod;

		// disease
		double NoDiseaseYears;
		double lambdaE;
		double lambdaI;
		double InfectiousFecundityFactor;
		double MaternalTransmission;
		double LionTransmission;
		double OutgroupEncounter;
		double BuffaloTransmission;
		DiseaseSpread BuffaloDisease;

		// Fight Matrix - nomads are along x-axis, residents along y-axis
		double FightNomadAttacks[10][10];

		// Life expenctancy table (background mortality only, optismistic) (used for disease calcs)
		double LifeExpectancy[2][60];

	};
}
#endif

