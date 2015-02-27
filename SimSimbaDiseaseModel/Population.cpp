#include "stdafx.h"
#include "Population.h"

using namespace SimSimba;

Population::Population(string fname,Param par,bool dostats,fstream& fs,fstream& fs2,bool wtt) {
	timestep = 0;
	nextLionNumber = 0;
	nextGroupNumber = 0;
	nextTerritoryNumber = 0;
	initialHuntingPopulation = 0;
	currentHuntingStrategy = 1;
	param = par;
	stats = &fs;
	stats2 = &fs2;
	recordStatistics = dostats;
	writeToTranscript = wtt;

	// clear death indices and special stats
	int i,j,k;
	for (i=0;i<9;i++)
		for (j=0;j<7;j++) {
			assert(i<9);
			assert(j<7);
			deaths[i][j] = 0;
		}
	for (i=0;i<((param.MaxAge+1)*2);i++)
		for (j=0;j<2;j++)
			for (k=0;k<2;k++) {
				assert(k<2);
				assert(j<2);
				assert(i<50);
				age_deaths[k][j][i] = 0;
			}
	for (i=0;i<40;i++) {
		assert(i<40);
		disdeath[i] = 0;
	}

	countEdeaths = 0;
	countEtoI = 0;

	Load(fname);
	OpenTranscript();
	WriteToTranscript("Population loaded: ");
	stringstream iss;
	iss << territories.size() << " territories, " << prides.size() << " prides, ";
	iss << coalitions.size() << " coalitions, " << lions.size() << " lions\n";
	WriteToTranscript(iss.str());
}

Population::~Population() {
	list<Coalition*>::iterator citer;
	for (citer=coalitions.begin();citer!=coalitions.end();citer++)
		delete *citer;
	coalitions.clear();

	list<Pride*>::iterator piter;
	for (piter=prides.begin();piter!=prides.end();piter++)
		delete *piter;
	prides.clear();

	for (piter=wanderingFemales.begin();piter!=wanderingFemales.end();piter++)
		delete *piter;
	wanderingFemales.clear();

	list<Lion*>::iterator liter;
	for (liter=lions.begin();liter!=lions.end();liter++)
		delete *liter;
	lions.clear();

	list<Territory*>::iterator titer;
	for (titer=territories.begin();titer!=territories.end();titer++)
		delete *titer;
	territories.clear();

	WriteToTranscript("End of Population\n");
	CloseTranscript();

}



void Population::Step() {

	timestep += TIMESTEP;

	stringstream iss;
	iss << "\n\nStep " << timestep << "\n";
	WriteToTranscript(iss.str());

	// update ages
	WriteToTranscript("\n  Update Ages\n");
	UpdateAges();

	// harvesting
	WriteToTranscript("\n  Harvest Trophies\n");	
	HarvestTrophies();

	// cub production
	WriteToTranscript("\n  Cub Production\n");
	ProduceCubs();
	if (param.SimulationType == LION)
		AbandonCubs();

	// determine survival
	WriteToTranscript("\n  Survival\n");
	CalculateSurvival();
	CalculateDiseaseSurvival();

	// females
	WriteToTranscript("\n  Homeless Females Attempt to take Territory\n");
	HomelessFemalesTakeTerritory();
	WriteToTranscript("\n  Subadult Females Join their Natal Pride\n");
	if (param.SimulationType == LION)
		FemalesJoinPride();
	else
		FemalesDisperseSeparately();
	KickOutOldGirls();

	// males
	WriteToTranscript("\n  Subadult Males Form Coalitions\n");
	MalesFormCoalitions();
	WriteToTranscript("\n  Subadult Coalitions become Nomadic\n");
	PromoteAdultMales();
	WriteToTranscript("\n  Male Coalitions Move and Fight\n");
	MalesMoveAndFight();

	// land conversion
	WriteToTranscript("\n  Convert Land\n");
	LandLoss();

	// cleanup
	DeleteMarkedGroups();

	// disease
	if (timestep > param.NoDiseaseYears)
		UpdateDisease();

	// statistics
	Statistics();
}


Territory* Population::GetTerritory(int id) {
	Territory* t = NULL;
	list<Territory*>::iterator iter;
	for (iter=territories.begin();iter!=territories.end();) {
		if ((*iter)->GetIDNumber() == id) {
			t = (*iter);
			iter = territories.end();
		}
		else 
			iter++;
	}
	return t;
}

Lion* Population::GetLion(int id) {
	Lion* l = NULL;
	list<Lion*>::iterator iter;
	for (iter=lions.begin();iter!=lions.end();) {
		if ((*iter)->GetIDNumber() == id) {
			l = (*iter);
			iter = lions.end();
		}
		else
			iter++;
	}
	return l;
}

LionGroup* Population::GetGroup(int id) {
	LionGroup* g = NULL;
	list<Pride*>::iterator iter;
	list<Coalition*>::iterator iter2;

//	if (id==-1) return wanderingFemales;

	for (iter=prides.begin();iter!=prides.end();) {
		if ((*iter)->GetIDNumber() == id) {
			g = (*iter);
			iter = prides.end();
		}
		else
			iter++;
	}
	if (g==NULL)
		for (iter=wanderingFemales.begin();iter!=wanderingFemales.end();) {
			if ((*iter)->GetIDNumber() == id) {
				g = (*iter);
				iter = wanderingFemales.end();
			}
			else
				iter++;
		}
	if (g==NULL) 
		for (iter2=coalitions.begin();iter2!=coalitions.end();) {
			if ((*iter2)->GetIDNumber() == id) {
				g = (*iter2);
				iter2 = coalitions.end();
			}
			else
				iter2++;
		}
	return g;
}


void Population::Load(string fname) {

	fstream lf;
	istringstream iss;
	string line;
	char type;
	int idnum,adj,spot,status,sexnum,momid,grp;
	int biglionid=0;
	int biggroupid=0;
	Sex sex;
	Lion* mom;
	double age;
	bool hunt;
	double diet;
	double disbuf;
	list<string> ters;
	list<string> pris;
	list<string> coas;
	list<string> lios;
	list<string>::iterator iter;
	list<Territory*>::iterator iter2;
	list<int> nums;
	list<int>::iterator iter3;
	MaleStatus mstatus;

	lf.open(fname.c_str(), ios::in);
	if (!lf.is_open()) 
		cout << "Could not open file " << fname << endl;

	else {    
		getline (lf,line);
		if (line != "SIMSIMBA7")
			cout << fname << " is in the wrong format" << endl;

	    else {
			while (getline(lf,line)) {
				if (line.size()>0) {

					//  chomp
					line.erase(1 + line.find_last_not_of(' '));

					iss.str(line);
					iss >> type;
					switch (type) {
					case 'T': ters.push_back(line); break;
					case 'P': pris.push_back(line); break;
					case 'C': coas.push_back(line); break;
					case 'L': lios.push_back(line); break;
					default: // all other lines are assumed to be comments
						break;
					}
				}
			}
		}
	}
	lf.close();

	// go through the file lines
	// territories first
	// make the territories
	for (iter=ters.begin();iter!=ters.end();iter++) {
		iss.clear();
		iss.str(*iter);
		iss >> type >> idnum >> hunt >> diet >> disbuf;

		if (param.BuffaloDisease == CONSTANT) 
			territories.push_back(new Territory(idnum,hunt,diet,disbuf,0));
		else // LOGISTIC
			territories.push_back(new Territory(idnum,hunt,diet,0,disbuf));

	}
	param.OrigNumTerritories = int(territories.size());

	// and set up their adjacencies
	for (iter=ters.begin();iter!=ters.end();iter++) {
		iss.clear();
		iss.str(*iter);
		iss >> type >> idnum >> hunt >> diet >> disbuf;
		Territory* t1 = GetTerritory(idnum);
		while (!iss.eof()) {
			iss >> adj;
			if (adj == idnum)
				cout << "Territory " << idnum << " cannot be adjacent to itself" << endl;
			else {
				Territory* t2 = GetTerritory(adj);
				if (t2==NULL)
					cout << "No territory has id number " << adj << endl;
				else
					t1->AddAdjacentTerritory(t2);
			}
		}
	}
	// and expand neighborhood
	for (iter2=territories.begin();iter2!=territories.end();iter2++) 
		(*iter2)->CalculateNeighbors();
	
	// next, make the prides
	for (iter=pris.begin();iter!=pris.end();iter++) {
		iss.clear();
		iss.str(*iter);
		iss >> type >> idnum >> status >> spot;
		if (idnum>biggroupid) biggroupid = idnum;
		Territory* t = GetTerritory(spot);
		if (t->HasPride() && status==2)
			cout << "Can't have multiple prides at territory " << spot << endl;
		else {
			Pride* p = new Pride(idnum,t,param);

			if (status==1) {
				p->SetWandering(true);
				wanderingFemales.push_back(p);
			}
			else if (status==2) {
				prides.push_back(p);
				t->SetPride(p);
			}
			else
				cout << "Pride status must be 1 (wandering) or 2 (resident pride\n";

		}
	}	

	// next, the coalitions
	for (iter=coas.begin();iter!=coas.end();iter++) {
		nums.clear();
		iss.clear();
		iss.str(*iter);
		iss >> type >> idnum >> status;
		if (idnum>biggroupid) biggroupid = idnum;
		if (status<0 || status>2)
			cout << "invalid status of " << status << endl;
		else {
			switch (status) {
				case 0: mstatus = SUBADULT; break;
				case 1: mstatus = NOMADIC; break;
				case 2:	mstatus = RESIDENT; break;
			}
			while (!iss.eof()) {
				iss >> spot;
				nums.push_back(spot);
			}
			if ((mstatus == SUBADULT || mstatus == NOMADIC) && nums.size() > 1)
				cout << "subadult and nomadic coalitions can not be assigned more than one territory" << endl;
			spot = nums.front();
			Coalition* c = new Coalition(idnum,GetTerritory(spot),param);
			coalitions.push_back(c);
			c->SetStatus(mstatus);
			if (mstatus == RESIDENT) 
				for (iter3=nums.begin();iter3!=nums.end();iter3++) {
					Territory* t = GetTerritory(*iter3);
					if (t->HasCoalition())
						cout << "can not assign multiple resident coalitions to territory " << *iter3 << endl;
					else if (!t->HasPride())
						cout << "can not assign resident coalition " << idnum << " to territory " << *iter3 << ", which has no prides" << endl;
					else {
						c->AddResidentTerritory(t);
						t->SetCoalition(c);
					}
				}
		}
	}

	// next the lions
	for (iter=lios.begin();iter!=lios.end();iter++) {
		iss.clear();
		iss.str(*iter);
		iss >> type >> idnum >> sexnum >> age >> momid >> grp;
		if (idnum>biglionid) biglionid = idnum;
		if (sexnum==0) sex = MALE;
		else sex = FEMALE;
		if (momid==-1)
			mom = NULL;
		else
			mom = GetLion(momid);
		Lion* nlion = new Lion(idnum,sex,mom,param);
		nlion->SetAge(age);
		lions.push_back(nlion);
		if (momid!=-1)
			mom->AddOffspring(nlion);

		LionGroup* lgroup = GetGroup(grp);
		if (lgroup->GetSex() == MALE && sex == FEMALE) 
			cout << "lionesses may not be assigned to coalitions" << endl;
		else if (lgroup->GetSex() == FEMALE && sex == MALE && age >= param.SubadultMaleMinAge)
			cout << "adult lions may not be assigned to prides" << endl;
		else  {
			lgroup->AddLion(nlion);
			nlion->SetGroup(lgroup);
			if (sex==FEMALE && 
				(age>=param.AdultFemaleMinAge || ((Pride*)(lgroup))->GetWandering()))
				nlion->AllowToStay();
		}
	}

	// set the group and lion ID numbers appropriately
	nextGroupNumber = biggroupid+1;
	nextLionNumber = biglionid+1;
}

void Population::Save(string fname) {

	// create a new file with some name, get handle to file and open it
	fstream sf;
	sf.open(fname.c_str(), ios::out);

	sf << "SIMSIMBA7\n";

	// territories
	list<Territory*>::iterator titer,titer2;
	list<Territory*> tlist;
	for (titer=territories.begin();titer!=territories.end();titer++) {
		sf << "T " << (*titer)->GetIDNumber() << " ";
		sf << (*titer)->IsHuntable() << " ";
		sf << (*titer)->GetLionDietBuffalo() << " ";
		if (param.BuffaloDisease == CONSTANT) 
			sf << (*titer)->GetDiseasedBuffalo();
		else // LOGISTIC
			sf << (*titer)->GetStartStep();
		tlist = (*titer)->GetTerritoryAwayBy(1);
		for (titer2=tlist.begin();titer2!=tlist.end();titer2++)
			sf << " " << (*titer2)->GetIDNumber();
		sf << endl;
	}

	// prides
	list<Pride*>::iterator piter;
	for (piter=prides.begin();piter!=prides.end();piter++) {
		sf << "P " << (*piter)->GetIDNumber() << " 2 ";
		sf << (*piter)->GetLocation()->GetIDNumber() << endl;
	}
	for (piter=wanderingFemales.begin();piter!=wanderingFemales.end();piter++) {
		sf << "P " << (*piter)->GetIDNumber() << " 1 ";
		sf << (*piter)->GetLocation()->GetIDNumber() << endl;
	}

	// coalitions
	list<Coalition*>::iterator citer;
	for (citer=coalitions.begin();citer!=coalitions.end();citer++) {
		sf << "C " << (*citer)->GetIDNumber() << " ";
		sf << (*citer)->GetStatus();
		if ((*citer)->GetStatus() == RESIDENT) {
			tlist = (*citer)->GetResidentTerritories();
			for (titer=tlist.begin();titer!=tlist.end();titer++)
				sf << " " << (*titer)->GetIDNumber();
		}
		else
			sf << " " << (*citer)->GetLocation()->GetIDNumber();
		sf << endl;
	}

	// lions
	list<Lion*>::iterator liter;
	for (liter=lions.begin();liter!=lions.end();liter++) {
		sf << "L " << (*liter)->GetIDNumber() << " ";
		sf << (*liter)->GetSex() << " ";
		sf << (*liter)->GetAge() << " ";
		Lion* mom = (*liter)->GetMom();
		if (mom==NULL)
			sf << "-1 ";
		else
			sf << mom->GetIDNumber() << " ";
		sf << (*liter)->GetGroup()->GetIDNumber() << endl;
	}

	sf.close();
}


