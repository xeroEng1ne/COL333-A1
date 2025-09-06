#include "utils.h"

// to generate a random number between range [l,r] inclusive
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
int random_number(int l, int r) {
	uniform_int_distribution<int> dist(l, r);
	int x = dist(rng);
	return x;
}

// returns the total value of a trip
double getTripValue(Trip& trip,const ProblemData& problem,vector<double>& food_delivered_so_far,vector<double>& other_delivered_so_far){
    double marginal_value_gained=0.0;
    for(const auto& drop : trip.drops){
        const Village& village=problem.villages[drop.village_id-1];
        double max_food_needed=village.population*9.0;
        double food_room_left=max(0.0,max_food_needed-food_delivered_so_far[village.id]);
        double food_in_this_drop=drop.dry_food+drop.perishable_food;
        double effective_food_this_drop=min(food_in_this_drop,food_room_left);

        double effective_vp=min((double)drop.perishable_food,effective_food_this_drop);
        marginal_value_gained+=effective_vp*problem.packages[1].value;

        double remaining_effective_food=effective_food_this_drop-effective_vp;
        double effective_vd=min((double)drop.dry_food,remaining_effective_food);
        marginal_value_gained+=effective_vd*problem.packages[0].value;

        double max_other_needed=village.population*1.0;
        double other_room_left=max(0.0,max_other_needed-other_delivered_so_far[village.id]);
        double effective_vo=min((double)drop.other_supplies,other_room_left);
        marginal_value_gained+=effective_vo*problem.packages[2].value;
    }
    return marginal_value_gained;
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

double getTripWeight(Trip& trip,const ProblemData& problem){
    double dry_food_weight=problem.packages[0].weight;
    double perishable_food_weight=problem.packages[1].weight;
    double other_supplies_weight=problem.packages[2].weight;

    double total_weight=0.0;
    total_weight+=trip.dry_food_pickup*dry_food_weight;
    total_weight+=trip.perishable_food_pickup*perishable_food_weight;
    total_weight+=trip.other_supplies_pickup*other_supplies_weight;

    return total_weight;
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


