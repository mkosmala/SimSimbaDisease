/**
	Territory.cpp
	A piece of land on which a lion group resides. Each territory is adjacent to some other 
	territories and together all the territories form the landscape for the population.
	
	@author mkosmala
*/

#include "stdafx.h"
#include "Territory.h"

using namespace SimSimba;

Territory::~Territory() {}

/**
	Calculate which territories are neighbors of this territory 
*/
void Territory::CalculateNeighbors() {

	// yourself is 0 away
	nearbyTerritories[0].push_back(this);

	// one-away neighbors have already been set up

	// calc 2,3,4,5-away neighbors
	int i;
	list<Territory*> already,nextto;
	list<Territory*>::iterator iter,iter2;

	// collect a list of territories that are already neighbors
	for (i=0;i<=1;i++) {
		for (iter=nearbyTerritories[i].begin();iter!=nearbyTerritories[i].end();iter++)
			already.push_back(*iter);
	}

	// for each distance away...
	for (i=2;i<=5;i++) {

		// ... collect a list of possibilities ...
		for (iter=nearbyTerritories[i-1].begin();iter!=nearbyTerritories[i-1].end();iter++) {
			nextto = (*iter)->GetTerritoryAwayBy(1);

			// ... go through those possibilities ...
			for (iter2=nextto.begin();iter2!=nextto.end();iter2++) {

				// ... and add it if we don't have it already
				if (!(*iter2)->IsInList(already)) {
					nearbyTerritories[i].push_back(*iter2);
					already.push_back(*iter2);
				}
			}
		}
	}
}

/**
	Return whether this territory is in the list of passed territories
	@param tlist list of territories to check
	@return true if this territory is in the list, false otherwise
*/
bool Territory::IsInList(list<Territory*> tlist) {
	bool inlist = false;
	list<Territory*>::iterator iter;
	for (iter=tlist.begin();iter!=tlist.end();)
		if ((*iter) == this) {
			inlist = true;
			iter = tlist.end();
		}
		else 
			iter++;
	return inlist;
}

/**
	Find a territory without a lion group that is the specified number of
	adjacencies away
	@param ab number of adjacencies away (self=0, neighbors=1, etc.)
	@return list of territories
*/
list<Territory*> Territory::GetEmptyTerritoryAwayBy(int ab) {

	list<Territory*> nearby = GetTerritoryAwayBy(ab);
	list<Territory*>::iterator iter;
	list<Territory*> empties;

	if (!nearby.empty())
		for (iter=nearby.begin();iter!=nearby.end();iter++) 
			if ((*iter)->GetPride() == NULL)
				empties.push_back(*iter);
	
	return empties;
}

/**
	Separate this territory from its neighbor
	@param id the ID of the neighboring territory to separate
*/
void Territory::RemoveNeighbor(int id) {

	int i;
	bool found = false;
	list<Territory*>::iterator iter;
	for (i=1;i<6;i++) {
		for (iter=nearbyTerritories[i].begin();iter!=nearbyTerritories[i].end();) {
			if ((*iter)->GetIDNumber() == id) {
				nearbyTerritories[i].erase(iter);
				iter = nearbyTerritories[i].end();
				found = true;
			}
			else
				iter++;
		}
		if (found) i = 6;
	}
}

/**
	Calculate the background disease prevalence on this territory 
	(i.e. the rate of bovine tuberculosis in the buffalo on this territory)
	@param tstep the timestep for which to calculate disease
*/
void Territory::CalculateDisease(double tstep) {

	double diff;
	double t = -4.5; 
	
	diff = tstep - startStepForDiseasedBuffalo;
	if (diff >= 0) {
		t += diff*0.1145; 
		// logistic growth
		diseasedBuffalo = 0.67 / ( 1 + exp(-t));  
	}
	else 
		diseasedBuffalo = 0;
}