void Population::DeleteMarkedGroups() {
	LionGroup* group;
	while (!toDelete.empty()) {
		group = toDelete.front();
		toDelete.pop_front();
		delete group;
	}
}

void Population::ProduceCubs() {

	list<Lion*>::iterator iter;
	list<Lion*> preexisting;
	Lion* lion;
	double r;
	int litsize = 0;
	int i;
	stringstream iss;

	for (iter=lions.begin();iter!=lions.end();iter++) 
		preexisting.push_back(*iter);

	for (iter=preexisting.begin();iter!=preexisting.end();iter++) {

		lion = *iter;

		// only females of reproductive age without cubs reproduce
		// resident males must be present
		if (lion->GetSex() == FEMALE &&
			!((Pride*)(lion->GetGroup()))->GetWandering() &&
			lion->GetAge() >= param.FemaleReproduceAge &&
			lion->GetNumberOfCubs() == 0 &&    // if they reach 2 this turn, they're adult
			lion->GetGroup()->GetLocation()->HasCoalition()) {	

				// if you're infectious, maybe you have cubs and maybe not
				if (lion->GetDiseaseState() == INFECTIOUS)
					r =  double(rand())/(double(RAND_MAX) + 1.0);
				else
					r = -1;

				if (r < param.InfectiousFecundityFactor) {

					r =  double(rand())/(double(RAND_MAX) + 1.0);
					if (r < param.LitterSize1)
						litsize = 1;
					else if (r < param.LitterSize1 + param.LitterSize2)
						litsize = 2;
					else if (r < param.LitterSize1 + param.LitterSize2 + param.LitterSize3)
						litsize = 3;
					else
						litsize = 4;
				
					iss.str("");
					iss	<< "    Female " << lion->GetIDNumber() << " has " << litsize << " cub(s)\n";
					WriteToTranscript(iss.str());

					for (i=0;i<litsize;i++)
						CreateCub(lion);
				}
				else {
					iss.str("");
					iss << "    Female " << lion->GetIDNumber() << " is infectious and has no cubs.\n";
					WriteToTranscript(iss.str());
				}
		}
	}
}

void Population::CreateCub(Lion* mom) {

	double r;
	Sex s;
	Lion* cub;
	LionGroup* group;

	r = double(rand())/(double(RAND_MAX) + 1.0);
	if (r < param.CubMale)	s = MALE;
	else					s = FEMALE;
				
	cub = new Lion(GetNextLionNumber(),s,mom,param);
	mom->AddOffspring(cub); // tell mom
	group = mom->GetGroup(); 
	cub->SetGroup(group); // set cub's group
	group->AddLion(cub); // tell group
	lions.push_back(cub); // tell population

	stringstream iss;
	iss << "      New Cub " << cub->GetIDNumber() << ", ";
	if (cub->GetSex() == MALE) iss << "male, ";
	else iss << "female, ";
	iss << "group " << cub->GetGroup()->GetIDNumber() << endl;
	WriteToTranscript(iss.str());
}

void Population::AbandonCubs() {
	list<Pride*>::iterator piter;
	list<Lion*>::iterator liter;
	list<Lion*> lionesses;
	int infants=0;
	int newborns=0;
	double r;

	for (piter=prides.begin();piter!=prides.end();piter++) {
		lionesses = (*piter)->GetLions();
		// count the cubs under a year old in each pride
		for (liter=lionesses.begin();liter!=lionesses.end();liter++) {
			infants += (*liter)->GetNumberOfCubsUnderAge(INFANT);
			newborns += (*liter)->GetNumberOfCubsUnderAge(NEWBORN);
		}

		// if there is only one cub under a year old, it's abandoned if it's newborn
		if (newborns==1 && infants==1) {
			
			// probability of abandonment
		 	r = double(rand())/(double(RAND_MAX) + 1.0);
			if (r<param.CubAbandoned) {

				// abandon the singleton
				for (liter=lionesses.begin();liter!=lionesses.end();) {
					if ((*liter)->GetNumberOfCubsUnderAge(INFANT) > 0) {
						Abandon((*liter)->GetOffspring().back()); // abandon the youngest
						liter = lionesses.end();
					}
					else
						liter++;
				}
			}
		}
	}
}

void Population::Abandon(Lion* cub) {
	stringstream iss;
	iss << "    Cub " << cub->GetIDNumber() << " is abandoned. (Mom=" << cub->GetMom()->GetIDNumber();
	iss << ", Group=" << cub->GetGroup()->GetIDNumber() << ")\n";
	WriteToTranscript(iss.str());

	Kill(cub,ABANDONED);
}

void Population::Kill(Lion* lion, DeathReason reason) {

	stringstream iss;

	// put in death indices
	assert(lion->GetDemographicGroup()<9);
	assert(reason<7);
	deaths[lion->GetDemographicGroup()][reason]++;

	// keep track of disease
	if (reason != DISEASE) {
		disdeath[0] ++;
		if (lion->GetDiseaseState() == INFECTIOUS)
			disdeath[1] ++;
	}
	else {
		assert(1+(int)((lion->GetAge() - lion->GetAgeOfInfectious()) * 2.0) < 40);
		disdeath[1+(int)((lion->GetAge() - lion->GetAgeOfInfectious()) * 2.0)] ++;
	}

	if (lion->GetDiseaseState() == EXPOSED)
		countEdeaths++;

	int mf,ph,ind;
	if (lion->GetSex() == MALE) mf = 0;
	else mf = 1;
	bool huntme;
	if (mf==0) {
		Coalition* lcoal = (Coalition*)(lion->GetGroup());
		huntme = lcoal->IsHuntable();
	}
	else
		huntme = lion->GetGroup()->GetLocation()->IsHuntable();
	if (huntme) ph=1;
	else ph=0;
	ind = int(2*lion->GetAge());

	assert(mf<2);
	assert(ph<2);
	assert(ind<50);
	age_deaths[mf][ph][ind]++;



	list<Lion*>::iterator iter,iter2;
	list<Lion*> cubs;
	list<Lion*> temp;

	// tell mom
	Lion* mom = lion->GetMom();
	if (mom!=NULL)
		mom->RemoveOffspring(lion);

	// tell offspring
	list<Lion*> offs = lion->GetOffspring();
	if (!offs.empty()) 
		for (iter=offs.begin();iter!=offs.end();iter++) 
			(*iter)->SetMomDead();

	// tell group
	LionGroup* group = lion->GetGroup();
	group->RemoveLion(lion);

	// remove from population
	for (iter=lions.begin();iter!=lions.end();) {
		if (*iter == lion) {
			lions.erase(iter);
			iter = lions.end();
		}
		else
			iter++;
	}

	// delete
	delete lion;

	// if that was the last lion in that group to die, remove whole group
	if (group->GetLions().size() == 0) {
		Remove(group);		
	}

	// if the number in a coalition reduced to 2 or 3, we may have to give up territory
	else if (group->GetSex() == MALE) {
		
		Coalition* coal = (Coalition*)(group);
		if ((coal->GetLions().size() == 3 && coal->GetResidentTerritories().size() == 3) ||
			(coal->GetLions().size() == 2 && coal->GetResidentTerritories().size() == 2)) {
			
				Territory* terr = ChooseRandomTerritory(coal->GetResidentTerritories());
				terr->SetCoalition(NULL);
				coal->Evict(terr);
		}
	}
}

void Population::Remove(LionGroup* group) {
	
	list<Pride*>::iterator piter;
	list<Territory*>::iterator titer;
	list<Coalition*>::iterator citer;
	

	stringstream iss;
	iss << "    Group " << group->GetIDNumber() << " is gone";
	if (group->GetSex() == FEMALE && ((Pride*)group)->GetWandering())
		iss << " (wandering female)\n";
	else
		iss << endl;
	WriteToTranscript(iss.str());

	// coalition
	if (group->GetSex() == MALE) {

		// get it out of any prides
		list<Pride*>::iterator piter;
		for (piter=prides.begin();piter!=prides.end();piter++)
			(*piter)->RemoveSubadultMaleCoaltion((Coalition*)group);

		list<Territory*> resterr = ((Coalition*)(group))->GetResidentTerritories();
		for (titer=resterr.begin();titer!=resterr.end();titer++)
			(*titer)->SetCoalition(NULL);

		for (citer=coalitions.begin();citer!=coalitions.end();) {
			if (*citer == group) { 
				coalitions.erase(citer);
				citer = coalitions.end();
			}
			else citer++;
		}
	}
	// wandering female
	else if (((Pride*)group)->GetWandering()) {
		for (piter=wanderingFemales.begin();piter!=wanderingFemales.end();) {
			if (*piter == group) { 
				wanderingFemales.erase(piter);
				piter = wanderingFemales.end();
			}
			else piter++;
		}
	}
	//pride
	else {
		Territory* terr = group->GetLocation();
		terr->SetPride(NULL);

		// no male coalitions allowed on this territory anymore
		Coalition* rescoal = terr->GetCoalition();
		if (rescoal!=NULL) {
			rescoal->Evict(terr);
			rescoal->SetEvicted(false);
			terr->SetCoalition(NULL);
		}

		// erase the pride
		for (piter=prides.begin();piter!=prides.end();) {
			if (*piter == group) { 
				prides.erase(piter);
				piter = prides.end();
			}
			else piter++;
		}
	}

	//delete group;
	toDelete.push_back(group);
}

void Population::CalculateSurvival() {
	list<Lion*>::iterator iter;
	Lion* lion;
	double r;
	bool die;
	double age;	
	Sex sex;
	Coalition* coal;
	bool iscub = false;
	bool issubadult = false;
	double maxage;
	int i;
	list<Lion*> adults;

	list<Lion*> markedToDie;

	for (iter=lions.begin();iter!=lions.end();iter++) {
		lion = *iter;
		r = double(rand())/(double(RAND_MAX) + 1.0);
		die = false;
		age = lion->GetAge();
		sex = lion->GetSex();

		if ((sex==MALE && age<param.SubadultMaleMinAge) ||
			(sex==FEMALE && age<param.SubadultFemaleMinAge))
			iscub = true;

		else if ((sex==MALE && age<param.AdultMaleMinAge) ||
			(sex==FEMALE && age<param.AdultFemaleMinAge))
			issubadult = true;

		// too old
		if (age > param.MaxAge)
			die = true;

		// cubs
		else if (iscub) {

			adults = lion->GetGroup()->GetLionsOverAge((param.SubadultFemaleMinAge-0.1),FEMALE);

			// no adults at all
			if (adults.size() == 0 || IsSubsetOfList(adults,markedToDie))
				die = true;

			// orphan
			else if (lion->GetMom() == NULL || IsSubsetOfList(lion->GetMom(),markedToDie)) {
				if (r >= param.SurviveOrphan)
					die = true;
			}

			else {
				// for each of the cub groups
				for (i=1;i<=param.CubCategoriesDefined;i++) {

					if (param.CubCategoriesDefined==i) {
						if (sex==FEMALE)
							maxage = param.SubadultFemaleMinAge;
						else
							maxage = param.SubadultMaleMinAge;
					}
					else {
						assert(i+1<5);
						maxage = param.CubMinAge[i+1];
					}
	
					assert(i<5);
					if (age >= param.CubMinAge[i] && age < maxage) {
						if (r >= param.SurviveCub[i]) 
							die = true;
						i = param.CubCategoriesDefined + 1;
					}
				}				
			}
		}

		// subadults
		else if (issubadult) {
			if (lion->GetSex() == MALE) {
				coal = (Coalition*)(lion->GetGroup());
				if (coal->GetStatus() != RESIDENT && r >= param.SurviveSubadultMale) {
						die = true;
				}
			}
			else {
				if (((Pride*)(lion->GetGroup()))->GetWandering()) {
					if (r >= param.SurviveHomelessFemale)
						die = true;
				}

				else {
					if (r >= param.SurviveSubadultFemale)
						die = true;
				}
			}
		}

		// adults
		else {
			if (lion->GetSex() == MALE) {
				coal = (Coalition*)(lion->GetGroup());
				if (coal->GetStatus() == RESIDENT) {
					if (r >= param.SurviveResidentMale) 
						die = true;
				}
				else if (coal->GetStatus() == SUBADULT) {
					if (r >= param.SurviveSubadultMale)
						die = true;
				}
				else if (coal->IsEvicted()) {
					if (r >= param.SurviveEvictedMale)
						die = true;
				}
				else {
					if (r >= param.SurviveNomadicMale)
						die = true;
				}
			}
			else {
				// homeless
				if (((Pride*)(lion->GetGroup()))->GetWandering()) {
					if (r >= param.SurviveHomelessFemale)
						die = true;
				}

				else 
					if (r >=param.SurviveAdultFemale)
						die = true;
			}
		}
	
		if (die) {
			markedToDie.push_back(lion);

			stringstream iss;
			iss << "    Lion " << lion->GetIDNumber() << " dies, age " << lion->GetAge();
			if (lion->GetSex() == MALE)
				iss << ", sex = M";
			else
				iss << ", sex = F";
			iss << ", group " << lion->GetGroup()->GetIDNumber() << endl; ;
			WriteToTranscript(iss.str());
		}
	}

	for (iter=markedToDie.begin();iter!=markedToDie.end();iter++) {
		Kill(*iter,BACKGROUND);
	}

}

