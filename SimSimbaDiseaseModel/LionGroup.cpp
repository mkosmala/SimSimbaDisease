/**
	LionGroup.cpp
	Defines a group of lions, either a pride (females and cubs) or coalition (males)
	
	@author mkosmala
*/

#include "stdafx.h"
#include "LionGroup.h"

using namespace SimSimba;

LionGroup::~LionGroup() {}

/* Getters for getting lions of various ages from this group */
list<Lion*> LionGroup::GetLionsAtAge(double age,Sex sex) {
	return GetLionsBetweenAges(age,age,sex);
}

list<Lion*> LionGroup::GetLionsOverAge(double age,Sex sex) {
	return GetLionsBetweenAges(age+0.5,1000.0,sex);
}

list<Lion*> LionGroup::GetLionsUnderAge(double age,Sex sex) {
	return GetLionsBetweenAges(-1.0,age-0.5,sex);
}

// inclusive
list<Lion*> LionGroup::GetLionsBetweenAges(double young,double old,Sex sex) {
	list<Lion*> cohort;
	list<Lion*>::iterator iter;

	for (iter=lions.begin();iter!=lions.end();iter++)
		if ((*iter)->GetAge()+0.01 >= young && (*iter)->GetAge()-0.01 <= old &&
			(*iter)->GetSex() == sex)
			cohort.push_back(*iter);

	return cohort;	
}

list<Lion*> LionGroup::GetCubs() {
	list<Lion*> cohort;
	list<Lion*>::iterator iter;
	for (iter=lions.begin();iter!=lions.end();iter++)
		if (((*iter)->GetSex() == MALE && (*iter)->GetAge() < param.SubadultMaleMinAge) ||
			((*iter)->GetSex() == FEMALE && (*iter)->GetAge() < param.SubadultFemaleMinAge))
			cohort.push_back(*iter);

	return cohort;
}

/**
	Returns whether this group is in the passed list of LionGroups
	@return whether this group is in the passed list of LionGroups (true/false)
*/
bool LionGroup::IsInList(list<LionGroup*> tlist) {
	bool inlist = false;
	list<LionGroup*>::iterator iter;
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
	Calculates the infectious contacts for this lion group. In particular, 
	caulculates the number of infectious lions within the group,
	the number of infectious lions in affiliated groups, and the prevelence
	of disease in the buffalo in the territories this group uses.
*/
void LionGroup::CalculateInfectiousContacts() {

	list<Lion*> allins;
	list<Lion*>::iterator liter;
	list<LionGroup*>::iterator giter;
	list<Territory*>::iterator titer;

	// go through all lions in ingroup and count those infectious
	infectiousIns = 0;
	for (giter=inGroup.begin();giter!=inGroup.end();giter++) {
		allins = (*giter)->GetLions();
		for (liter=allins.begin();liter!=allins.end();liter++) {
			if ((*liter)->GetDiseaseState() == INFECTIOUS)
				infectiousIns ++;
		}
	}

	// go through all lions in outgroup and count those infectious
	infectiousOuts = 0;
	for (giter=outGroup.begin();giter!=outGroup.end();giter++) {
		allins = (*giter)->GetLions();
		for (liter=allins.begin();liter!=allins.end();liter++) {
			if ((*liter)->GetDiseaseState() == INFECTIOUS)
				infectiousOuts ++;
		}
	}

	// from territories
	aveDiseasedDietBuffalo = 0;
	for (titer=orderedVisit.begin();titer!=orderedVisit.end();titer++) {
		aveDiseasedDietBuffalo += (*titer)->GetLionDietBuffalo() * (*titer)->GetDiseasedBuffalo();
	}
	aveDiseasedDietBuffalo = aveDiseasedDietBuffalo / orderedVisit.size();
}

