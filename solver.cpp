#include "solver.h"
#include <iostream>
#include "utils.h"
#include <set>
#include <algorithm>

using namespace std;

// ----------- EVALUATION FUNCTION ------------

double evaluate(Solution& solution, const ProblemData& problem){
    double tot_value=0.0;
    double tot_trip_cost=0.0;

    for(auto& plan : solution){

        Helicopter curr_heli=problem.helicopters[plan.helicopter_id-1];

        const Point& homeCity = problem.cities[curr_heli.home_city_id-1];

        for(auto& trip : plan.trips){
            tot_value += getTripValue(trip,problem);
            double tripDistance=getTripDistance(trip,problem,homeCity);
            double tripCost=curr_heli.fixed_cost+(curr_heli.alpha*tripDistance);

            tot_trip_cost+=tripCost;
        }
    }

    return tot_value-tot_trip_cost;
}

// -----------NEIGHBOURHOOD FUNCTION ----------

Solution neighbour(Solution& currSoln, const ProblemData& problem){

    int H=currSoln.size(); // number of helicopters
    Solution bestSoln = currSoln;
    int best=evaluate(currSoln,problem);

    for(int i=0;i<H;i++){

        Solution candidate = currSoln;

        // current helicopter
        int heliId = candidate[i].helicopter_id;
        Helicopter currHeli = problem.helicopters[heliId-1];
        int homeId = currHeli.home_city_id;
        Point home = problem.cities[homeId-1];

        if(candidate[i].trips.empty()) continue;

        int number_of_trips = candidate[i].trips.size();

        // select a random trip to modify
        int tripId = random_number(0,number_of_trips-1);
        Trip& trip = candidate[i].trips[tripId];

        // select what kind of move we wanna use to modify the trip
        // moves can be of "quality" type or "quality" type
        int move = random_number(1, 6);

        double total_distance_travlled=0; // <= Dmax
        for(auto trip : candidate[i].trips){
            double tripDistance = getTripDistance(trip,problem,home);
            total_distance_travlled+=tripDistance;
        }
        double curr_distance = getTripDistance(trip,problem,home);

        if(move==1){
            // delete a village from the trip or delete the empty trip
            trip.drops.pop_back();
        }
        else if(move==2){
            // add a village to the trip
            set<int> present_villages;
            for(auto drop : trip.drops){
                present_villages.insert(drop.village_id);
            }

            int vil=problem.villages.size();
            vector<double> packageNumbers = getPackagesNumber(trip, problem);

            for(auto drops : trip.drops){
                present_villages.insert(drops.village_id);
            }

            for(int v=1;v<=vil;v++){
                if(present_villages.find(v)==present_villages.end()){
                    Trip newTrip = trip;
                    Drop newDrop = Drop();
                    newDrop.village_id=v;

                    int remd=trip.dry_food_pickup-packageNumbers[0];
                    int remp=trip.perishable_food_pickup-packageNumbers[1];
                    int remo=trip.perishable_food_pickup-packageNumbers[2];
                    newDrop.dry_food=remd;
                    newDrop.perishable_food=remp;
                    newDrop.other_supplies=remo;

                    newTrip.drops.push_back(newDrop);
                    double new_distance = getTripDistance(newTrip,problem,home);
                    // check if it is valid
                    if(new_distance<=currHeli.distance_capacity && new_distance-curr_distance+total_distance_travlled<=problem.d_max){
                        trip=newTrip;
                        break;
                    }
                }
            }

        }
        else if(move==3){
            // change drop values of a village
            for(auto& drop : trip.drops){
                if(drop.dry_food>0){
                    drop.dry_food--;
                    drop.perishable_food++;
                }
                else if(drop.perishable_food>0){
                    drop.perishable_food--;
                    drop.dry_food++;
                }
            }
        }
        // else if(move==4){
        //     // change the cargo that is carried by the helicopter
        //     int d=trip.dry_food_pickup;
        //     int p=trip.perishable_food_pickup;
        //     int o=trip.other_supplies_pickup;
        //     int dw=problem.packages[0].weight;
        //     int pw=problem.packages[1].weight;
        //     int ow=problem.packages[2].weight;
        //
        //     int curr_weight=d*dw+p*pw+o*ow;
        //     int rem_capacity=currHeli.weight_capacity-curr_weight;
        //
        //     if(rem_capacity>=pw){
        //         trip.perishable_food_pickup++;
        //     }
        //     else if(trip.dry_food_pickup>0){
        //         while(trip.dry_food_pickup>0 && rem_capacity<pw){
        //             trip.dry_food_pickup--;
        //             rem_capacity+=dw;
        //             trip.perishable_food_pickup++;
        //         }
        //     }
        //     else if(rem_capacity>=dw){
        //         trip.dry_food_pickup++;
        //     }
        //     else if(rem_capacity>=ow){
        //         trip.other_supplies_pickup++;
        //     }
        // }

        else if(move==5){
            // delete a trip -> changes the numbers of trip
            if(!candidate[i].trips.empty()) candidate[i].trips.pop_back();
        }
        else if(move==6){
            // add a new trip
            Trip newTrip = Trip();
            newTrip.dry_food_pickup=(currHeli.weight_capacity/2)/problem.packages[0].weight;
            newTrip.perishable_food_pickup=(currHeli.weight_capacity/2)/problem.packages[1].weight;
            newTrip.other_supplies_pickup=0;
            Solution bestSoln = currSoln;

            Drop newDrop= Drop();
            int v = problem.villages.size();
            newDrop.village_id=random_number(1,v);
            newDrop.dry_food=newTrip.dry_food_pickup;
            newDrop.perishable_food=newTrip.perishable_food_pickup;
            newDrop.other_supplies=0;

            newTrip.drops.push_back(newDrop);
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

        Trip trip = Trip();
        Drop drop = Drop();

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
        
        std::sort(efficiency.rbegin(), efficiency.rend()); // Sort in descending order
        
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
    int hval = evaluate(currSoln, problem);

    double time = problem.time_limit_minutes;
    auto start=clk::now();
    auto end=start+chrono::duration<double>(time*60.0-1.0);

    uint64_t iters=0;
    while(true){
        //greedy hill climibing
        Solution neighbourState=neighbour(currSoln, problem);
        int neighbour_hval = evaluate(neighbourState,problem);

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