void Population::CalculateDiseaseSurvival() {
	list<Lion*>::iterator iter;
	Lion* lion;
	double r;
	bool die;
	list<Lion*> markedToDie;
	stringstream iss;

	iss << endl << "  Survival (disease)" << endl;
	WriteToTranscript(iss.str());

	for (iter=lions.begin();iter!=lions.end();iter++) {
		lion = *iter;
		r = double(rand())/(double(RAND_MAX) + 1.0);
		die = false;
	
		if (lion->GetDiseaseState() == INFECTIOUS) {

			// modelled as Poisson process with constant probability of death
			r = double(rand())/(double(RAND_MAX) + 1.0);
			if (r < param.lambdaI) 
				die = true;
		}

		if (die) {
			markedToDie.push_back(lion);

			iss.str("");
			iss << "    Lion " << lion->GetIDNumber() << " dies of disease, age " << lion->GetAge();
			if (lion->GetSex() == MALE)
				iss << ", sex = M";
			else
				iss << ", sex = F";
			iss << ", group " << lion->GetGroup()->GetIDNumber();
			iss << ", infectious at age " << lion->GetAgeOfInfectious() << endl;	
			WriteToTranscript(iss.str());
		}
	}

	for (iter=markedToDie.begin();iter!=markedToDie.end();iter++) {
		Kill(*iter,DISEASE);
	}

}


void Population::UpdateAges() {
	list<Lion*>::iterator iter;
	for (iter=lions.begin();iter!=lions.end();iter++)
		(*iter)->SetAge((*iter)->GetAge() + TIMESTEP);
}

void Population::KickOutOldGirls() {

	list<Pride*>::iterator piter;
	list<Lion*> leos;
	list<Lion*>::iterator liter;
	ostringstream iss;

	for (piter=prides.begin();piter!=prides.end();piter++) {
		leos = (*piter)->GetLionsAtAge(param.FemaleReproduceAge-0.5,FEMALE);
	
		for (liter=leos.begin();liter!=leos.end();liter++) {

			if ((*liter) != (*piter)->GetOldest() && !(*liter)->GetAllowedToStay()) {
				iss.str("");
				iss << "      Subadult female " << (*liter)->GetIDNumber() << " is kicked out at age " << (*liter)->GetAge() << endl;
				WriteToTranscript(iss.str());

				Pride* wanderer = new Pride(GetNextGroupNumber(),(*piter)->GetLocation(),param);
				wanderer->SetWandering(true);
				wanderingFemales.push_back(wanderer);
				wanderer->AddLion(*liter);
				(*piter)->RemoveLion(*liter);
				(*liter)->SetGroup(wanderer);
				(*liter)->AllowToStay();

				wanderer->ResetOrderedVisit();
				wanderer->AddToOrderedVisit(wanderer->GetLocation());

			}
		}
	}
}

void Population::HomelessFemalesTakeTerritory() {
	list<Pride*>::iterator piter;
	list<Lion*> femgroup;
	list<Pride*> successful;
	ostringstream iss;
	list<Lion*>::iterator liter;

	for (piter=wanderingFemales.begin();piter!=wanderingFemales.end();piter++) {
		(*piter)->ResetOrderedVisit();
		(*piter)->AddToOrderedVisit((*piter)->GetLocation());

		femgroup = (*piter)->GetLions();

		iss.str("");
		iss << "    Homeless female group " << (*piter)->GetIDNumber() << " (ages ";
		for (liter=femgroup.begin();liter!=femgroup.end();liter++)
			iss << (*liter)->GetAge() << " ";
		iss << ") looks near Territory ";
		iss << (*piter)->GetLocation()->GetIDNumber() << endl;
		WriteToTranscript(iss.str());

		Disperse(femgroup);
		if (((*piter)->GetLions()).empty()) 
			successful.push_back(*piter);

	}

	// if successful, remove homeless prides
	for (piter=successful.begin();piter!=successful.end();piter++) 
		Remove(*piter);

}


void Population::FemalesDisperseSeparately() {
	list<Pride*>::iterator piter;
	list<Lion*>::iterator liter;
	list<Lion*> cohort;
	Lion* mom;
	list<Lion*> justone;

	for (piter=prides.begin();piter!=prides.end();piter++) {

		cohort = (*piter)->GetLionsBetweenAges(param.SubadultFemaleMinAge,param.MaxAge,FEMALE);
		
		if (cohort.size()>1) {

			// remove the oldest (mom)
			mom = (*piter)->GetOldest();
			for (liter=cohort.begin();liter!=cohort.end();) {
				if (*liter == mom) {
					cohort.erase(liter);
					liter = cohort.end();
				}
				else
					liter++;
			}
	
			// everyone else needs to disperse (individually)
			for (liter=cohort.begin();liter!=cohort.end();liter++) {
				justone.clear();
				justone.push_back(*liter);
				Disperse(justone);
			}
		}
	}
}

void Population::FemalesJoinPride() {

	list<Pride*>::iterator piter;
	list<Lion*>::iterator liter;
	list<Lion*> cohort;
	list<Pride*> startprides;
	Lion* lion;
	int pridesize;

	stringstream iss;
	iss.str("");
	iss << "    There are " << prides.size() << " prides\n";
	WriteToTranscript(iss.str());

	for (piter=prides.begin();piter!=prides.end();piter++) {
		startprides.push_back(*piter);
		(*piter)->ResetOrderedVisit();
		(*piter)->AddToOrderedVisit((*piter)->GetLocation());
	}

	for (piter=startprides.begin();piter!=startprides.end();piter++) {
		pridesize = (int)((*piter)->GetLionsOverAge(param.SubadultFemaleMinAge,FEMALE).size());
		cohort = (*piter)->GetLionsAtAge(param.SubadultFemaleMinAge,FEMALE);

		iss.str("");
		iss << "    Group " << (*piter)->GetIDNumber() << " has " << pridesize 
			<< " adults and " << cohort.size() << " subadults\n";
		WriteToTranscript(iss.str());

		// allow to join if current size is below "max" (really, "ideal" or average size)
		if (pridesize < param.FemaleMax) {
			for (liter=cohort.begin();liter!=cohort.end();liter++) 
				(*liter)->AllowToStay();
			cohort.clear();
		}

		// cohort now contains the non-recruits
		if (cohort.size()>0) {
			
			// if there's fewer than 3, they go
			if (cohort.size()<3) {
				for (liter=cohort.begin();liter!=cohort.end();liter++) {
					iss.str("");
					iss << "      Subadult lioness " << (*liter)->GetIDNumber() << " goes a-wandering (<3 in cohort)\n";
					WriteToTranscript(iss.str());

					Pride* wanderer = new Pride(GetNextGroupNumber(),(*piter)->GetLocation(),param);
					wanderer->AddToOrderedVisit((*piter)->GetLocation());
					wanderer->SetWandering(true);
					wanderingFemales.push_back(wanderer);
					wanderer->AddLion(*liter);
					(*piter)->RemoveLion(*liter);
					(*liter)->SetGroup(wanderer);
					(*liter)->AllowToStay();

				}
			}
			else {

				// if more than 10, kill off the others
				// if more than max size, kick out the others
				while (cohort.size()>10) {
					lion = cohort.back();
					cohort.pop_back();

					iss.str("");
					iss << "      Subadult lioness " << lion->GetIDNumber() << " goes a-wandering (>max)\n";
					WriteToTranscript(iss.str());

					Pride* wanderer = new Pride(GetNextGroupNumber(),(*piter)->GetLocation(),param);
					wanderer->AddToOrderedVisit((*piter)->GetLocation());
					wanderer->SetWandering(true);
					wanderingFemales.push_back(wanderer);
					wanderer->AddLion(lion);
					(*piter)->RemoveLion(lion);
					lion->SetGroup(wanderer);
					lion->AllowToStay();

				}

				Disperse(cohort);
			}
		}

		iss.str("");
		iss << "      after joining/dispersing, has " << (*piter)->GetLionsOverAge(param.SubadultFemaleMinAge-0.1,FEMALE).size() << endl;
		WriteToTranscript(iss.str());

	}
}

void Population::Disperse(list<Lion*> cohort) {

	Pride* mompride = (Pride*)(cohort.front()->GetGroup());
	Territory* home = mompride->GetLocation();
	Territory* newhome;
	list<Territory*> poss;
	list<Territory*>::iterator iter;
	list<Lion*>::iterator liter;
	stringstream iss;

	// first try to settle open territory one away
	bool found = false;
	int awayby = 1;
	double prob,r;
	int settleme,i;

	while (!found && awayby<=5) {
		
		switch(awayby) {
			case 1: prob = 1.0; break;
			case 2: prob = param.FemaleMoves2; break;
			case 3: prob = param.FemaleMoves3; break;
			case 4: prob = param.FemaleMoves4; break;
			case 5: prob = param.FemaleMoves5; break;
		}

		r = double(rand())/(double(RAND_MAX) + 1.0);

		// perform move with some probability
		if (r<prob) {
			poss = home->GetEmptyTerritoryAwayBy(awayby);
			if (poss.size()>0) {
				// choose one at random
				settleme = rand() % int(poss.size());
				iter = poss.begin();
				for (i=0;i<settleme;i++)
					iter++;
				assert(iter!=poss.end());
				newhome = *iter;
				found = true;
			}
		}

		awayby++;
	}

	// if we find a new home, settle there
	if (found) {
		Pride* npride = new Pride(GetNextGroupNumber(),newhome,param);
		newhome->SetPride(npride);
		npride->AddToOrderedVisit(newhome);

		iss.str("");
		iss << "      New Resident Group (" << npride->GetIDNumber() << ") forms in Territory " << 
			newhome->GetIDNumber() << endl;
		WriteToTranscript(iss.str());

		prides.push_back(npride);
		for (liter=cohort.begin();liter!=cohort.end();liter++) {
			// tell old pride
			mompride->RemoveLion(*liter);
			// tell new pride
			npride->AddLion(*liter);
			// update lion internally
			(*liter)->SetGroup(npride);
			(*liter)->AllowToStay();

			iss.str("");
			iss << "        Female " << (*liter)->GetIDNumber() << "(age ";
			iss << (*liter)->GetAge() << ") moves to new group\n";
			if (mompride->GetWandering())
				iss << "          (was homeless)\n";
			WriteToTranscript(iss.str());
		}
	}

}

void Population::MalesFormCoalitions() {

	stringstream iss;
	list<Pride*>::iterator iter;
	list<Lion*>::iterator liter;
	Pride* pri;
	list<Lion*> subadults;
	list<Lion*> subadults2;
	list<Lion*> subadults3;
	Coalition* sacoal;
	bool done;

	// for each pride, find the new subadults
	for (iter=prides.begin();iter!=prides.end();iter++) {
		pri = *iter;

		// collect the new subadults
		subadults = pri->GetLionsAtAge(param.SubadultMaleMinAge,MALE);
		
		if (!subadults.empty()) {

			if (param.SimulationType == LION) {
	
				// see if there's another group around (on same territory)
				// can only join it if 1) there are no lions over 3 in it and
				// 2) if we don't exceed the max size by joining it
				sacoal = GetSubAdultCoaltion(pri->GetLocation());
				done = false;
				if (sacoal!=NULL) {
					if ((sacoal->GetLionsOverAge(param.SubadultMaleMinAge+1.0,MALE)).empty() &&
						int((sacoal->GetLions()).size() + subadults.size()) <= param.MaleMax) {
							done = true;

							iss.str("");
							iss << "      Subadult males in pride " << pri->GetIDNumber();
							iss << " join existing coalition " << sacoal->GetIDNumber() << endl;
							WriteToTranscript(iss.str());
					}
				}

				// form our own new group(s)
				if (!done) {
					sacoal = new Coalition(GetNextGroupNumber(),pri->GetLocation(),param);
					pri->AddSubadultMaleCoalition(sacoal);
					coalitions.push_back(sacoal);

					iss.str("");
					iss << "      Subadult males in pride " << pri->GetIDNumber();
					iss << " create new coalition " << sacoal->GetIDNumber() << endl;
					WriteToTranscript(iss.str());
				}

				// move the subadult lions over to the group
				iss.str("");
				iss << "        Starting Coalition size: " << (sacoal->GetLions()).size() << endl;
				WriteToTranscript(iss.str());
				iss.str("");
				iss << "        New Subadult males: " << subadults.size() << endl;
				WriteToTranscript(iss.str());

				for (liter=subadults.begin();liter!=subadults.end();liter++) {
					pri->RemoveLion(*liter);
					sacoal->AddLion(*liter);
					(*liter)->SetGroup(sacoal);
				}			
			}
			else { // LEOPARD
				// each one gets its own coalition and all go their own way
				for (liter=subadults.begin();liter!=subadults.end();liter++) {
					sacoal = new Coalition(GetNextGroupNumber(),pri->GetLocation(),param);
					pri->AddSubadultMaleCoalition(sacoal);
					coalitions.push_back(sacoal);
					pri->RemoveLion(*liter);
					sacoal->AddLion(*liter);
					(*liter)->SetGroup(sacoal);
					iss.str("");
					iss << "        Subadult male " << (*liter)->GetIDNumber();
					iss << " from Pride " << pri->GetIDNumber() << " forms new Coalition ";
					iss << sacoal->GetIDNumber() << endl;
					WriteToTranscript(iss.str());				
				}
			}
		}
	}

	// see if any prides are empty now and remove if so
	for (iter=prides.begin();iter!=prides.end();) {
		pri = *iter;
		if (pri->GetLions().empty()) {
			iter++;
			Remove(pri);
		}
		else
			iter++;
	}
}

