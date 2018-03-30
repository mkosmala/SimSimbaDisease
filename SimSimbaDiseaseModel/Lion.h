#ifndef LION_H
#define LION_H

#include "stdafx.h"
#include "Basics.h"

using namespace std;
namespace SimSimba {

	class LionGroup;

	class Lion {
	public:
		
		/**
		Constructor
		@param i lion ID number
		@param s lion sex (MALE, FEMALE)
		@param m this lion's mom
		@param par pointer to simulation parameters
		*/
		Lion(int i, Sex s, Lion* m,Param par) {
			age = 0;
			ageOfExposed = -1;
			idnum = i;
			sex = s;
			mom = m;
			param = par;
			permissionToStay = false;
			disease = SUSCEPTIBLE;
		}
		~Lion();

		/* Getters and Setters */
		int GetIDNumber() { return idnum; }
		double GetAge() { return age; }
		Sex GetSex() { return sex; }
		Lion* GetMom() { return mom; }
		list<Lion*> GetOffspring() { return offspring; }
		LionGroup* GetGroup() { return group; }

		int GetNumberOfCubsUnderAge(double age) {
			list<Lion*>::iterator iter;
			int ret = 0;
			for (iter=offspring.begin();iter!=offspring.end();iter++)
				if ((*iter)->GetAge() < age)
					ret++;
			return ret;
		}

		int GetNumberOfCubs() {
			list<Lion*> cubs;
			list<Lion*>::iterator iter;
			for (iter=offspring.begin();iter!=offspring.end();iter++) 
				if (((*iter)->GetSex() == MALE && (*iter)->GetAge() < param.SubadultMaleMinAge) ||
					((*iter)->GetSex() == FEMALE && (*iter)->GetAge() < param.SubadultFemaleMinAge))
					cubs.push_back(*iter);
			
			return int(cubs.size());
		}

		void SetAge(double a) { age = a; }
		void SetGroup(LionGroup* g) { group = g; }

		void AddOffspring(Lion* cub) { offspring.push_back(cub); }
		void RemoveOffspring(Lion* cub);
		void SetMomDead() { mom = NULL; }

		void AllowToStay() { permissionToStay = true; }
		bool GetAllowedToStay() { return permissionToStay; }

		int GetDemographicGroup();

		DiseaseState GetDiseaseState() { return disease; }
		void SetDiseaseState(DiseaseState ds);

		double GetAgeOfInfectious() { return ageOfInfectious; }
		double GetAgeOfExposed() { return ageOfExposed; }

		int GetInfectiousIns();
		int GetInfectiousOuts();
		double GetAveDiseasedDietBuffalo();

	private:
		int idnum;
		double age;
		Sex sex;
		Lion* mom;
		list<Lion*> offspring;
		LionGroup* group;
		Param param;
		bool permissionToStay;
		DiseaseState disease;
		double ageOfInfectious;
		double ageOfExposed;
	};

}
#endif

