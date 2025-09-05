
#include "solver.h"
#include <iostream>
#include "utils.h"
#include <set>

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

// for debugging
// vector<Solution> neighbours;

// ------------------- Extras ---------------------------------

// void printSolution(const Solution& solution, const ProblemData& problem){
//     cout<<"-------------------------------------------------"<<endl;
//     double totalScore=evaluate(const_cast<Solution&>(solution),problem);
//
//     for(const auto& plan : solution){
//         cout<<" [Helicopter "<<plan.helicopter_id<<"]"<<endl;
//         if(plan.trips.empty()){
//             cout<<"     - No trips assigned"<<endl;
//             continue;
//         }
//         int tripNum=1;
//         for(const auto& trip:plan.trips){
//             cout<<" - Trip "<<tripNum++<<":"<<endl;
//             cout<<" Pickup: "<<trip.dry_food_pickup<<"d, "<<trip.perishable_food_pickup<<"p, "<<trip.other_supplies_pickup<<"o"<<endl;
//             if(trip.drops.empty()){
//                 cout<<" - No drops"<<endl;
//             }
//             for(const auto& drop : trip.drops){
//                 cout<<" -> Drop at Village "<<drop.village_id<<": "
//                     <<drop.dry_food<<"d, "
//                     <<drop.perishable_food<<"p, "
//                     <<drop.other_supplies<<"o"<<endl;
//             }
//         }
//     }
//     cout<<"-------------------------------------------------"<<endl;
// }

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
        else if(move==4){
            // to fix: never changes the drop amount
            // change the cargo that is carried by the helicopter
            int d=trip.dry_food_pickup;
            int p=trip.perishable_food_pickup;
            int o=trip.other_supplies_pickup;
            double dw=problem.packages[0].weight;
            double pw=problem.packages[1].weight;
            double ow=problem.packages[2].weight;

            double curr_weight=d*dw+p*pw+o*ow;
            double rem_capacity=currHeli.weight_capacity-curr_weight;

            if(rem_capacity>=pw){
                trip.perishable_food_pickup++;
            }
            else if(trip.dry_food_pickup>0){
                while(trip.dry_food_pickup>0 && rem_capacity<pw){
                    trip.dry_food_pickup--;
                    rem_capacity+=dw;
                    trip.perishable_food_pickup++;
                }
            }
            else if(rem_capacity>=dw){
                trip.dry_food_pickup++;
            }
            else if(rem_capacity>=ow){
                trip.other_supplies_pickup++;
            }
        }
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

        // neighbours.push_back(candidate);
    }

    // cout<<"Best neighbour is :"<<endl;
    // printSolution(const_cast<Solution&>(bestSoln),problem);
    return bestSoln;
}

//------------------ Initial Greedy Solution ----------------------//

