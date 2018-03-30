/**
	Pride.cpp
	A pride of lions, which includes adult and subadult females and cubs
	
	@author mkosmala
*/

#include "stdafx.h"
#include "Pride.h"

using namespace SimSimba;

// cast coaltions to liongroups
list<LionGroup*> Pride::GetSubadultMaleGroups() {

	list<LionGroup*> toRet;
	list<Coalition*>::iterator iter;
	
	for (iter=subadultMales.begin();iter!=subadultMales.end();iter++) 
		toRet.push_back((LionGroup*)(*iter));

	return toRet;
}

