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
		/**
		Constuctor
		@param i Group ID number
		@param l The location of the coalition
		@param par Pointer to the simulation parameters
		*/
		Coalition(int i, Territory* l,Param par) : LionGroup(i,l,par) {
			sex = MALE;
			status = SUBADULT;
			evicted = false;
		}

		/**
		Get the status of this coalition: SUBADULT, NOMADIC, or RESIDENT
		@return coalition status (SUBADULT, NOMADIC, or RESIDENT)
		*/
		MaleStatus GetStatus() { return status; }
		
		/**
		Set the status of this coalition: SUBADULT, NOMADIC, or RESIDENT
		@param ms coalition status (SUBADULT, NOMADIC, or RESIDENT)
		*/		
		void SetStatus(MaleStatus ms) { 
			status = ms; 
			if (status==RESIDENT)
				evicted = false;
		}

		/**
		Get whether the coalition has been evicted from their territory
		@return whether coalition is evicted
		*/
		bool IsEvicted() { return evicted; }
		
		/** 
		Set whether the coalition has been evicted from their territory
		@param ev whether coalition is evicted
		*/
		void SetEvicted(bool ev) { evicted = ev; }

		/**
		Get a list of the territories this coalition controls
		@return a list of the territories this coalition controls
		*/
		list<Territory*> GetResidentTerritories() {
			if (status!=RESIDENT) {
				list<Territory*> none;
				return none;
			}
			return residentTerritories;
		}
		/**
		Add a new territory to the list of territories this coalition controls
		@param t the territory to add
		*/
		void AddResidentTerritory(Territory* t) { residentTerritories.push_back(t); }

		/** 
		Evict this coalition from the given territory
		@param terr the territory this coalition no longer controls
		*/
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

		/**
		Reset the list of visited territories
		*/
		void ResetVisited() { visited.clear(); }
		
		/**
		Add a territory to the list of visited territories
		@param terr the territory to add to the list
		*/
		void AddVisited(Territory* terr) { visited.push_back(terr); }
		
		/** 
		Get the list of territories that have been visted
		@return the list of territories that have been visted
		*/
		list<Territory*> GetVisited() { return visited; }

		/**
		Get the average age of the lions in the coalition
		@return the average age of the lions in the coalition
		*/
		double GetAverageAge() {
			list<Lion*>::iterator liter;
			double aveage = 0.0;
			for (liter=lions.begin();liter!=lions.end();liter++)
				aveage += (*liter)->GetAge();
			aveage = aveage / double(lions.size());
			return aveage;
		}

		/** 
		Check to see if this coalition is affiliated with a territory where hunting is allowed
		@return whether or not this coalition can be hunted
		*/
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

