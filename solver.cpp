#include "solver.h"
#include <iostream>
#include "utils.h"
#include <set>
#include <algorithm>

using namespace std;

// ----------- EVALUATION FUNCTION ------------

double evaluate(Solution& solution, const ProblemData& problem) {
    vector<double> food_delivered_to_village(problem.villages.size() + 1, 0.0);
    vector<double> other_delivered_to_village(problem.villages.size() + 1, 0.0);

    double total_value_gained = 0.0;
    double total_trip_cost = 0.0;

    // Loop through the entire solution to calculate costs and values
    for (const auto& plan : solution) {
        const Helicopter& curr_heli = problem.helicopters[plan.helicopter_id - 1];
        const Point& homeCity = problem.cities[curr_heli.home_city_id - 1];

        for (const auto& trip : plan.trips) {
            // Rule: Empty trips have zero cost.
            if (trip.drops.empty()) {
                continue;
            }

            // --- Calculate Trip Cost ---
            double tripDistance = getTripDistance(const_cast<Trip&>(trip), problem, homeCity);
            total_trip_cost += curr_heli.fixed_cost + (curr_heli.alpha * tripDistance);

            // --- Calculate Value Gained from this Trip's Drops ---
            for (const auto& drop : trip.drops) {
                const Village& village = problem.villages[drop.village_id - 1];
                
                // --- Food Value Calculation (Cumulative) ---
                double max_food_needed = village.population * 9.0;
                // CRITICAL: Calculate how much more food the village still needs
                double food_room_left = max(0.0, max_food_needed - food_delivered_to_village[village.id]);
                
                double food_in_this_drop = drop.dry_food + drop.perishable_food;
                double effective_food_this_drop = min(food_in_this_drop, food_room_left);

                // Perishable-first logic
                double effective_vp = min((double)drop.perishable_food, effective_food_this_drop);
                total_value_gained += effective_vp * problem.packages[1].value;

                double remaining_effective_food = effective_food_this_drop - effective_vp;
                double effective_vd = min((double)drop.dry_food, remaining_effective_food);
                total_value_gained += effective_vd * problem.packages[0].value;
                
                // Update the total food delivered to this village for subsequent calculations
                food_delivered_to_village[village.id] += food_in_this_drop;


                // --- Other Supplies Value Calculation (Cumulative) ---
                double max_other_needed = village.population * 1.0;
                double other_room_left = max(0.0, max_other_needed - other_delivered_to_village[village.id]);
                double effective_vo = min((double)drop.other_supplies, other_room_left);
                total_value_gained += effective_vo * problem.packages[2].value;

                // Update the total "other" delivered to this village
                other_delivered_to_village[village.id] += drop.other_supplies;
            }
        }
    }

    return total_value_gained - total_trip_cost;
}

// -----------NEIGHBOURHOOD FUNCTION ----------