Coalition* Population::GetSubAdultCoaltion(Territory* terr) {

	Coalition* toret = NULL;

	list<Coalition*>::iterator iter;
	for (iter=coalitions.begin();iter!=coalitions.end();) {
		if ((*iter)->GetLocation() == terr &&
			(*iter)->GetStatus() == SUBADULT) {
			toret = *iter;
			iter = coalitions.end();
		}
		else 
			iter++;		
	}

	return toret;
}


void Population::PromoteAdultMales() {
	list<Coalition*>::iterator iter;
	stringstream iss;
	double aveage;
	list<Lion*> leos;
	list<Lion*>::iterator liter;
	list<Pride*>::iterator piter;

	// promote when average age is >= 3
	// OLD only promote if all subadults have reached age 3.0
	// OLD (promote if oldest has reached age 3.0)
	for (iter=coalitions.begin();iter!=coalitions.end();iter++) {
		aveage = (*iter)->GetAverageAge();
		if ((*iter)->GetStatus() == SUBADULT && aveage > param.MaleReproduceAge) {
			(*iter)->SetStatus(NOMADIC);

			for (piter=prides.begin();piter!=prides.end();piter++) 
				(*piter)->RemoveSubadultMaleCoaltion(*iter);

			iss.str("");
			iss << "      Subadult Coalition " << (*iter)->GetIDNumber() << " (average age = ";
			iss << aveage << ") is promoted to Nomadic\n";
			WriteToTranscript(iss.str());
		}
	}
}

void Population::MalesMoveAndFight() {

	list<Coalition*> coals;
	list<Coalition*>::iterator iter;
	stringstream iss;
	list<Lion*> leos;
	list<Lion*>::iterator liter;
	list<Territory*> terrs;
	list<Territory*>::iterator titer;

	// coalitions go oldest first
	for (iter=coalitions.begin();iter!=coalitions.end();iter++)
		coals.push_back(*iter);

	// do each coalition
	for (iter=coals.begin();iter!=coals.end();iter++) {

		// make sure this group hasn't been deleted
		if (!(*iter)->IsInList(toDelete)) {


			iss.str("");
			iss << "    Male Coalition " << (*iter)->GetIDNumber() << " : ";
			if ((*iter)->GetStatus() == RESIDENT) {
				iss << "RESIDENT in territory/ies";
				terrs = (*iter)->GetResidentTerritories();
				for (titer=terrs.begin();titer!=terrs.end();titer++)
					iss << " " << (*titer)->GetIDNumber();
			}
			else if ((*iter)->GetStatus() == NOMADIC)
				iss << "NOMADIC in territory " << (*iter)->GetLocation()->GetIDNumber();
			else
				iss << "SUBADULT in territory " << (*iter)->GetLocation()->GetIDNumber();
			iss << " ( age ";
			leos = (*iter)->GetLions();
			for (liter=leos.begin();liter!=leos.end();liter++)
				iss << (*liter)->GetAge() << " ";
			iss << ")\n";
			WriteToTranscript(iss.str());

			(*iter)->ResetVisited();
			(*iter)->ResetOrderedVisit();
			if ((*iter)->GetStatus() != NOMADIC)
				(*iter)->AddToOrderedVisit((*iter)->GetLocation());

			for (int i=0;i<param.NomadMoves;i++) {
				if ((*iter)->GetStatus() == SUBADULT)
					SubadultMove(*iter);
				else if ((*iter)->GetStatus() == NOMADIC)
					NomadicMoveAndFight(*iter);
				else // RESIDENT
					ResidentMoveAndFight(*iter);

				// if the coalition has no more alive lions, we're done
				if ((*iter)->IsInList(toDelete))
					i = param.NomadMoves;
			}

			iss.str("");
			iss << "    Now ";
			if ((*iter)->GetStatus() == RESIDENT) {
				iss << "RESIDENT in territory/ies";
				terrs = (*iter)->GetResidentTerritories();
				for (titer=terrs.begin();titer!=terrs.end();titer++)
					iss << " " << (*titer)->GetIDNumber();
			}
			else if ((*iter)->GetStatus() == NOMADIC)
				iss << "NOMADIC in territory " << (*iter)->GetLocation()->GetIDNumber();
			else
				iss << "SUBADULT in territory " << (*iter)->GetLocation()->GetIDNumber();
			iss << " ( age ";
			leos = (*iter)->GetLions();
			for (liter=leos.begin();liter!=leos.end();liter++)
				iss << (*liter)->GetAge() << " ";
			iss << ")\n\n";
			WriteToTranscript(iss.str());


		}
	}

	// see if any of the nomads want to join up with one another
	NomadsTeamUp();
}

void Population::NomadsTeamUp() {
	list<Coalition*>::iterator iter,iter2;
	list<Coalition*> nomads;
	list<Lion*>::iterator liter;
	list<Lion*> llist;
	stringstream iss;

	// collect the nomadic groups of 1 or 2
	for (iter=coalitions.begin();iter!=coalitions.end();iter++) {
		if ((*iter)->GetStatus() == NOMADIC &&
			((*iter)->GetLions()).size() <= 2) 
			nomads.push_back(*iter);
	}

	// for each nomad, see if on same territory as another
	for (iter=nomads.begin();iter!=nomads.end();iter++) {

		if (!(*iter)->IsInList(toDelete)) 
		for (iter2=nomads.begin();iter2!=nomads.end();iter2++) {

			if (!(*iter2)->IsInList(toDelete)) 
			if ((*iter)->GetLocation() == (*iter2)->GetLocation() &&
				*iter != *iter2 && 
				(((*iter)->GetLions()).size() + ((*iter)->GetLions()).size()) <=3) {

					llist = (*iter2)->GetLions();

					iss.str("");
					iss << "        Males (" << (*iter2)->GetLions().size() << ")";
					iss << " from nomadic coalition " << (*iter2)->GetIDNumber();
					iss << " join nomadic coalition " << (*iter)->GetIDNumber() << " (";
					iss << (*iter)->GetLions().size() << " males)\n";
					WriteToTranscript(iss.str());

					// move lions
					for (liter=llist.begin();liter!=llist.end();liter++) {
						(*iter2)->RemoveLion(*liter);
						(*iter)->AddLion(*liter);
						(*liter)->SetGroup(*iter);
					}
					// remove 2nd group
					Remove(*iter2);		
			}
		}
	}
}

list<Coalition*> Population::RandomizeCoalitionList(list<Coalition*> clist) {

	list<Coalition*> rlist;

	// if empty, skip the work
	if (clist.empty())
		return rlist;

	// random shuffle doesn't work on lists, so we copy to a vector and back
	vector<Coalition*> cvect;
	copy(clist.begin(),clist.end(),back_inserter(cvect));

	random_shuffle(cvect.begin(),cvect.end());

	clist.clear();
	copy(cvect.begin(),cvect.end(),back_inserter(rlist));

	cvect.clear();

	return rlist;
}

void Population::SubadultMove(Coalition* coal) {
	list<Lion*> leos = coal->GetLions();
	list<Lion*>::iterator iter;

	Territory* origt = coal->GetLocation();

	// if our youngest is 2.5 years old, we can take nearby unoccupied territory
	// OLD (if we have any lions >=2.5 years old, we can take nearby unoccupied territory)
	if ((coal->GetLionsUnderAge(param.MaleReproduceAge,MALE)).empty()) 
		ProcureUnoccupiedTerritory(coal,param.SubadultMaleMoves);

	if (coal->GetLocation() != origt)
		coal->AddToOrderedVisit(coal->GetLocation());
}

void Population::NomadicMoveAndFight(Coalition* coal) {
	list<Lion*> leos;
	list<Lion*>::iterator liter;
	stringstream iss;

	// take X steps and see if can find a territory
	list<Territory*> already;

	// see if there is a free territory nearby (if old enough to procure)
	if (coal->GetAverageAge() >= param.MaleReproduceAge)
		ProcureUnoccupiedTerritory(coal,1);

	// if not, move and challenge
	if (coal->GetStatus() == NOMADIC) {
		coal->AddVisited(coal->GetLocation());			
		MoveToNeighboringTerritory(coal);
	}
	
	coal->AddToOrderedVisit(coal->GetLocation());
}

// return false if unsuccessful
bool Population::MoveToNeighboringTerritory(Coalition* coal) {
	stringstream iss;
	list<Territory*> nterr;
	Territory* gohere;
	list<Territory*>::iterator iter,iter2;
	list<Territory*> already = coal->GetVisited();

	// list of neighboring territories
	nterr = coal->GetLocation()->GetTerritoryAwayBy(1);
	list<Territory*> safety(nterr.begin(),nterr.end());

	// if nowhere to move, return
	if (nterr.empty()) return false;

	// remove from list all those that we have already visited
	for (iter=nterr.begin();iter!=nterr.end();) {
		if ((*iter)->IsInList(already)) {
			iter2 = iter;
			iter++;
			nterr.erase(iter2);
		}
		else
			iter++;
	}

	// if there are none left, we can revisit an old one
	if (nterr.empty())
		nterr = safety;

	// pick one at random and go there
	gohere = ChooseRandomTerritory(nterr);
	coal->SetLocation(gohere);

	iss.str("");
	iss << "        Moving to territory " << gohere->GetIDNumber() << endl;
	WriteToTranscript(iss.str());

	// encounter resident males if there are any (and if old enough to challenge)
	if (gohere->HasCoalition())
		AttackResidentCoalition(coal,gohere);

	return true;
}

void Population::AttackResidentCoalition(Coalition* coal,Territory* terr) {
	stringstream iss;
	
	//Territory* terr = coal->GetLocation();
	Coalition* res = terr->GetCoalition();

	double q = CalculateQ(coal,res);
	//double q = numnom / (numres * param.FightNomadAttacks[agenom][ageres]);

	list<Lion*> leos;
	list<Lion*>::iterator liter;
	iss.str("");
	iss << "        Confrontation between nomad coalition " << coal->GetIDNumber() << " [ ";
	leos = coal->GetLions();
	for (liter=leos.begin();liter!=leos.end();liter++)
		iss << (*liter)->GetAge() << " ";
	iss << "]" << endl;
	iss << "        and resident coalition " << res->GetIDNumber() << " [ ";
	leos = res->GetLions();
	for (liter=leos.begin();liter!=leos.end();liter++)
		iss << (*liter)->GetAge() << " ";
	iss << "]. Q=" << q << endl;
	WriteToTranscript(iss.str());
	if (q<1.0) {
		iss.str("");
		iss << "        No fight\n";
		WriteToTranscript(iss.str());
	}

	// if q<1.0, there is no fight

	// if q is between 1 and 1.1, then there's a fight and the residents win
	if (q >= 1.0 && q <= 1.1) 
		Fight(coal,res,res);

	// if q>1.1 then there's a fight and the nomads win
	else if (q > 1.1) {
		res->SetLocation(terr);
		Fight(coal,res,coal);
	}
	

}

double Population::CalculateQ(Coalition* att,Coalition* def) {
	// formula is q=number of attackers / (number of defenders * Q)
	// we average Q over all combinations of attacking and defending lions
	// (instead of picking an arbitrary oldest age of group or average of group age)
	// half years (e.g. 3.5) are considered rounded down
	list<Lion*> dlions = def->GetLions();
	list<Lion*> alions = att->GetLions();
	list<Lion*>::iterator diter, aiter;

	int numdef = int(dlions.size());
	int numatt = int(alions.size());

	if (numatt==0) 
		return 0;
	else if (numdef==0)
		return 2;

	int aage;
	int dage;
	double sum=0;
	for (aiter=alions.begin();aiter!=alions.end();aiter++) {		
		aage = int((*aiter)->GetAge());
		if (aage>9) aage=9;
		for (diter=dlions.begin();diter!=dlions.end();diter++) {
			dage = int((*diter)->GetAge());
			if (dage>9) dage=9;
			assert(aage<10);
			assert(dage<10);
			sum += param.FightNomadAttacks[aage][dage];
		}
	}
	double b = sum / (numdef*numatt);

	return (numatt / (numdef*b));
}

void Population::Fight(Coalition* att,Coalition* def,Coalition* win) {

	stringstream iss;
	iss.str("");
	if (att==win)
		iss << "        Attackers (nomads) win\n";
	else
		iss << "        Defenders (residents) win\n";
	WriteToTranscript(iss.str());
	
	double attSurvive = param.SurviveAttackingMaleLoses;
	double defSurvive = param.SurviveDefendingMaleWins;

	// attackers (nomads) win
	if (att==win) {
		TakeOverTerritory(att,def->GetLocation());
		attSurvive = param.SurviveAttackingMaleWins;
		defSurvive = param.SurviveDefendingMaleLoses;
	}

	list<Lion*> fighters;
	list<Lion*> markToDie;
	list<Lion*>::iterator iter;
	double r;

	// calculate attackers' mortality
	fighters = att->GetLions();
	for (iter=fighters.begin();iter!=fighters.end();iter++) {
		r =  double(rand())/(double(RAND_MAX) + 1.0);
		if (r > attSurvive)
			markToDie.push_back(*iter);
	}

	// calculate defenders' mortality
	fighters = def->GetLions();
	for (iter=fighters.begin();iter!=fighters.end();iter++) {
		r =  double(rand())/(double(RAND_MAX) + 1.0);
		if (r > defSurvive)
			markToDie.push_back(*iter);
	}
	
	// kill em off
	for (iter=markToDie.begin();iter!=markToDie.end();iter++) {
		iss.str("");
		iss << "        Male " << (*iter)->GetIDNumber() << " dies in fight. (Coalition ";
		iss << (*iter)->GetGroup()->GetIDNumber() << ")\n";
		WriteToTranscript(iss.str());

		Kill(*iter,FIGHT);
	}
	
}

Territory* Population::ChooseRandomTerritory(list<Territory*> terr) {

	int takeme,i;
	list<Territory*>::iterator iter;

	takeme = rand() % int(terr.size());
	iter = terr.begin();
	for (i=0;i<takeme;i++)
		iter++;

	return *iter;
}