Solution generateInitialSolution(const ProblemData& problem){

    Solution soln = Solution();
    int H=problem.helicopters.size();

    int dw=problem.packages[0].weight;
    int pw=problem.packages[1].weight;
    int ow=problem.packages[2].weight;

    for(int h=0;h<H;h++){

        HelicopterPlan heli_plan=HelicopterPlan();
        heli_plan.helicopter_id=h+1;

        const Helicopter& currHeli = problem.helicopters[h];

        Trip trip = Trip();
        Drop drop = Drop();

        bool assigned=false;
        Point home=problem.cities[currHeli.home_city_id-1];
        for(auto village : problem.villages){
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

        int change=random_number(0,currHeli.weight_capacity/4);

        int d = (currHeli.weight_capacity/2-change)/dw;
        int p = (currHeli.weight_capacity/2-change)/pw;
        int o = (currHeli.weight_capacity - d*dw - p*pw)/ow;

        drop.dry_food=d;
        drop.perishable_food=p;
        drop.other_supplies=o;

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
    auto end=start+chrono::duration<double>(time*60.0);

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
        if((++iters & 0xFFFF)==0){
            if(clk::now()>=end) break;
        }
    }

    cout<<"Solver finished."<<endl;
    return currSoln;

}

// int main() {
//     // 1. SETUP THE PROBLEM with the new data
//     ProblemData prob;
//
//     prob.time_limit_minutes = 1;
//     prob.d_max = 100;
//
//     // Packages: {weight, value}
//     prob.packages = {{0.01, 1}, {0.1, 2}, {0.005, 0.1}};
//
//     // Cities: {x, y}
//     prob.cities = {{0, 0}, {10, 10}};
//
//     // Villages: {id, {coords}, population}
//     Village v1 = {1, {0, 5}, 1000};
//     Village v2 = {2, {0, 10}, 1000};
//     prob.villages = {v1, v2};
//
//     // Helicopters: {id, home_city_id, wcap, dcap, F, alpha}
//     Helicopter h1 = {1, 1, 100, 25, 10, 1};
//     Helicopter h2 = {2, 2, 100, 50, 10, 1};
//     prob.helicopters = {h1, h2};
//
//
//     // 2. CREATE A SIMPLE, PREDICTABLE INITIAL SOLUTION
//     Solution initialSolution;
//
//     // Plan for Helicopter 1
//     HelicopterPlan plan1;
//     plan1.helicopter_id = 1;
//     Trip trip1 = {50, 10, 0, {{1, 50, 10, 0}}}; // Pickup 50d, 10p, 0o. Drop at Village 1.
//     plan1.trips.push_back(trip1);
//     initialSolution.push_back(plan1);
//
//     // Plan for Helicopter 2
//     HelicopterPlan plan2;
//     plan2.helicopter_id = 2;
//     Trip trip2 = {60, 5, 0, {{2, 60, 5, 0}}}; // Pickup 60d, 5p, 0o. Drop at Village 2.
//     plan2.trips.push_back(trip2);
//     initialSolution.push_back(plan2);
//
//
//     // 3. PRINT THE ORIGINAL SOLUTION
//     std::cout << "--- ORIGINAL SOLUTION ---" << std::endl;
//     printSolution(initialSolution, prob);
//
//
//     // 4. GENERATE ALL NEIGHBOURS
//     std::cout << "\n--- GENERATING ALL NEIGHBOURS ---" << std::endl;
//
//     // NOTE: This logic assumes you have a global vector `neighbours` and your
//     // `neighbour` function adds candidates to it.
//     // It is very important to clear the global vector before calling the function.
//     neighbours.clear(); 
//     Solution bestInternalNeighbour = neighbour(initialSolution, prob);
//     std::vector<Solution> candidates = neighbours;
//
//
//     // 5. PRINT ALL GENERATED CANDIDATES
//     if (candidates.empty()) {
//         std::cout << "No valid neighbors were generated in this step." << std::endl;
//     } else {
//         std::cout << "\n--- " << candidates.size() << " CANDIDATE NEIGHBOURS FOUND ---" << std::endl;
//         int i = 1;
//         for (auto& candidate : candidates) {
//             std::cout << "\n--- Candidate #" << i++ << " ---";
//             printSolution(candidate, prob);
//         }
//     }
//
//
//     // 6. FIND AND DISPLAY THE CHOSEN NEIGHBOUR (the one with the highest score)
//     Solution bestNeighbour = initialSolution;
//     double bestScore = evaluate(initialSolution, prob);
//
//     for (auto& candidate : candidates) {
//         double candidateScore = evaluate(candidate, prob);
//         if (candidateScore > bestScore) {
//             bestScore = candidateScore;
//             bestNeighbour = candidate;
//         }
//     }
//
//     std::cout << "\n========================================" << std::endl;
//     std::cout << "--- THE CHOSEN NEIGHBOUR ---" << std::endl;
//     std::cout << "========================================" << std::endl;
//     printSolution(bestNeighbour, prob);
//
//     return 0;
// }
