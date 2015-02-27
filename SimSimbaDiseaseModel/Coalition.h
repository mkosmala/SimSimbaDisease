#ifndef COALITION_H
#define COALITION_H

#include "stdafx.h"
#include "Basics.h"
#include "LionGroup.h"
#include "Territory.h"

using namespace std;
namespace SimSimba {

	class Coalition : public LionGroup {
	public:
		Coalition(int i, Territory* l,Param par) : LionGroup(i,l,par) {
			sex = MALE;
			status = SUBADULT;
			evicted = false;
		}

		MaleStatus GetStatus() { return status; }
		void SetStatus(MaleStatus ms) { 
			status = ms; 
			if (status==RESIDENT)
				evicted = false;
		}

		bool IsEvicted() { return evicted; }
		void SetEvicted(bool ev) { evicted = ev; }

		list<Territory*> GetResidentTerritories() {
			if (status!=RESIDENT) {
				list<Territory*> none;
				return none;
			}
			return residentTerritories;
		}
		void AddResidentTerritory(Territory* t) { residentTerritories.push_back(t); }

		void Evict(Territory* terr) {
			list<Territory*>::iterator iter;
			for (iter=residentTerritories.begin();iter!=residentTerritories.end();) {
				if ((*iter)==terr) {
					residentTerritories.erase(iter);
					iter = residentTerritories.end();
				}
				else
					iter++;
			}

			if (residentTerritories.size() == 0) {
				status = NOMADIC;
				evicted = true;
			}
			else {
				location = residentTerritories.front();
			}
		}

		void ResetVisited() { visited.clear(); }
		void AddVisited(Territory* terr) { visited.push_back(terr); }
		list<Territory*> GetVisited() { return visited; }

		double GetAverageAge() {
			list<Lion*>::iterator liter;
			double aveage = 0.0;
			for (liter=lions.begin();liter!=lions.end();liter++)
				aveage += (*liter)->GetAge();
			aveage = aveage / double(lions.size());
			return aveage;
		}

		bool IsHuntable() {
			list<Territory*>::iterator iter;
			
			if (status != RESIDENT)
				return location->IsHuntable();

			// otherwise check all territories
			bool huntme = false;
			for (iter=residentTerritories.begin();iter!=residentTerritories.end();iter++) {
				if ((*iter)->IsHuntable())
					huntme = true;
			}

			return huntme;
		}

	private:
		MaleStatus status;
		list<Territory*> residentTerritories;
		bool evicted;
		list<Territory*> visited;
	};
}
#endif