void Population::ResidentMoveAndFight(Coalition* coal) {
	stringstream iss;
	list<Territory*>::iterator iter,iter2;
	list<Territory*> rterrs = coal->GetResidentTerritories();
	int tsize = int(rterrs.size());

	list<Territory*> nterr;
	list<Territory*> templist;

	bool takeanother = false;
	double r = double(rand())/(double(RAND_MAX) + 1.0);

	coal->ResetOrderedVisit();
	for (iter=rterrs.begin();iter!=rterrs.end();iter++)
		coal->AddToOrderedVisit(*iter);

	int nummales = int((coal->GetLions()).size());
	assert(nummales<15);
	assert(tsize+1<10);
	if (r < param.ResidentTakes[nummales][tsize+1])
		takeanother = true;

	if (!takeanother)
		return;

	// check neighboring territories of each resident territory
	for (iter=rterrs.begin();iter!=rterrs.end();) {
		coal->SetLocation(*iter);
		ProcureUnoccupiedTerritory(coal,1);

		if (int((coal->GetResidentTerritories()).size()) > tsize) 
			iter = rterrs.end();
		else
			iter++;
	}

	// if there aren't any unoccupied ones, try taking occupied ones
	if (int((coal->GetResidentTerritories()).size()) == tsize) {

		// get neighboring territory
		for (iter=rterrs.begin();iter!=rterrs.end();iter++) {
			templist = (*iter)->GetTerritoryAwayBy(1);
			nterr.splice(nterr.end(),templist);
		}

		// delete own territory out of the list
		for (iter=nterr.begin();iter!=nterr.end();) {
			if ((*iter)->GetCoalition() == coal) {
				iter2 = iter;
				iter++;
				nterr.erase(iter2);
			}
			else
				iter++;
		}
		
		if (!nterr.empty()) {
			list<Territory*> safety(nterr.begin(),nterr.end());
			Territory* tryme = ChooseRandomTerritory(safety);

			iss.str("");
			iss << "        Testing territory " << tryme->GetIDNumber() << endl;
			WriteToTranscript(iss.str());

			if (tryme->HasCoalition())
				AttackResidentCoalition(coal,tryme);
		}
	}
}

void Population::ProcureUnoccupiedTerritory(Coalition* coal,int awayby) {

	list<Territory*> terrs;
	list<Territory*>::iterator iter;
	Territory* found = NULL;
	stringstream iss;
	int i;

	// get list of neighboring territories
	for (i=1;i<=awayby;i++) {
		terrs = (coal->GetLocation())->GetTerritoryAwayBy(i);

		// see if they 1) have a lioness pride, and 2) don't have a resident coalition
		for (iter=terrs.begin();iter!=terrs.end();) {
			if ((*iter)->HasPride() && !(*iter)->HasCoalition()) {
				found = *iter;
				iter = terrs.end();
				i = awayby;
			}
			else
				iter++;
		}
	}

	// found one!
	if (found != NULL) {

		iss.str("");
		iss << "      Coalition " << coal->GetIDNumber() << " takes over FREE territory " << found->GetIDNumber() << endl;
		WriteToTranscript(iss.str());

		TakeOverTerritory(coal,found);
	}
}

void Population::TakeOverTerritory(Coalition* coal,Territory* terr) {

	list<Lion*> cubs,temp;
	list<Lion*>::iterator iter;
	list<Coalition*>::iterator citer;
	Lion* cub;
	list<Lion*> markedToDie;
	double r;
	stringstream iss;
	int i;
	double maxage;
	list<Lion*> defFem;

	iss.str("");
	iss << "      Coalition " << coal->GetIDNumber() << " takes over territory " << terr->GetIDNumber() << endl;
	WriteToTranscript(iss.str());

	// evict current coalition
	if (terr->HasCoalition()) {

		iss.str("");
		iss << "      Coalition " << terr->GetCoalition()->GetIDNumber() << " is evicted\n";
		WriteToTranscript(iss.str());

		terr->GetCoalition()->Evict(terr);
	}

	// re-assign coalition and territory info
	coal->SetLocation(terr);
	coal->SetStatus(RESIDENT);
	coal->AddResidentTerritory(terr);
	terr->SetCoalition(coal);

	// if no prides here, don't go on
	if (!terr->HasPride())
		return;

	// attack cubs
	Pride* prid = terr->GetPride();
	cubs = prid->GetCubs();

	// moms defend if there are cubs
	if (cubs.size() > 0) {
		defFem = prid->GetLionsOverAge(param.FemaleReproduceAge-0.01,FEMALE);

		for (iter=defFem.begin();iter!=defFem.end();iter++) {
			r =  double(rand())/(double(RAND_MAX) + 1.0);
	
			if (r >= param.SurviveDefendingFemale)
				markedToDie.push_back(*iter);
		}
	}

	for (iter=markedToDie.begin();iter!=markedToDie.end();iter++) {

		iss.str("");
		iss << "      Female ";
		iss << (*iter)->GetIDNumber() << " killed defending cubs (age " << (*iter)->GetAge();
		iss << ", group " << (*iter)->GetGroup()->GetIDNumber() << ")\n";
		WriteToTranscript(iss.str());

		Kill(*iter,FIGHT);
	}
	markedToDie.clear();


	// commit infanticide
	for (iter=cubs.begin();iter!=cubs.end();iter++) {
		cub = *iter;

		r =  double(rand())/(double(RAND_MAX) + 1.0);
		
		for (i=1;i<=param.CubCategoriesDefined;i++) {
			if (i==param.CubCategoriesDefined) {
				if (cub->GetSex() == MALE)
					maxage = param.SubadultMaleMinAge;
				else
					maxage = param.SubadultFemaleMinAge;
			}
			else {
				assert(i+1<5);
				maxage = param.CubMinAge[i+1];
			}

			assert(i<5);
			if (cub->GetAge() >= param.CubMinAge[i] && 
				cub->GetAge() < maxage) {
				if (r >= param.SurviveTakeoverCub[i])
					markedToDie.push_back(cub);
				i = param.CubCategoriesDefined + 1;
			}
		}
	}

	for (iter=markedToDie.begin();iter!=markedToDie.end();iter++) {

		iss.str("");
		iss << "      Infanticide in pride " << prid->GetIDNumber() << ": cub ";
		iss << (*iter)->GetIDNumber() << " killed (age " << (*iter)->GetAge();
		iss << ", group " << (*iter)->GetGroup()->GetIDNumber() << ")\n";
		WriteToTranscript(iss.str());

		Kill(*iter,INFANTICIDE);
	}

}

bool Population::HarvestSingleTrophy(Sex huntsex) {

	bool success = false;
	int j;
	int troph;
	list<Lion*>::iterator iter;
	list<Lion*> candidates;
	stringstream iss;

	candidates = GetHuntableLions(huntsex);

	if (!candidates.empty()) {
		troph = rand() % int(candidates.size());
		iter = candidates.begin();
		for (j=0;j<troph;j++)
			iter++;

		iss.str("");
		iss << "    Lion " << (*iter)->GetIDNumber() << " (age " << (*iter)->GetAge();
		if (huntsex==MALE)
			iss << ", male";
		else
			iss << ", female";
		iss << ") is hunted and killed. (Group ";
		iss << (*iter)->GetGroup()->GetIDNumber() << ")" << endl;
		WriteToTranscript(iss.str());

		Kill(*iter,HUNTED);
		success = true;
	}

	return success;
}


void Population::HarvestTrophies() {

	stringstream iss;

	// check to see what the population status is and switch strategies if appropriate
	int numHuntable = int(GetLions(MALE,param.TrophyMinAge,param.MaxAge+1).size()) +
		int(GetLions(FEMALE,param.TrophyMinAge,param.MaxAge+1).size());

	if (initialHuntingPopulation == 0) {
		initialHuntingPopulation = numHuntable;

		iss.str("");
		iss << "    Initial population of huntable males and females with min age ";
		iss << param.TrophyMinAge << " = " << numHuntable << endl;
		WriteToTranscript(iss.str());
	}
	else {
		iss.str("");
		iss << "    Huntable population = " << numHuntable << endl;
		WriteToTranscript(iss.str());
	}
	
	double switchAt = initialHuntingPopulation * param.SwitchHuntingStrategyAt;
	
	// if biannual, always do this
	// if only annual, check that it's a full year
	if (param.HuntingFrequency == ANNUAL &&
		double(int(timestep)) != timestep)
		return;

	// take as many trophies as quota allows
	numberHunted=0;
	Sex huntsex = MALE;
	double r;
	double sexError = param.TrophySexError;
	bool success;
	int i;
	int tries = param.TrophyQuota;
	int totalPop = numHuntable;
	int malesHunted = 0;

	if (param.IgnoreQuota) {
		tries = int(GetHuntableMales().size());
		if (param.TrophySexError < 1.0)
			tries = int(tries / (1.0 - param.TrophySexError)); // to account for females


	}

	for (i=0;i<tries;i++) {

		huntsex = MALE;
		r =  double(rand())/(double(RAND_MAX) + 1.0);
		if (r < sexError) // error in getting males, harvest a female
			huntsex = FEMALE;

		success = HarvestSingleTrophy(huntsex);
		if (success) {
			numberHunted++;
			if (huntsex == MALE)
				malesHunted++;

			totalPop--;
			if (currentHuntingStrategy == 1 && totalPop <= switchAt) {
				// stop hunting this turn
				i = tries;

				// switch strategy
				currentHuntingStrategy = 2;
				param.TrophyMinAge = param.TrophyMinAge2;
				param.TrophyQuota = param.TrophyQuota2;
				param.TrophySexError = param.TrophySexError2;
				param.HuntingFrequency = param.HuntingFrequency2;
				param.IgnoreQuota = param.IgnoreQuota2;

				iss.str("");
				iss << "    Population has fallen to " << totalPop << ". Switching hunting strategy." << endl;
				WriteToTranscript(iss.str());
			}
		}
	}	

	
		
	iss.str("");
	iss << "    Hunted " << malesHunted << " males and " << (numberHunted-malesHunted) << " females.";
	if (numberHunted > 0)
		iss << " (" << (100*(numberHunted-malesHunted)/numberHunted) << "% female)";
	iss << endl;
	WriteToTranscript(iss.str());
	

}

list<Lion*> Population::GetHuntableLions(Sex huntsex) {
	list<Lion*> toret;
	if (huntsex == MALE)
		toret = GetHuntableMales();
	else
		toret = GetHuntableFemales();
	return toret;
}

list<Lion*> Population::GetHuntableFemales() {

	list<Lion*> hlion;
	list<Lion*> clion;
	list<Pride*>::iterator iter;

	// go through each pride
	for (iter=prides.begin();iter!=prides.end();iter++) {

		// see if it's in huntable location
		if ((*iter)->GetLocation()->IsHuntable()) {

			// and the right age
			clion = (*iter)->GetLionsOverAge(param.TrophyMinAge-0.1,FEMALE);
			hlion.splice(hlion.end(),clion);
		}
	}

	return hlion;
}

list<Lion*> Population::GetHuntableMales() {
	list<Lion*> hlion;
	list<Lion*> clion;
	list<Lion*>::iterator liter;
	list<Coalition*>::iterator iter;
	list<Territory*> terrs;
	list<Territory*>::iterator titer;
	bool huntable;

	// go through each coalition
	for (iter=coalitions.begin();iter!=coalitions.end();iter++) {

		huntable = false;
		// check to see if in huntable location
		// residents -- check all locations
		if ((*iter)->GetStatus() == RESIDENT) {
			terrs = (*iter)->GetResidentTerritories();
			for (titer=terrs.begin();titer!=terrs.end();titer++) {
				if ((*titer)->IsHuntable())
					huntable = true;
			}
		}
		// nomads and subadults have only one location
		else {
			if ((*iter)->GetLocation()->IsHuntable())
				huntable = true;
		}

		// add all lions in coalition if in huntable territory, if they're of proper age
		if (huntable) {
			clion = (*iter)->GetLions();
			for (liter=clion.begin();liter!=clion.end();liter++) {
				if ((*liter)->GetAge() >= param.TrophyMinAge)
					hlion.push_back(*liter);
			}
		}
	}

	return hlion;
}

list<Lion*> Population::GetLions(double agemin,double agemax) {
	list<Lion*>::iterator iter;
	list<Lion*> toret;

	for (iter=lions.begin();iter!=lions.end();iter++) {
		if ((*iter)->GetAge() >= agemin &&
			(*iter)->GetAge() <= agemax)
			toret.push_back(*iter);
	}

	return toret;
}

list<Lion*> Population::GetLions(Sex sex,double agemin,double agemax) {

	list<Lion*>::iterator iter;
	list<Lion*> toret;

	for (iter=lions.begin();iter!=lions.end();iter++) {
		if ((*iter)->GetSex() == sex && 
			(*iter)->GetAge() >= agemin &&
			(*iter)->GetAge() <= agemax)
			toret.push_back(*iter);
	}

	return toret;
}

list<Lion*> Population::GetLions(list<Territory*> ters,Sex sex,double agemin,double agemax) {

	list<Lion*>::iterator iter;
	list<Lion*> toret;
	list<Lion*> poss = GetLions(ters,agemin,agemax);

	for (iter=poss.begin();iter!=poss.end();iter++) {
		if ((*iter)->GetSex() == sex)
			toret.push_back(*iter);
	}

	return toret;
}

