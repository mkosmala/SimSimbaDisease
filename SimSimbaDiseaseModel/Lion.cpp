#include "stdafx.h"
#include "Lion.h"
#include "Coalition.h"

using namespace SimSimba;

Lion::~Lion() {}

void Lion::RemoveOffspring(Lion* cub) {
	list<Lion*>::iterator iter;
	for (iter=offspring.begin();iter!=offspring.end();) {
		if (*iter == cub) {
			offspring.erase(iter);
			iter = offspring.end();
		}
		else
			iter++;
	}
}

int Lion::GetDemographicGroup() {
	int ret;

	if ((sex == FEMALE && age<param.SubadultFemaleMinAge) ||
		(sex == MALE && age<param.SubadultMaleMinAge)) {
		if (param.CubCategoriesDefined>1) {
			if (age < param.CubMinAge[2])
				ret = CUB0;
			else if (param.CubCategoriesDefined>2) {
				if (age < param.CubMinAge[3])
					ret = CUB1;
				else if (param.CubCategoriesDefined==4) {
					if (age < param.CubMinAge[4])
						ret = CUB2;
					else
						ret = CUB3;
				}
				else
					ret = CUB2;
			}
			else
				ret = CUB1;
		}
		else
			ret = CUB0;
	}
	else if (sex == FEMALE) {
		if (age<param.AdultFemaleMinAge) 
			ret = SUBADULT_F;
		else 
			ret = ADULT_F;
	}
	else {
		if (age<param.AdultMaleMinAge) 
			ret = SUBADULT_M;
		else {
			if (((Coalition*)group)->GetStatus() == NOMADIC)
				ret = ADULT_M_NOMADIC;
			else
				ret = ADULT_M_RESIDENT;
		}
	}

	return ret;
}


void Lion::SetDiseaseState(DiseaseState ds) {

	// set state
	disease = ds;

	// calculate age at which to move to infectious if just moved to exposed
	if (ds==EXPOSED) {
		ageOfExposed = age;
	}
	// calculate age of death from disease
	else if (ds==INFECTIOUS) {
		ageOfInfectious = age;
	}
}


int Lion::GetInfectiousIns() { return group->GetInfectiousIns(); }
int Lion::GetInfectiousOuts() { return group->GetInfectiousOuts(); }
double Lion::GetAveDiseasedDietBuffalo() { return group->GetAveDiseasedDietBuffalo(); }

