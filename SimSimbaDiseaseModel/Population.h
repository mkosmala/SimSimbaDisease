#ifndef POPULATION_H
#define POPULATION_H

#include "stdafx.h"
#include "Basics.h"
#include "Territory.h"
#include "Lion.h"
#include "Pride.h"
#include "Coalition.h"

using namespace std;
namespace SimSimba {

	class Population {
	public:
		Population(string fname,Param par,bool dostats,fstream& fs,fstream& fs2,bool wtt);
		~Population();

		double GetTimeStep() { return timestep; }
		int GetNextLionNumber() { 
			int x = nextLionNumber;
			nextLionNumber++;
			return x;
		}
		int GetNextGroupNumber() {
			int x = nextGroupNumber;
			nextGroupNumber++;
			return x;
		}
		int GetNextTerritoryNumber() {
			int x = nextTerritoryNumber;
			nextTerritoryNumber++;
			return x;
		}

		list<Territory*> GetAllTerritories() { return territories; }
		list<Lion*> GetAllLions() { return lions; }
		list<Pride*> GetAllPrides() { return prides; }
		list<Coalition*> GetAllCoalitions() { return coalitions; }

		void Save(string fname);
		void Step();
		void OpenTranscript() { transcript.open("transcript.txt", ios::out); }
		void WriteToTranscript(string line) { 
			if (writeToTranscript)
				transcript << line; 
		}
		void CloseTranscript() { transcript.close(); }

		void WriteToStats(string line) { 
			if (recordStatistics)
				(*stats) << line; 
		}
		void WriteToStats2(string line) { 
			if (recordStatistics)
				(*stats2) << line; 
		}



	private:
		double timestep;
		int nextLionNumber;
		int nextGroupNumber;
		int nextTerritoryNumber;
		bool writeToTranscript;
		bool recordStatistics;
		
		list<Pride*> wanderingFemales;

		Param param;

		list<Territory*> territories;
		list<Lion*> lions;
		list<Pride*> prides;
		list<Coalition*> coalitions;

		int deaths[9][7];
		int age_deaths[2][2][50]; // only needs 28 (not 50) right now (mf,hp,age)

		int disdeath[40];

		list<LionGroup*> toDelete;

		int numberHunted;
		fstream transcript;
		fstream* stats;
		fstream* stats2;

		int countEdeaths;
		int countEtoI;

		Territory* GetTerritory(int id);
		Lion* GetLion(int id);
		LionGroup* GetGroup(int id);

		int initialHuntingPopulation;
		int currentHuntingStrategy;

		void Load(string fname);
		void ProduceCubs();
		void CreateCub(Lion* mom);
		void AbandonCubs();
		void Abandon(Lion* cub);
		void Kill(Lion* lion,DeathReason reason);
		void CalculateSurvival();
		void Remove(LionGroup* group);
		void UpdateAges();
		void FemalesJoinPride();
		void FemalesDisperseSeparately();
		void Disperse(list<Lion*> cohort);
		void KickOutOldGirls();
		void MalesFormCoalitions();
		void PromoteAdultMales();
		void MalesMoveAndFight();
		Coalition* GetSubAdultCoaltion(Territory* terr);
		list<Coalition*> RandomizeCoalitionList(list<Coalition*> clist);
		void SubadultMove(Coalition* coal);
		void NomadicMoveAndFight(Coalition* coal);
		void ResidentMoveAndFight(Coalition* coal);
		void ProcureUnoccupiedTerritory(Coalition* coal,int awayby);
		void TakeOverTerritory(Coalition* coal,Territory* terr);
		bool MoveToNeighboringTerritory(Coalition* coal);
		Territory* ChooseRandomTerritory(list<Territory*> terr);
		void AttackResidentCoalition(Coalition* coal,Territory* terr);
		void Fight(Coalition* att,Coalition* def,Coalition* win);
		void DeleteMarkedGroups();
		void NomadsTeamUp();
		void HarvestTrophies();
		bool HarvestSingleTrophy(Sex huntsex);
		list<Lion*> GetHuntableLions(Sex huntsex);
		list<Lion*> GetHuntableMales();
		list<Lion*> GetHuntableFemales();
		list<Lion*> GetLions(double agemin,double agemax);
		list<Lion*> GetLions(Sex sex,double agemin,double agemax);
		list<Lion*> GetLions(list<Territory*> ters,double agemin,double agemax);
		list<Lion*> GetLions(list<Territory*> ters,Sex sex,double agemin,double agemax);
		double CalculateQ(Coalition* att,Coalition* def);
		list<Territory*> GetTerritories(bool huntable);
		void CoalitionLosesTerritory(Coalition* coal);
		double GetAveragePrideSize();
		double GetAverageResidentCoalitionSize();
		void TranscriptStatistics();
		void Statistics();
		bool IsSubsetOfList(list<Lion*> sub,list<Lion*> big);
		bool IsSubsetOfList(Lion* sub,list<Lion*> big);
		list<Coalition*> GetCoalitions(MaleStatus stat);
		void HomelessFemalesTakeTerritory();

		void LandLoss();
		void DeleteRandomTerritory();
		void DeleteEdgeTerritory();
		void DeleteTerritory(int id);
		void DisplaceLionsInTerritory(int id);
		list<Territory*> GetEdgeTerritories();

		void IdentifyIngroupAndOutgroup();
		list<LionGroup*> GetAllGroups();
		list<LionGroup*> DedupeGroups(list<LionGroup*> gps);
		list<LionGroup*> RemoveGroups(list<LionGroup*> big,list<LionGroup*> lit);

		void UpdateDisease();
		void UpdateBuffaloDisease();
		void CalculateDiseaseSurvival();

	};
}
#endif