list<Lion*> Population::GetLions(list<Territory*> ters,double agemin,double agemax) {

	list<Lion*>::iterator iter;
	list<Territory*>::iterator titer,titer2;
	list<Territory*> rester;
	list<Lion*> toret;
	list<Lion*> poss;
	list<Lion*> templist;
	list<Coalition*>::iterator citer;
	list<Pride*>::iterator piter;
	bool isin;

	// check prides
	for (piter=prides.begin();piter!=prides.end();piter++) {
		isin = false;
		for (titer=ters.begin();titer!=ters.end();) {
			if ((*titer) == (*piter)->GetLocation()) {
				isin = true;
				titer = ters.end();
			}
			else 
				titer++;
		}
		if (isin) {
			templist = (*piter)->GetLions();
			poss.splice(poss.end(),templist);
		}
	}
	// check coalitions
	for (citer=coalitions.begin();citer!=coalitions.end();citer++) {
		isin = false;
		for (titer=ters.begin();titer!=ters.end();) {
			if ((*titer) == (*citer)->GetLocation()) {
				isin = true;
				titer = ters.end();
			}
			else if ((*citer)->GetStatus() == RESIDENT) {
				rester = (*citer)->GetResidentTerritories();
				for (titer2=rester.begin();titer2!=rester.end();titer2++) {
					if ((*titer) == (*titer2)) 
						isin = true;
				}
				if (isin)
					titer = ters.end();
				else
					titer++;
			}
			else
				titer++;
		}
		if (isin) {
			templist = (*citer)->GetLions();
			poss.splice(poss.end(),templist);
		}
	}

	for (iter=poss.begin();iter!=poss.end();iter++) {
		if ((*iter)->GetAge() >= agemin &&
			(*iter)->GetAge() <= agemax)
			toret.push_back(*iter);
	}

	return toret;
}


list<Territory*> Population::GetTerritories(bool huntable) {
	list<Territory*> ters;
	list<Territory*>::iterator iter;
	for (iter=territories.begin();iter!=territories.end();iter++) 
		if (((*iter)->IsHuntable() && huntable) ||
			(!(*iter)->IsHuntable() && !huntable))
			ters.push_back(*iter);
	return ters;
}

void Population::CoalitionLosesTerritory(Coalition* coal) {
}


double Population::GetAveragePrideSize() {
	list<Pride*> pris = GetAllPrides();
	list<Pride*>::iterator iter;

	double sum = 0.0;
	for (iter=pris.begin();iter!=pris.end();iter++) 
		sum += double((*iter)->GetLionsOverAge(param.AdultFemaleMinAge-0.1,FEMALE).size());
	
	return (sum/double(pris.size()));
}

double Population::GetAverageResidentCoalitionSize() {
	list<Coalition*> coals = GetAllCoalitions();
	list<Coalition*>::iterator iter;

	double sum = 0.0;
	int counter = 0;
	for (iter=coals.begin();iter!=coals.end();iter++) {
		if ((*iter)->GetStatus() == RESIDENT) {
			sum += double((*iter)->GetLionsOverAge(2,MALE).size());
			counter++;
		}
	}

	return (sum/double(counter));
}

list<Coalition*> Population::GetCoalitions(MaleStatus stat) {
	list<Coalition*> toret;
	list<Coalition*>::iterator iter;

	for (iter=coalitions.begin();iter!=coalitions.end();iter++)
		if ((*iter)->GetStatus() == stat)
			toret.push_back(*iter);

	return toret;
}


