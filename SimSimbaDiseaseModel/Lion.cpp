/**
	Lion.cpp
	A lion
	
	@author mkosmala
*/
#include "stdafx.h"
#include "Lion.h"
#include "Coalition.h"

using namespace SimSimba;

Lion::~Lion() {}

/**
	Detach offspring from this lion, either because it died or we died
	@param cub offspring to remove
*/
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

/**
	Get the demographic group to which this lion belongs (cubs of 4 ages or subadult or adult,
	male or female, and if male, whether resident or nomadic)
	@return lion demographic group (CUB0,CUB1,CUB2,CUB3,SUBADULT_F,ADULT_F,SUBADULT_M,
	ADULT_M_NOMADIC, ADULT_M_RESIDENT)
*/
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

/** 
	Set the disease state for this lion
	@param ds disease state (SUSCEPTIBLE, EXPOSED, INFECTIOUS)
*/
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

/**
	Get the number of infectious lions in the same group as this lion.
	@return number of infections lions in this lion's group
*/
int Lion::GetInfectiousIns() { return group->GetInfectiousIns(); }

/**
	Get the number of infectious lions in groups affiliated with this lion's group
	@return number of infections lions in the groups affiliated with this lion's group
*/
int Lion::GetInfectiousOuts() { return group->GetInfectiousOuts(); }

/**
	Get the average prevelence of disease in buffalos in this lion's territory/ies
	@return the average prevelence of disease in buffalos in this lion's territory/ies
*/
double Lion::GetAveDiseasedDietBuffalo() { return group->GetAveDiseasedDietBuffalo(); }

