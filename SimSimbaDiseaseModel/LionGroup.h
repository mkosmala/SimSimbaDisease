#ifndef LIONGROUP_H
#define LIONGROUP_H

#include "stdafx.h"
#include "Basics.h"
#include "Lion.h"
#include "Territory.h"

using namespace std;
namespace SimSimba {

	class LionGroup {
	public:
		/**
		Constuctor
		@param i ID number of the lion group
		@param l the location of this lion group
		@param par the simulation parameters
		*/
		LionGroup(int i, Territory* l,Param par) {
			idnum = i;
			location = l;
			param = par;
		}
		~LionGroup();

		/*
		Getters and Setters
		*/
		int GetIDNumber() { return idnum; }
		list<Lion*> GetLions() { return lions; }
		Territory* GetLocation() { return location; }
		Sex GetSex() { return sex; }

		void SetLocation(Territory* l) { location = l; }

		void AddLion(Lion* l) { lions.push_back(l); }
		void RemoveLion(Lion* l) { 
			list<Lion*>::iterator iter;
			for (iter=lions.begin();iter!=lions.end();) {
				if (*iter == l) {
					lions.erase(iter);
					iter = lions.end();
				}
				else
					iter++;
			}
		}

		list<Lion*> GetCubs();
		list<Lion*> GetLionsAtAge(double age,Sex sex);
		list<Lion*> GetLionsOverAge(double age,Sex sex);
		list<Lion*> GetLionsUnderAge(double age,Sex sex);
		list<Lion*> GetLionsBetweenAges(double young,double old,Sex sex);
		Lion* GetOldest() {
			list<Lion*>::iterator iter;
			Lion* oldest = lions.front();
			for (iter=lions.begin();iter!=lions.end();iter++) 
				if ((*iter)->GetAge() > oldest->GetAge())
					oldest = *iter;
			return oldest;
		}

		bool IsInList(list<LionGroup*> tlist);

		void SetInGroup(list<LionGroup*> lg) { inGroup = lg; }
		void SetOutGroup(list<LionGroup*> lg) { outGroup = lg; }
		list<LionGroup*> GetInGroup() { return inGroup; }
		list<LionGroup*> GetOutGroup() { return outGroup; }

		void ResetOrderedVisit() { orderedVisit.clear(); }
		void AddToOrderedVisit(Territory* terr) { orderedVisit.push_back(terr); }
		list<Territory*> GetOrderedVisitList() { return orderedVisit; }

		void CalculateInfectiousContacts();
		int GetInfectiousIns() { return infectiousIns; }
		int GetInfectiousOuts() { return infectiousOuts; }
		double GetAveDiseasedDietBuffalo() { return aveDiseasedDietBuffalo; }

	protected:
		int idnum;
		Sex sex;
		list<Lion*> lions;
		Territory* location;
		Param param;

		list<LionGroup*> inGroup;
		list<LionGroup*> outGroup;
		list<Territory*> orderedVisit;

		int infectiousIns;
		int infectiousOuts;
		double aveDiseasedDietBuffalo;

	};
}
#endif