void Population::TranscriptStatistics() {

	stringstream iss;
	int i;
	int c,d,nom,res;
	int diss,dise,disi;
	list<Territory*> terrs;
	list<Territory*>::iterator titer;
	Territory* terr;
	list<Lion*> leos;
	list<Lion*>::iterator liter;
	list<Coalition*> coals;
	list<Coalition*>::iterator citer;

	iss.str("");
	iss << "\n--- Step Summary ---\n";
	iss << "# Lions = " << lions.size() << endl;
	iss << "  Adult Females = " << GetLions(FEMALE,param.AdultFemaleMinAge,param.MaxAge).size() << endl;
	iss << "  Adult Males = " << GetLions(MALE,param.AdultMaleMinAge,param.MaxAge).size() << endl;
	iss << "  Subadult Females = " << GetLions(FEMALE,param.SubadultFemaleMinAge,param.AdultFemaleMinAge-0.1).size() << endl;
	iss << "  Subadult Males = " << GetLions(MALE,param.SubadultMaleMinAge,param.AdultMaleMinAge-0.1).size() << endl;
	iss << "  Cubs = " << (GetLions(FEMALE,0,param.SubadultFemaleMinAge-0.1).size() + 
		GetLions(MALE,0,param.SubadultMaleMinAge-0.1).size()) << endl;
	iss << "# Prides = " << prides.size() << endl;
	iss << "# Resident Coalitions = " << GetCoalitions(RESIDENT).size() << endl;
	iss << "# Nomadic Coalitions = " << GetCoalitions(NOMADIC).size() << endl;
	iss << "# Subadult Coalitions = " << GetCoalitions(SUBADULT).size() << endl;
	iss << endl;

	iss << "Territories\n";
	for (titer=territories.begin();titer!=territories.end();titer++) {
		terr = *titer;
		iss << terr->GetIDNumber() << ": ";
		if (terr->HasPride()) {
			iss << "Pride " << terr->GetPride()->GetIDNumber() << " (";
			leos = terr->GetPride()->GetLions();
			for (liter=leos.begin();liter!=leos.end();liter++) {
				if (liter!=leos.begin())
					iss << ",";
				if ((*liter)->GetSex() == MALE)
					iss << "M";
				else
					iss << "F";
				iss << (*liter)->GetAge();
			}
			iss << ")";
		}
		else 
			iss << "No Pride";

		if (terr->HasCoalition()) {
			iss << "; Coalition " << terr->GetCoalition()->GetIDNumber() << " (";
			leos = terr->GetCoalition()->GetLions();
			for (liter=leos.begin();liter!=leos.end();liter++) {
				if (liter!=leos.begin())
					iss << ",";
				iss << "M" << (*liter)->GetAge();
			}
			iss << ")\n";
		}
		else
			iss << "; No Resident Coaliton\n";
	}
	coals = GetCoalitions(RESIDENT);
	for (citer=coals.begin();citer!=coals.end();citer++) {
		iss << "Resident Coalition " << (*citer)->GetIDNumber() << ": ";
		terrs = (*citer)->GetResidentTerritories();
		for (titer=terrs.begin();titer!=terrs.end();titer++) {
			if (titer!=terrs.begin())
				iss << ",";
			iss << (*titer)->GetIDNumber();
		}
		iss << endl;
	}

	iss << "\nDeaths:\n";
	iss << "            c1 c2 c3 c4 sf af sm mn mr";
	iss << "\n background ";
	for (i=0;i<9;i++) {
		c = deaths[i][0];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << "\n  abandoned ";
	for (i=0;i<9;i++) {
		c = deaths[i][1];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << "\ninfanticide ";
	for (i=0;i<9;i++) {
		c = deaths[i][2];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << "\n      fight ";
	for (i=0;i<9;i++) {
		c = deaths[i][3];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << "\n     hunted ";
	for (i=0;i<9;i++) {
		c = deaths[i][4];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << "\n  lost land ";
	for (i=0;i<9;i++) {
		c = deaths[i][5];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << "\n    disease ";
	for (i=0;i<9;i++) {
		c = deaths[i][6];
		if (c<10) iss << " ";
		iss << c << " ";
	}
	iss << endl;

	iss << "Mortality this turn:\n";

	c = 0;
	for (i=0;i<6;i++)
		c += deaths[CUB0][i];
	if (param.CubCategoriesDefined > 1)
		d = int(GetLions(param.CubMinAge[1],param.CubMinAge[2]-0.1).size());
	else
		d = int(GetLions(MALE,param.CubMinAge[1],param.SubadultMaleMinAge-0.1).size()) +
			int(GetLions(FEMALE,param.CubMinAge[1],param.SubadultFemaleMinAge-0.1).size());
	iss << "Cubs1 - " << c << " died out of " << (d + c);
	if (c+d>0)
		iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
	else iss << endl;

	if (param.CubCategoriesDefined>=2) {
		c = 0;
		for (i=0;i<6;i++)
			c += deaths[CUB1][i];
		if (param.CubCategoriesDefined > 2)
				d = int(GetLions(param.CubMinAge[2],param.CubMinAge[3]-0.1).size());
		else
			d = int(GetLions(MALE,param.CubMinAge[2],param.SubadultMaleMinAge-0.1).size()) +
				int(GetLions(FEMALE,param.CubMinAge[2],param.SubadultFemaleMinAge-0.1).size());
		iss << "Cubs2 - " << c << " died out of " << (d + c);
		if (c+d>0)
			iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
		else iss << endl;
	}

	if (param.CubCategoriesDefined>=3) {
		c = 0;
		for (i=0;i<6;i++)
			c += deaths[CUB2][i];
		if (param.CubCategoriesDefined > 3)
			d = int(GetLions(param.CubMinAge[3],param.CubMinAge[4]-0.1).size());
		else
			d = int(GetLions(MALE,param.CubMinAge[3],param.SubadultMaleMinAge-0.1).size()) +
				int(GetLions(FEMALE,param.CubMinAge[3],param.SubadultFemaleMinAge-0.1).size());
		iss << "Cubs3 - " << c << " died out of " << (d + c);
		if (c+d>0)
			iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
		else iss << endl;
	}

	if (param.CubCategoriesDefined==4) {
		c = 0;
		for (i=0;i<6;i++)
			c += deaths[CUB2][i];
		d = int(GetLions(MALE,param.CubMinAge[4],param.SubadultMaleMinAge-0.1).size()) +
			int(GetLions(FEMALE,param.CubMinAge[4],param.SubadultFemaleMinAge-0.1).size());
		iss << "Cubs4 - " << c << " died out of " << (d + c);
		if (c+d>0)
			iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
		else iss << endl;
	}

	c = 0;
	for (i=0;i<6;i++)
		c += deaths[SUBADULT_F][i];
	d = int(GetLions(FEMALE,param.SubadultFemaleMinAge,param.AdultFemaleMinAge-0.1).size());
	iss << "Subadult females - " << c << " died out of " << (d + c);
	if (c+d>0)
		iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
	else iss << endl;

	c = 0;
	for (i=0;i<6;i++)
		c += deaths[SUBADULT_M][i];
	d = int(GetLions(MALE,param.SubadultMaleMinAge,param.AdultMaleMinAge-0.1).size());
	iss << "Subadult males - " << c << " died out of " << (d + c);
	if (c+d>0)
		iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
	else iss << endl;

	c = 0;
	for (i=0;i<6;i++)
		c += deaths[ADULT_F][i];
	d = int(GetLions(FEMALE,param.AdultFemaleMinAge,param.MaxAge).size());
	iss << "Adult females - " << c << " died out of " << (d + c);
	if (c+d>0)
		iss << " (" << int(1000*c/(c+d))/10.0 << "% mortality)\n";
	else iss << endl;

	c = 0;
	for (i=0;i<6;i++)
		c += deaths[ADULT_M_NOMADIC][i];
	leos = GetLions(MALE,param.AdultMaleMinAge,param.MaxAge);
	list<Lion*>::iterator iter;
	nom=0;
	res=0;
	for (iter=leos.begin();iter!=leos.end();iter++) {
		if (((Coalition*)((*iter)->GetGroup()))->GetStatus() == NOMADIC)
			nom++;
		else
			res++;
	}
	iss << "Adult nomadic males - " << c << " died out of " << (nom + c);
	if (c+nom>0)
		iss << " (" << int(1000*c/(c+nom))/10.0 << "% mortality)\n";
	else iss << endl;

	c = 0;
	for (i=0;i<6;i++)
		c += deaths[ADULT_M_RESIDENT][i];
	iss << "Adult resident males - " << c << " died out of " << (res + c);
	if (c+res>0)
		iss << " (" << int(1000*c/(c+res))/10.0 << "% mortality)\n";
	else iss << endl;

	iss << endl;
	iss << "Disease:\n";

	diss=0; dise=0; disi=0;
	for (liter=lions.begin();liter!=lions.end();liter++) {
		switch ((*liter)->GetDiseaseState()) {
			case SUSCEPTIBLE: diss++; break;
			case EXPOSED: dise++; break;
			case INFECTIOUS: disi++; break;
		}
	}
	iss << "   Susceptible: " << diss;
	if (lions.size()>0) iss << " (" << (diss/((1.0)*lions.size())) << "%)";
	iss << "\n   Exposed: " << dise;
	if (lions.size()>0) iss << " (" << (dise/((1.0)*lions.size())) << "%)";
	iss << "\n   Infectious: " << disi;
	if (lions.size()>0) iss << " (" << (disi/((1.0)*lions.size())) << "%)";

	iss << "\n\nTotal died\n   of other than disease: " << disdeath[0];
	iss << "\n   of other than disease yet were infectious: " << disdeath[1];
	for (i=2;i<40;i++) 
		iss << "\n   infectious " << (i-1)/2.0 << " years: " << disdeath[i];

	iss << "\n--------------------\n\n";
	
	WriteToTranscript(iss.str());
	
}


void Population::Statistics() {


	stringstream iss;
	int i,j,k;
	double fi;
	int ddiss[3] = {0,0,0};
	int ddise[3] = {0,0,0};
	int ddisi[3] = {0,0,0};
	int loc,reg,tot;
	double dmin[3];
	double dmax[3];
	double dsum[3];
	int dcount[3];
	double prev;
	int mf=0;
	int ph=0;
	list<Lion*>::iterator liter;
	list<Territory*>::iterator titer;
	list<Lion*> nlions;
	list<Lion*> clions;
	list<Lion*> slions;
	
	iss.str("");

	switch (param.StatisticsType) {

		case STAT_RESCOAL:
			iss << GetAverageResidentCoalitionSize() << ",";
			break;

		case STAT_PRIDE:
			iss << GetAveragePrideSize() << ",";
			break;

		case STAT_HUNTED:
			iss << numberHunted << ",";
			break;

		case STAT_DEATH_AGES:
			mf=0;
			ph=0;
			if (param.StatisticsSexUsed && param.StatisticsSex == FEMALE)
				mf = 1;
			if (param.StatisticsHuntUsed && param.StatisticsHuntable == true)
				ph = 1;
		
			if (param.StatisticsSexUsed && param.StatisticsHuntUsed)
				for (i=0;i<((param.MaxAge+1)*2);i++) {
					assert(mf<2);
					assert(ph<2);
					assert(i<50);
					iss << age_deaths[mf][ph][i] << ",";
				}
			else if (param.StatisticsSexUsed)
				for (i=0;i<((param.MaxAge+1)*2);i++) { 
					assert(mf<2);
					assert(i<50);
					iss << (age_deaths[mf][0][i]+age_deaths[mf][1][i]) << ",";
				}
			else if (param.StatisticsHuntUsed)
				for (i=0;i<((param.MaxAge+1)*2);i++) {
					assert(ph<2);
					assert(i<50);
					iss << (age_deaths[0][ph][i]+age_deaths[1][ph][i]) << ",";
				}
			else 
				for (i=0;i<((param.MaxAge+1)*2);i++)  {
					assert(i<50);
					iss << (age_deaths[0][0][i]+age_deaths[1][0][i]+
							age_deaths[0][1][i]+age_deaths[1][1][i]) << ",";
				}

			iss << endl;
			break;

		case STAT_DEMOGRAPHICS:
			for (fi=0;fi<param.MaxAge+0.5;fi+=0.5) 
				iss << GetLions(FEMALE,fi,fi).size() << ",";	
			for (fi=0;fi<param.MaxAge+0.5;fi+=0.5) 
				iss << GetLions(MALE,fi,fi).size() << ",";	
			iss << endl;
			break;

		case STAT_FOOD_INFECT:
			for (i=0;i<3;i++) {
				dmin[i] = 1;
				dmax[i] = 0;
				dsum[i] = 0;
				dcount[i] = 0;
			}
			for (titer=territories.begin();titer!=territories.end();titer++) {
				loc = (*titer)->GetIDNumber();
				if (loc<300) reg = 0; // north
				else if (loc<500) reg = 1; // central
				else reg = 2; // south

				prev = (*titer)->GetDiseasedBuffalo();
				assert(reg<3);
				if (prev<dmin[reg]) dmin[reg] = prev;
				if (prev>dmax[reg]) dmax[reg] = prev;
				dsum[reg] += prev;
				dcount[reg] ++;
			}

			// north: min, max, ave, count; central: min, max, ave, count; south: min, max, ave, count
			for (i=0;i<3;i++) {
				iss << dmin[i] << "," << dmax[i] << "," << dsum[i]/dcount[i] << "," << dcount[i] << ",";
			}
			// total average
			iss << (dsum[0]+dsum[1]+dsum[2])/(dcount[0]+dcount[1]+dcount[2]) << endl;
			break;

		case STAT_DISEASE:
			tot=0;

			// for the first line, just show the scenario number and disease parameter values
			// order: scenario number, L, B, O, E, I
			if (timestep==0.5) {
				iss << param.ScenarioNum << "," << param.LionTransmission << "," << param.BuffaloTransmission << ",";
				iss << param.OutgroupEncounter << "," << param.lambdaE << "," << param.lambdaI << endl;
			}

			// now the disease information
			for (liter=lions.begin();liter!=lions.end();liter++) {
				if ((*liter)->GetAge() >= 0.5) { // don't count very young animals in disease counts
					loc = (*liter)->GetGroup()->GetLocation()->GetIDNumber();
					if (loc<300) reg = 0; // north
					else if (loc<500) reg = 1; // central
					else reg = 2; // south

					assert(reg<3);
					switch ((*liter)->GetDiseaseState()) {
						case SUSCEPTIBLE: ddiss[reg]++; break;
						case EXPOSED: ddise[reg]++; break;
						case INFECTIOUS: ddisi[reg]++; break;
					}

					if (timestep==64.0) {
						switch (reg) {
							case 0: nlions.push_back(*liter); break;
							case 1: clions.push_back(*liter); break; 
							case 2: slions.push_back(*liter); break;
						}
					}
				}
			}
			// north S,E,I,total; central S,E,I,total; south S,E,I,total; population total; deaths in state E; conversions E->I; E or I in 1999 in each of north, central, south
			for (i=0;i<3;i++) {
				iss << ddiss[i] << "," << ddise[i] << "," << ddisi[i] << "," << ddiss[i]+ddise[i]+ddisi[i] << ",";
				tot+= ddiss[i]+ddise[i]+ddisi[i];
			}
			iss << tot << "," << countEdeaths << "," << countEtoI;

			// for certain year(s), sample the lions for disease; lots of magic numbers here...
			if (timestep==64.0) { // 1999

				int npos = 0;
				int cpos = 0;
				int spos = 0;
				int rnum;

				// north
				for (i=0;i<22;i++) { // 22
					if (nlions.size()==0) { // not enough lions
						npos = -1; 
						i = 99;
					}
					else {
						rnum = rand() % int(nlions.size());
						liter = nlions.begin();
						for (j=0;j<rnum;j++) 
							liter++;
						if ((*liter)->GetDiseaseState() != SUSCEPTIBLE) 
							npos++;
						nlions.erase(liter);
					}
				}

				// central
				for (i=0;i<39;i++) { // 39
					if (clions.size()==0) { // not enough lions
						cpos = -1; 
						i = 99;
					}
					else {
						rnum = rand() % int(clions.size());
						liter = clions.begin();
						for (j=0;j<rnum;j++) 
							liter++;
						if ((*liter)->GetDiseaseState() != SUSCEPTIBLE) 
							cpos++;
						clions.erase(liter);
					}
				}

				// south
				for (i=0;i<64;i++) { //64
					if (slions.size()==0) { // not enough lions
						spos = -1; 
						i = 99;
					}
					else {
						rnum = rand() % int(slions.size());
						liter = slions.begin();
						for (j=0;j<rnum;j++) 
							liter++;
						if ((*liter)->GetDiseaseState() != SUSCEPTIBLE) 
							spos++;
						slions.erase(liter);
					}
				}

				iss << "," << npos << "," << cpos << "," << spos;
			}

			iss << endl;
			break;
	
		case STAT_SURVIVE:
			double agemin = 0;
			double agemax = param.MaxAge;

			if (param.StatisticsAgeUsed) {
				agemin = param.StatisticsAge1;
				if (!param.StatisticsAgeMin)
					agemax = param.StatisticsAge2;
			}

			if (param.StatisticsHuntUsed) {
				list<Territory*> ters = GetTerritories(param.StatisticsHuntable); // hunted or not

				if (param.StatisticsSexUsed) 
					iss << GetLions(ters,param.StatisticsSex,agemin,agemax).size();
				else
					iss << GetLions(ters,agemin,agemax).size();
			}
			else {
				if (param.StatisticsSexUsed)
					iss << GetLions(param.StatisticsSex,agemin,agemax).size();
				else 
					iss << GetLions(agemin,agemax).size();
			}
			iss << ",";
			break;
	}


	WriteToStats(iss.str());

	// clear death indices
	for (i=0;i<9;i++)
		for (j=0;j<7;j++) {
			assert(i<9);
			assert(j<7);
			deaths[i][j] = 0;
		}
	for (i=0;i<((param.MaxAge+1)*2);i++)
		for (j=0;j<2;j++)
			for (k=0;k<2;k++) {
				assert(k<2);
				assert(j<2);
				assert(i<50);
				age_deaths[k][j][i] = 0;
			}

			
}




bool Population::IsSubsetOfList(Lion* sub,list<Lion*> big) {
	list<Lion*> leos;
	leos.push_back(sub);
	return IsSubsetOfList(leos,big);
}
bool Population::IsSubsetOfList(list<Lion*> sub,list<Lion*> big) {
	list<Lion*>::iterator iterb,iters;
	bool toret = true;
	bool found;

	for (iters=sub.begin();iters!=sub.end();) {
		found = false;
		for (iterb=big.begin();iterb!=big.end();) {
			if (*iterb == *iters) {
				found = true;
				iterb=big.end();
			}
			else
				iterb++;
		}
		if (!found) {
			toret = false;
			iters=sub.end();
		}
		else 
			iters++;
	}

	return toret;
}


void Population::LandLoss() {

	// if we're past the threshhold, stop
	if (timestep > param.LandLossYears)
		return;

	// calculate if we actually lose any land
	int loseme = 0;
	double currentLoss = 1 - double(territories.size())/double(param.OrigNumTerritories);
	double targetLoss = timestep*param.LandLossPercent/(param.LandLossYears);
	while (currentLoss < targetLoss) {
		loseme++;
		currentLoss = 1 - double(territories.size()-loseme)/double(param.OrigNumTerritories);
	}

	stringstream iss;
	iss.str("");
	iss << "   (Losing " << loseme << " territories.)\n";
	iss << "   (Total loss will be " << int(currentLoss*1000)/10.0 << "%)\n";
	iss << "   (Target loss is " << int(targetLoss*1000)/10.0 << "%)\n\n";
	WriteToTranscript(iss.str());

	int i;
	for (i=0;i<loseme;i++) {
		if (param.LandLossMethod == EDGES)
			DeleteEdgeTerritory();
		else
			DeleteRandomTerritory();
	}

}


list<Territory*> Population::GetEdgeTerritories() {

	int nnum;
	int min = int(((territories.front())->GetTerritoryAwayBy(1)).size());
	list<Territory*> toReturn;
	list<Territory*>::iterator iter;

	// get the number of least connections
	for (iter=territories.begin();iter!=territories.end();iter++) {
		nnum = int(((*iter)->GetTerritoryAwayBy(1)).size());
		if (nnum < min)
			min = nnum;
	}

	// get all the territories of that size
	for (iter=territories.begin();iter!=territories.end();iter++) {
		if (int(((*iter)->GetTerritoryAwayBy(1)).size()) == nnum)
			toReturn.push_back(*iter);
	}

	return toReturn;
}

void Population::DeleteRandomTerritory() {
	int num = rand() % int(territories.size());
	int i=0;
	list<Territory*>::iterator iter;
	for (iter=territories.begin();i<num;iter++)
		i++;
	int id = (*iter)->GetIDNumber();

	stringstream iss;
	iss.str("");
	iss << "    Deleting Territory # " << id << "\n";
	WriteToTranscript(iss.str());

	DeleteTerritory(id);
}

void Population::DeleteEdgeTerritory() {

	list<Territory*> edges = GetEdgeTerritories();
	
	int num = rand() % int(edges.size());
	int i=0;
	list<Territory*>::iterator iter;
	for (iter=edges.begin();i<num;iter++)
		i++;
	int id = (*iter)->GetIDNumber();

	stringstream iss;
	iss.str("");
	iss << "    Deleting Territory # " << id << "\n";
	WriteToTranscript(iss.str());

	DeleteTerritory(id);
}

void Population::DeleteTerritory(int id) {

	// first remove the lions on this territory
	DisplaceLionsInTerritory(id);

	// then remove the pointers to this territory
	Territory* ot = GetTerritory(id);
	list<Territory*> nterr;
	list<Territory*>::iterator iter;
	int i;
	for (i=1;i<6;i++) {
		nterr = ot->GetTerritoryAwayBy(i);
		for (iter=nterr.begin();iter!=nterr.end();iter++) {
			(*iter)->RemoveNeighbor(id);
		}
	}

	// last delete the territory
	for (iter=territories.begin();iter!=territories.end();) {
		if ((*iter) == ot) {
			territories.erase(iter);
			iter = territories.end();
		}
		else
			iter++;
	}
	delete(ot);

}

void Population::DisplaceLionsInTerritory(int id) {

	stringstream iss;
	list<Lion*> leos;
	list<Lion*>::iterator iter;
	list<Lion*>::iterator tempiter;
	list<Territory*> oneigh;
	int close = 0;
	int i;
	Coalition* ocoal;
	list<Coalition*>::iterator iter3;
	list<Pride*>::iterator iter2;
	list<Pride*> allfem;
	list<Lion*> ocubs;
	list<Lion*> templist;

	Territory* ot = GetTerritory(id);

	// if we're a true island, everyone dies
	for (i=1;i<6;i++)
		close += int(ot->GetTerritoryAwayBy(i).size());
	if (close==0) {
		for (iter=lions.begin();iter!=lions.end();) {
			if ((*iter)->GetGroup()->GetLocation() == ot) {
				tempiter=iter;
				iter++;
				Kill(*tempiter,LANDGONE);
			}
			else
				iter++;
		}
		return;
	}

	// kill cubs (if any)
	list<Territory*> oneTerr;
	oneTerr.push_back(ot);
	ocubs = GetLions(oneTerr,MALE,0,param.SubadultMaleMinAge-0.1);
	iter = ocubs.end();
	templist = GetLions(oneTerr,FEMALE,0,param.SubadultFemaleMinAge-0.1);
	ocubs.splice(iter,templist);
	for (iter=ocubs.begin();iter!=ocubs.end();iter++) {
		iss.str("");
		iss << "      Cub " << (*iter)->GetIDNumber() << " dies. Age = " << (*iter)->GetAge() << "\n";
		WriteToTranscript(iss.str());
		Kill(*iter,LANDGONE);
	}

//	bool didmove;
	Territory* gohere;
	// kick out resident males (if any)
	if (ot->HasCoalition()) {
		ocoal = ot->GetCoalition();
		ocoal->Evict(ot);
		ot->SetCoalition(NULL);
			// if the coalition has another territory, it'll move there, else 
			// forceably move it
		if (ocoal->IsEvicted()) {
			iss.str("");
			iss << "      Resident coalition " << ocoal->GetIDNumber() << " is now nomadic and moves ";
				oneigh.clear();
				i = 0;
				while (oneigh.empty()) {
					i++;
					oneigh = ot->GetTerritoryAwayBy(i);
				}	
				gohere = ChooseRandomTerritory(oneigh);
				ocoal->SetLocation(gohere);
				iss << "to territory " << gohere->GetIDNumber() << endl; 
				WriteToTranscript(iss.str());
		}
	}

	// kick out the nomad males (& promote subadults to nomadic)
	for (iter3=coalitions.begin();iter3!=coalitions.end();iter3++) {
		if ((*iter3)->GetLocation() == ot) {
			// subadults are now nomadic
			if ((*iter3)->GetStatus() == SUBADULT) {
				(*iter3)->SetStatus(NOMADIC);
				iss.str("");
				iss << "      Subadult Coalition " << (*iter3)->GetIDNumber() << " is promoted to Nomadic\n";
				WriteToTranscript(iss.str());
			}

			// move them to a neighboring territory
			iss.str("");
			iss << "      Coalition " << (*iter3)->GetIDNumber() << " moves ";

			// if didn't move for some reason, force the move
				oneigh.clear();
				i = 0;
				while (oneigh.empty()) {
					i++;
					oneigh = ot->GetTerritoryAwayBy(i);
				}	
				gohere = ChooseRandomTerritory(oneigh);
				(*iter3)->SetLocation(gohere);

				iss << "to territory " << gohere->GetIDNumber() << endl; 
				WriteToTranscript(iss.str());
		}
	}

	// kick out resident females
	Pride* opride;
	if (ot->HasPride()) {
		opride = ot->GetPride();
		opride->SetWandering(true);
		ot->SetPride(NULL);
		wanderingFemales.push_back(opride);
			// remove from pride list
		for (iter2=prides.begin();iter2!=prides.end();) {
			if (*iter2 == opride) {
				prides.erase(iter2);
				iter2 = prides.end();
			}
			else
				iter2++;
		}
		iss.str("");
		iss << "      Resident pride " << opride->GetIDNumber() << " uprooted.\n        Lioness ages: ";
		list<Lion*> ofems = opride->GetLions();
		for (iter=ofems.begin();iter!=ofems.end();iter++)
			iss << (*iter)->GetAge() << " ";
		iss << "\n";
		WriteToTranscript(iss.str());
	}

	// kick out wandering females
	for (iter2=wanderingFemales.begin();iter2!=wanderingFemales.end();iter2++) {
		if ((*iter2)->GetLocation() == ot) {
	
			// pick one at random and go there
			oneigh.clear();
			i = 0;
			while (oneigh.empty()) {
				i++;
				oneigh = ot->GetTerritoryAwayBy(i);
			}	
			gohere = ChooseRandomTerritory(oneigh);
			(*iter2)->SetLocation(gohere);

			iss.str("");
			iss << "      Wandering pride " << (*iter2)->GetIDNumber() << " displaced to territory ";
			iss << gohere->GetIDNumber() << "\n";
			WriteToTranscript(iss.str());
		}
	}
}



void Population::IdentifyIngroupAndOutgroup() {

	list<Lion*>::iterator iter;
	list<LionGroup*> ingrp;
	list<LionGroup*> templist;
	list<LionGroup*> outgrp;
	list<LionGroup*> exa;
	list<LionGroup*> temp;
	list<LionGroup*>::iterator giter,niter;
	LionGroup* grp;
	Territory* myt;
	list<Territory*> terr;
	list<Territory*> neigh;
	list<Territory*> templist2;
	list<Territory*>::iterator titer,titer2;
	Coalition* coal;
	stringstream iss;
	Pride* pri;

	// each group stores info on who's ingroup and outgroup
	exa = GetAllGroups();
	for (giter=exa.begin();giter!=exa.end();giter++) {

		ingrp.clear();
		outgrp.clear();
		neigh.clear();
		terr.clear();

		grp = (*giter);
		myt = grp->GetLocation();

		// ingroup
		ingrp.push_back(grp);
		
		// if in a pride, include res coalition (if there is one) and all assocated subadult groups
		if (grp->GetSex() == FEMALE) {
			pri = (Pride*)grp;
			if (myt->GetPride() == grp && myt->HasCoalition()) 
				ingrp.push_back(grp->GetLocation()->GetCoalition());

			// get associated subadult groups
			templist = pri->GetSubadultMaleGroups();
			ingrp.splice(ingrp.end(),templist);
		}
		// if in a resident coalition, include all prides and associated subadult groups
		if (grp->GetSex() == MALE) {
			coal = (Coalition*)grp;
			if (coal->GetStatus() == RESIDENT) {
				terr = coal->GetResidentTerritories();
				for (titer=terr.begin();titer!=terr.end();titer++) {
					ingrp.push_back((*titer)->GetPride());
					// also get associated subadult groups
					templist = (*titer)->GetPride()->GetSubadultMaleGroups();
					ingrp.splice(ingrp.end(),templist);
				}
			}
			// if in a subadult coalition, include mom-pride, brother subadult coalitions and resident coalitions
			else if (coal->GetStatus() == SUBADULT) {
				myt = coal->GetLocation();
				if (myt->HasPride()) {
					ingrp.push_back(myt->GetPride());
					temp = myt->GetPride()->GetSubadultMaleGroups();
					for (niter=temp.begin();niter!=temp.end();niter++)
						if (*niter != grp)
							ingrp.push_back(*niter);
				}
				if (myt->HasCoalition()) {
					ingrp.push_back(myt->GetCoalition());
				}
			}
			else terr.push_back(myt);
		}
		else terr.push_back(myt);

		(*giter)->SetInGroup(ingrp);

		// outgroup
		// for outgroup, start with all neighbors, male and female
		// for each home territory, 
		for (titer=terr.begin();titer!=terr.end();titer++) {
			templist2 = (*titer)->GetTerritoryAwayBy(1);
			neigh.splice(neigh.end(),templist2);
		}

		// for all neighboring territories
		for (titer=neigh.begin();titer!=neigh.end();titer++) {
			if ((*titer)->HasPride()) {
				outgrp.push_back((*titer)->GetPride());
				templist = (*titer)->GetPride()->GetSubadultMaleGroups();
				outgrp.splice(outgrp.end(),templist);
			}
			if ((*titer)->HasCoalition())
				outgrp.push_back((*titer)->GetCoalition());
		}
		// now look to see who came through the current territory
		for (niter=exa.begin();niter!=exa.end();niter++) {
			terr = (*niter)->GetOrderedVisitList();
			for (titer=terr.begin();titer!=terr.end();titer++)
				if ((*titer) == myt)
					outgrp.push_back(*niter);
		}

		// remove duplicates
		outgrp = DedupeGroups(outgrp);

		// remove ingroups
		temp = ingrp;
		outgrp = RemoveGroups(outgrp,temp);

		(*giter)->SetOutGroup(outgrp);
	}
}

list<LionGroup*> Population::GetAllGroups() {

	list<LionGroup*> toRet;
	list<Pride*>::iterator piter;
	list<Coalition*>::iterator citer;
	for (piter=prides.begin();piter!=prides.end();piter++)
		toRet.push_back(*piter);
	for (piter=wanderingFemales.begin();piter!=wanderingFemales.end();piter++)
		toRet.push_back(*piter);
	for (citer=coalitions.begin();citer!=coalitions.end();citer++)
		toRet.push_back(*citer);

	return toRet;
}

list<LionGroup*> Population::DedupeGroups(list<LionGroup*> gps) {
	list<LionGroup*> toRet;
	list<LionGroup*>::iterator iter,iter2;
	bool found;

	for (iter=gps.begin();iter!=gps.end();iter++) {
		found = false;
		for (iter2=toRet.begin();iter2!=toRet.end();) {
			if (*iter == *iter2) {
				found = true;
				iter2 = toRet.end();
			}
			else
				iter2++;
		}
		if (!found)
			toRet.push_back(*iter);
	}

	return toRet;
}

list<LionGroup*> Population::RemoveGroups(list<LionGroup*> big,list<LionGroup*> lit) {
	list<LionGroup*> toRet;
	list<LionGroup*>::iterator iter,iter2;
	bool found;

	for (iter=big.begin();iter!=big.end();iter++) {
		found = false;
		for (iter2=lit.begin();iter2!=lit.end();) {
			if (*iter == *iter2) {
				found = true;
				iter2 = lit.end();
			}
			else iter2++;
		}
		if (!found)
			toRet.push_back(*iter);
	}

	return toRet;
}

void Population::UpdateDisease() {

	list<LionGroup*> exa;
	list<LionGroup*>::iterator giter;
	list<Lion*> leos;
	list<Lion*>::iterator liter;
	int infcons;
	double prob;
	double r;
	stringstream iss;
	
	iss << endl << "  Disease" << endl;
	WriteToTranscript(iss.str());

	// first update the buffalo disease (if changing)
	if (param.BuffaloDisease == LOGISTIC) 
		UpdateBuffaloDisease();

	// transitions from S to E

	IdentifyIngroupAndOutgroup();

	// go through all groups and count up infected ingroup and outgroup contacts
	exa = GetAllGroups();
	for (giter=exa.begin();giter!=exa.end();giter++) {
		(*giter)->CalculateInfectiousContacts();
	}

	// mom-cub transmission
	leos = GetLions(0,0.5);  // these are all nursing cubs in their first year of life
	for (liter=leos.begin();liter!=leos.end();liter++) {
		if ((*liter)->GetMom() != NULL) // make sure cub has a mom
			if ((*liter)->GetDiseaseState() == SUSCEPTIBLE &&
				(*liter)->GetMom()->GetDiseaseState() == INFECTIOUS) {

				r = double(rand())/(double(RAND_MAX) + 1.0);
				if (r < param.MaternalTransmission) {
					(*liter)->SetDiseaseState(EXPOSED);

					iss.str("");
					iss << "    Lion " << (*liter)->GetIDNumber() << " (age " << 
						(*liter)->GetAge() << ") is exposed to disease through mom" << endl;
					iss << "      and will be infectious at age "  <<
						(*liter)->GetAgeOfInfectious() << endl;
					WriteToTranscript(iss.str());
				}
			}
	}

	leos = GetAllLions();

	// in-group transmission, assumes contact with all in-group members is definite
	for (liter=leos.begin();liter!=leos.end();liter++) {
		if ((*liter)->GetDiseaseState() == SUSCEPTIBLE) {
			infcons = (*liter)->GetInfectiousIns();	
			prob = 1.0 - pow(1.0-param.LionTransmission,infcons); // cumulative transmission rate

			r = double(rand())/(double(RAND_MAX) + 1.0);
			if (r < prob) {
				(*liter)->SetDiseaseState(EXPOSED);

				iss.str("");
				iss << "    Lion " << (*liter)->GetIDNumber() << " (age " << 
					(*liter)->GetAge() << ", group " << (*liter)->GetGroup()->GetIDNumber() << 
					") is exposed to disease through in-group transmission" << endl;
				iss << "      and will be infectious at age "  <<
						(*liter)->GetAgeOfInfectious() << endl;
				WriteToTranscript(iss.str());
			}
		}
	}

	// out-group transmission
	for (liter=leos.begin();liter!=leos.end();liter++) {
		if ((*liter)->GetDiseaseState() == SUSCEPTIBLE) {
			infcons = (*liter)->GetInfectiousOuts();	
			prob = 1.0 - pow(1.0-param.LionTransmission,
							 infcons*param.OutgroupEncounter); // cumulative transmission rate

			r = double(rand())/(double(RAND_MAX) + 1.0);
			if (r < prob) {
				(*liter)->SetDiseaseState(EXPOSED);

				iss.str("");
				iss << "    Lion " << (*liter)->GetIDNumber() << " (age " << 
						(*liter)->GetAge() << ") is exposed to disease through out-group transmission" << endl;
				iss << "      and will be infectious at age "  <<
						(*liter)->GetAgeOfInfectious() << endl;
				WriteToTranscript(iss.str());

			}
		}
	}

	// food transmission 
	for (liter=leos.begin();liter!=leos.end();liter++) {
		if ((*liter)->GetDiseaseState() == SUSCEPTIBLE &&
			(*liter)->GetAge() >= 0.5) { // eat meat starting second half of first year

			// average territory numbers over all visited territories			
			prob = param.BuffaloTransmission * (*liter)->GetAveDiseasedDietBuffalo();

			// buffalo all miraculously cured in 2010
			//if (timestep > 75)
			//	prob = 0;

			r = double(rand())/(double(RAND_MAX) + 1.0);
			if (r < prob) {
				(*liter)->SetDiseaseState(EXPOSED);

				iss.str("");
				iss << "    Lion " << (*liter)->GetIDNumber() << " (age " << 
						(*liter)->GetAge() << ") is exposed to disease through through eating infected prey" << endl;
				iss << "      and will be infectious at age "  <<
						(*liter)->GetAgeOfInfectious() << endl;
				WriteToTranscript(iss.str());
			}
		}
	}

	// transition from E to I
	for (liter=leos.begin();liter!=leos.end();liter++) {
		if ((*liter)->GetDiseaseState() == EXPOSED &&
			(*liter)->GetAge() > (*liter)->GetAgeOfExposed()) { // don't allow S->E->I in same timestep

			// modeled as a Poisson process with same chance lambdaE each timestep
			r = double(rand())/(double(RAND_MAX) + 1.0);
			if (r<param.lambdaE) {
				(*liter)->SetDiseaseState(INFECTIOUS);
				countEtoI++;
			}

			iss.str("");
			iss << "    Lion " << (*liter)->GetIDNumber() << " transitions from E to I disease state" << endl;
			WriteToTranscript(iss.str());
		}

	}	

}

void Population::UpdateBuffaloDisease() {

	list<Territory*>::iterator titer;

	// for each territory, determine its (buffalo) disease prevalence
	for (titer=territories.begin();titer!=territories.end();titer++) {
		(*titer)->CalculateDisease(timestep-param.NoDiseaseYears);
	}
}

