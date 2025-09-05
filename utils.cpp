#include "utils.h"

// to generate a random number between range [l,r] inclusive
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
int random_number(int l, int r) {
	uniform_int_distribution<int> dist(l, r);
	int x = dist(rng);
	return x;
}

// returns the total value of a trip
double getTripValue(Trip &trip, const ProblemData &problem) {

    int val=0;
    // for a village, if more than 9 times the stranders amount of food is dropped, then no additional value is gained
    for(const auto drops : trip.drops){
        Village vil = problem.villages[drops.village_id-1];
        int n = vil.population;
        
        int foodDrop=drops.dry_food+drops.perishable_food;
        int dry_food_capped=drops.dry_food;
        int perishable_food_capped=drops.perishable_food;

        if(foodDrop>9*n){
            double scale =(double)(9*n)/foodDrop;
            dry_food_capped=drops.dry_food*scale;
            perishable_food_capped=drops.perishable_food*scale;
        }

        val+=(dry_food_capped*problem.packages[0].value);
        val+=(perishable_food_capped*problem.packages[1].value);
        val+=(drops.other_supplies*problem.packages[2].value);
    }

    return val;

}

// return the total distance travelled in a trip from a home city
double getTripDistance(Trip &trip, const ProblemData &problem, const Point &homeCity) {

	if (trip.drops.empty())
		return 0.0;

	double totDist = 0.0;
	Point currPos = homeCity;
	for (const auto &drop : trip.drops) {
		// todo: make this faster by using map
		auto &village = problem.villages[drop.village_id-1];
		totDist += distance(currPos, village.coords);
		currPos = village.coords;
	}

	totDist += distance(currPos, homeCity);
	return totDist;
}

vector<double> getPackagesNumber(Trip& trip, const ProblemData& problem){
    double d=0.0,p=0.0,o=0.0;
    for(const auto drop : trip.drops){
            d+=drop.dry_food;
            p+=drop.perishable_food;
            o+=drop.other_supplies;
        }

    return {d,p,o};
}

vector<Village> giveVillages(Trip& trip, const ProblemData& problem){

    vector<Village> vec;
    for(const auto drop : trip.drops){
        Village v = problem.villages[drop.village_id-1];
        vec.push_back(v);
    }
    return vec;
}