Solution neighbour(Solution& currSoln, const ProblemData& problem){

    int H=currSoln.size(); // number of helicopters
    Solution bestSoln = currSoln;
    double best=evaluate(currSoln,problem);

    for(int i=0;i<H;i++){

        Solution candidate = currSoln;

        // current helicopter
        int heliId = candidate[i].helicopter_id;
        Helicopter currHeli = problem.helicopters[heliId-1];
        int homeId = currHeli.home_city_id;
        Point home = problem.cities[homeId-1];

        if(candidate[i].trips.empty()) continue;

        int number_of_trips = (int)candidate[i].trips.size();

        // select a random trip to modify
        int tripId = random_number(0,number_of_trips-1);
        Trip& trip = candidate[i].trips[tripId];

        // select what kind of move we wanna use to modify the trip
        // moves can be of "quality" type or "quality" type
        int move = random_number(1, 6);

        double total_distance_travlled=0; // <= Dmax
        for(auto& trip : candidate[i].trips){
            double tripDistance = getTripDistance(trip,problem,home);
            total_distance_travlled+=tripDistance;
        }

        if(move==1){
            // delete a village from the trip or delete the empty trip

            //debug
            cout<<"debug : Move 1"<<endl;

            if(!trip.drops.empty()){

                int drop_to_remove=0;
                if(trip.drops.size()>1){
                    drop_to_remove=random_number(0,trip.drops.size()-1);
                }

                Drop removed_drop = trip.drops[drop_to_remove];
                trip.drops.erase(trip.drops.begin()+drop_to_remove);

                // Update the trip's pickup
                trip.dry_food_pickup-=removed_drop.dry_food;
                trip.perishable_food_pickup-=removed_drop.perishable_food;
                trip.other_supplies_pickup-=removed_drop.other_supplies;
            }
        }

        else if(move==2){
            // add a village to the trip

            //debug
            cout<<"//debug: Move 2"<<endl;

            if(trip.drops.empty() || problem.villages.size() <= trip.drops.size()){
                continue;
            }

            set<int> present_villages;
            for(const auto& drop : trip.drops){
                present_villages.insert(drop.village_id);
            }

            vector<int> available_villages;
            for(const auto& village : problem.villages){
                if(present_villages.find(village.id)==present_villages.end()){
                    available_villages.push_back(village.id);
                }
            }

            if(available_villages.empty()){
                continue;
            }

            //Pick a random village to add
            int new_village_id = available_villages[random_number(0,available_villages.size()-1)];
            double current_trip_weight=getTripWeight(trip,problem);
            double rem_capacity=currHeli.weight_capacity-current_trip_weight;
            Drop new_drop;
            new_drop.village_id=new_village_id;
            //initialize
            new_drop.dry_food=0;
            new_drop.perishable_food=0;
            new_drop.other_supplies=0;

            // Option A : Fill helicopter more
            if(rem_capacity>1e-5){
                //debug
                cout<<"Move 2 : option A"<<endl;

                double pw=problem.packages[1].weight;
                int num_p_to_add=rem_capacity/pw;
                if(num_p_to_add>0){
                    new_drop.perishable_food=num_p_to_add;
                    trip.perishable_food_pickup+=num_p_to_add;
                }
            }
            // Option B : Rearrange some cargo
            else if(!trip.drops.empty()){
                //debug
                cout<<"Move 2 : option B"<<endl;
                int source_id=random_number(0,trip.drops.size()-1);
                Drop& source=trip.drops[source_id];

                // Move 25%
                new_drop.dry_food=source.dry_food*0.25;
                source.dry_food-=new_drop.dry_food;
                new_drop.perishable_food=source.perishable_food*0.25;
                source.perishable_food-=new_drop.perishable_food;
                new_drop.other_supplies=source.other_supplies*0.25;
                source.other_supplies-=new_drop.other_supplies;
            }


            // Insert the new drop
            int insertion_index=random_number(0,trip.drops.size()-1);
            Trip new_trip=trip;
            new_trip.drops.insert(new_trip.drops.begin()+insertion_index,new_drop);

            double new_trip_distance=getTripDistance(new_trip,problem,home);
            double old_trip_distance=getTripDistance(trip,problem,home);
            double new_helic_total_dist=total_distance_travlled-old_trip_distance+new_trip_distance;

            //debug
            cout<<"//debug:   Checking add of Village "<<new_village_id<<": new trip_dist="<< new_trip_distance<<"/"<<currHeli.distance_capacity<<", new total_dist="<<new_helic_total_dist <<"/"<<problem.d_max<<endl;

            if(new_trip_distance<=currHeli.distance_capacity && new_helic_total_dist<=problem.d_max){
                trip=new_trip;
                cout<<"//debug SUCCESS"<<endl;
            }
        }

        else if(move==3){
            // change drop values of a village
            
            //debug
            cout<<"//debug: Move 3"<<endl;

            if(!trip.drops.empty()){
                int drop_index=0;
                if(trip.drops.size()>1){
                    drop_index=random_number(0,trip.drops.size()-1);
                }
                Drop& drop=trip.drops[drop_index];

                // upgrade d->p
                //debug
                cout<<"Move 3 : attempt upgrade"<<endl;
                if(drop.dry_food>0){
                    drop.dry_food--;
                    trip.dry_food_pickup--;
                    drop.perishable_food++;
                    trip.perishable_food_pickup++;

                    double new_weight=getTripWeight(trip, problem);
                    if(new_weight>currHeli.weight_capacity){
                        //revert
                        drop.dry_food++;
                        trip.dry_food_pickup++;
                        drop.perishable_food--;
                        trip.perishable_food_pickup--;
                    }
                }
            }
        }

        else if(move==4){
            // change the cargo that is carried by the helicopter
            
            //debug
            cout<<"//deubg: Move 4"<<endl;

            if(!trip.drops.empty()){

                int drop_index=0;
                if(trip.drops.size()>1){
                    drop_index=random_number(0,trip.drops.size()-1);
                }
                double dw=problem.packages[0].weight;
                double pw=problem.packages[1].weight;
                double ow=problem.packages[2].weight;

                double curr_weight=getTripWeight(trip, problem);
                double rem_capacity=currHeli.weight_capacity-curr_weight;


                if(rem_capacity<pw && trip.dry_food_pickup>0){
                    double needed_capacity=pw-rem_capacity;
                    int d_to_remove=ceil(needed_capacity/dw);

                    if(trip.dry_food_pickup>=d_to_remove){
                        trip.dry_food_pickup-=d_to_remove;
                        trip.perishable_food_pickup++;

                        if(!trip.drops.empty()){
                            trip.drops[drop_index].dry_food-=d_to_remove;
                            trip.drops[drop_index].perishable_food++;
                        }
                    }
                }
                else if(rem_capacity>=pw){
                    trip.perishable_food_pickup++;
                    if(!trip.drops.empty()){
                        trip.drops[drop_index].perishable_food++;
                    }
                }
                else if(rem_capacity>=dw){
                    trip.dry_food_pickup++;
                    if(!trip.drops.empty()){
                        trip.drops[drop_index].dry_food++;
                    }
                }
                else if(rem_capacity>=ow){
                    trip.other_supplies_pickup++;
                    if(!trip.drops.empty()){
                        trip.drops[drop_index].other_supplies++;
                    }
                }
            }
        }

        else if(move==5){
            // delete a trip -> changes the numbers of trip

            //debug
            cout<<"//debug: Move 5"<<endl;
            if(!candidate[i].trips.empty()){
                int trip_to_remove=0;
                if(candidate[i].trips.size()>1){
                    trip_to_remove=random_number(0,candidate[i].trips.size()-1);
                }
                candidate[i].trips.erase(candidate[i].trips.begin()+trip_to_remove);
            }
        }
        else if(move==6){
            // add a new trip

            //debug
            cout<<"//debug: Move "<<endl;

            double dmax_remaining=problem.d_max-total_distance_travlled;

            if(dmax_remaining>=1.0){
                vector<int> reachable_villages;
                for(auto& village:problem.villages){
                    double round_trip_dist=2*distance(home,village.coords);
                    if(round_trip_dist<=currHeli.distance_capacity){
                        reachable_villages.push_back(village.id);
                    }
                }

                if(!reachable_villages.empty()){
                    int target_village_id=reachable_villages[random_number(0, reachable_villages.size()-1)];
                    const Village& village=problem.villages[target_village_id-1];
                    double round_trip_dist=2*distance(home,village.coords);

                    vector<double> food_delivered(problem.villages.size()+1,0.0);
                    vector<double> other_delivered(problem.villages.size()+1,0.0);

                    //TODO: can we make this efficient?
                    for(const auto& plan:candidate){
                        for(const auto& trip: plan.trips){
                            for(const auto& drop: trip.drops){
                                if(drop.village_id==target_village_id){
                                    food_delivered[target_village_id]+=drop.dry_food+drop.perishable_food;
                                    other_delivered[target_village_id]+=drop.other_supplies;
                                }
                            }
                        }
                    }

                    Trip new_trip=Trip();
                    double dw=problem.packages[0].weight;
                    int d=currHeli.weight_capacity/dw;

                    new_trip.dry_food_pickup=d;
                    
                    new_trip.drops.push_back({target_village_id,d,0,0});

                    double trip_value=getTripValue(new_trip,problem,food_delivered,other_delivered);
                    double trip_cost=currHeli.fixed_cost+(currHeli.alpha*round_trip_dist);
                    double trip_score=trip_value-trip_cost;

                    if(trip_score>0){
                        candidate[i].trips.push_back({new_trip});
                    }
                }
            }

        }

        int score=evaluate(candidate, problem);
        if(score>=best){
            bestSoln=std::move(candidate);
            best=score;
        }

    }

    return bestSoln;
}

