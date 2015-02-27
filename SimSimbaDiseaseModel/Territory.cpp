#include "stdafx.h"
#include "Territory.h"

using namespace SimSimba;

Territory::~Territory() {}

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

