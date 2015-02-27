#ifndef PRIDE_H
#define PRIDE_H

#include "stdafx.h"
#include "Basics.h"
#include "LionGroup.h"

using namespace std;
namespace SimSimba {

	class Pride : public LionGroup {
	public:
		Pride(int i, Territory* l,Param par) : LionGroup(i,l,par) {
			sex = FEMALE;
			wandering = false;
		}

		void SetWandering(bool tf) { wandering = tf; }
		bool GetWandering() { return wandering; }

		list<Coalition*> GetSubadultMaleCoalitions() { return subadultMales; }
		list<LionGroup*> GetSubadultMaleGroups();

		void AddSubadultMaleCoalition(Coalition* coal) { subadultMales.push_back(coal); }
		void RemoveSubadultMaleCoaltion(Coalition* coal) {
			list<Coalition*>::iterator iter;
			for (iter=subadultMales.begin();iter!=subadultMales.end();) {
				if (*iter == coal) {
					subadultMales.erase(iter);
					iter = subadultMales.end();
				}
				else
					iter++;
			}
		}

	private:
		bool wandering;
		list<Coalition*> subadultMales;

	};
}
#endif