//------------------ Initial Greedy Solution ----------------------//

Solution generateInitialSolution(const ProblemData& problem){

    Solution soln = Solution();
    int H=problem.helicopters.size();

    double dw=problem.packages[0].weight;
    double pw=problem.packages[1].weight;
    double ow=problem.packages[2].weight;
    double dv=problem.packages[0].value;
    double pv=problem.packages[1].value;
    double ov=problem.packages[2].value;

    for(int h=0;h<H;h++){

        HelicopterPlan heli_plan=HelicopterPlan();
        heli_plan.helicopter_id=h+1;

        const Helicopter& currHeli = problem.helicopters[h];

        Trip trip;
        Drop drop;


        bool assigned=false;
        Point home=problem.cities[currHeli.home_city_id-1];
        for(const auto& village : problem.villages){
            if(2*distance(home,village.coords)<=currHeli.distance_capacity){
                drop.village_id=village.id;
                assigned=true;
                break;
            }
        }
        
        if(!assigned){
            soln.push_back(heli_plan);
            continue;
        }

        double capacity_to_use = currHeli.weight_capacity;
        
        // Calculate value per weight for each package type
        double dry_value_per_weight = dv / dw;
        double perishable_value_per_weight = pv / pw;
        double other_value_per_weight = ov / ow;
        
        // Create pairs of (value_per_weight, package_type) and sort by efficiency
        vector<pair<double, int>> efficiency;
        efficiency.push_back({dry_value_per_weight, 0}); // 0 = dry food
        efficiency.push_back({perishable_value_per_weight, 1}); // 1 = perishable food
        efficiency.push_back({other_value_per_weight, 2}); // 2 = other supplies
        
        sort(efficiency.rbegin(), efficiency.rend()); // Sort in descending order
        
        int d = 0, p = 0, o = 0;
        double remaining_capacity = capacity_to_use;
        
        // Allocate packages based on efficiency
        for(auto& eff : efficiency) {
            int package_type = eff.second;
            if(package_type == 0) { // dry food
                int max_dry = remaining_capacity / dw;
                d = max_dry;
                remaining_capacity -= d * dw;
            } else if(package_type == 1) { // perishable food
                int max_perishable = remaining_capacity / pw;
                p = max_perishable;
                remaining_capacity -= p * pw;
            } else { // other supplies
                int max_other = remaining_capacity / ow;
                o = max_other;
                remaining_capacity -= o * ow;
            }
        }

        drop.dry_food=d;
        drop.perishable_food=p;
        drop.other_supplies=o;

        trip.dry_food_pickup=d;
        trip.perishable_food_pickup=p;
        trip.other_supplies_pickup=o;

        trip.drops.push_back(drop);
        heli_plan.trips.push_back(trip);
        soln.push_back(heli_plan);
    }

    return soln;
}

// -------------------------------------------------------------------//


//------------------------- MAIN LOOP -------------------------------//

using clk = chrono::steady_clock;

Solution solve(const ProblemData& problem){

    cout<< "Starting solver..."<<endl;

    Solution currSoln = generateInitialSolution(problem);
    double hval = evaluate(currSoln, problem);

    double time = problem.time_limit_minutes;
    auto start=clk::now();
    auto end=start+chrono::duration<double>(time*60.0-1.0);

    uint64_t iters=0;
    while(true){
        //greedy hill climibing
        Solution neighbourState=neighbour(currSoln, problem);
        double neighbour_hval = evaluate(neighbourState,problem);

        if(neighbour_hval>=hval){
            currSoln=neighbourState;
            hval=neighbour_hval;
        }

        // check time
        if((++iters & 0xFFF)==0){
            if(clk::now()>=end) break;
        }
    }

    cout<<"Solver finished."<<endl;
    cout<<"Evaluation for the solution is : "<<evaluate(currSoln,problem)<<endl;
    return currSoln;

}

