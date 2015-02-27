#ifndef TERRITORY_H
#define TERRITORY_H

#include "stdafx.h"
#include "Basics.h"

using namespace std;
namespace SimSimba {

	class Pride;
	class Coalition;

	class Territory {
	public:
		Territory(int i,bool h,double ldb,double db,double ssfdb) {
			idnum = i;
			huntable = h;
			lionDietBuffalo = ldb;
			diseasedBuffalo = db;
			startStepForDiseasedBuffalo = ssfdb;
			lionessPride = NULL;
			lionCoalition = NULL;
			for (int j=0;j<6;j++){
				list<Territory*> nl;
				nearbyTerritories.push_back(nl);
			}
		}
		~Territory();

		int GetIDNumber() { return idnum; }
		bool IsHuntable() { return huntable; }
		list<Territory*> GetTerritoryAwayBy(int ab) { 
			if (ab>5) {
				list<Territory*> nothing;
				return nothing;
			}
			assert(ab < (int)nearbyTerritories.size());
			return nearbyTerritories[ab];
		}
		list<Territory*> GetEmptyTerritoryAwayBy(int ab);

		bool HasPride() { 
			if (lionessPride == NULL) return false;
			return true;
		}

		bool HasCoalition() {
			if (lionCoalition == NULL) return false;
			return true;
		}

		Pride* GetPride() { return lionessPride; }
		Coalition* GetCoalition() { return lionCoalition; }
		void SetPride(Pride* p) { lionessPride = p; }
		void SetCoalition(Coalition* c) { lionCoalition = c; }

		void AddAdjacentTerritory(Territory* t) { 
			assert(nearbyTerritories.size()>1);
			nearbyTerritories[1].push_back(t); 
		}
		void CalculateNeighbors();
		bool IsInList(list<Territory*> tlist);
		void RemoveNeighbor(int id);

		double GetLionDietBuffalo() { return lionDietBuffalo; }
		double GetDiseasedBuffalo() { return diseasedBuffalo; }
		void CalculateDisease(double tstep);
		double GetStartStep() { return startStepForDiseasedBuffalo; }

	private:
		int idnum;
		vector< list<Territory*> > nearbyTerritories;

		bool huntable;
		double lionDietBuffalo;
		double diseasedBuffalo;
		double startStepForDiseasedBuffalo;

		// residents
		Pride* lionessPride;
		Coalition* lionCoalition;
	};
}
#endif

