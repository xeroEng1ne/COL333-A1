#include "solver.h"
#include <iostream>
#include "evaluator.h"
#include "utils.h"

using namespace std;

// ------------------------- NEIGHBOURHOOD FUNCTION --------------------------------------------

Solution neighbour(Solution& currSoln, ProblemData& problem){

    vector<Solution> adj;
    double best=evaluate(currSoln, problem);
    int H=problem.helicopters.size();

    Solution bestSoln = currSoln;

    // Generating one neighbour for each Helicopter
    for(int i=0;i<H;i++){

        Solution candidate = currSoln;

        if(candidate[i].trips.empty()) continue;

        // generate a random trip index to modify
        int rndId=random_number(0,H-1);
        Trip& modify_trip=candidate[i].trips[rndId];
        
        //pick a random type of move
        int move=random_number(1, 3);

        if(move==1){
            // reroute the trip by moving villages
        }
        else if(move==2){
            // optimize cargo
        }
        else{
            // Add/delete villages

        }

        // todo: check validity
    }

    for(auto& neighbour : adj){
        double neighbour_score=evaluate(neighbour,problem);
        if(neighbour_score > best){
            best=neighbour_score;
            bestSoln=neighbour;
        }
    }
    return bestSoln;

}

//---------------------------------------------------------------//
Solution solve(const ProblemData& problem) {
    cout << "Starting solver..." << endl;

    Solution solution;
    for (const auto& helicopter : problem.helicopters) {
        HelicopterPlan plan;
        plan.helicopter_id = helicopter.id;

        if (!problem.villages.empty()) {
            Trip trip;
            // Pickup 1 of each package type
            trip.dry_food_pickup = 1;
            trip.perishable_food_pickup = 1;
            trip.other_supplies_pickup = 1;

            // Drop them at the first village
            Drop drop;
            drop.village_id = problem.villages[0].id;
            drop.dry_food = 1;
            drop.perishable_food = 1;
            drop.other_supplies = 1;

            trip.drops.push_back(drop);
            plan.trips.push_back(trip);
        }
        solution.push_back(plan);
    }
    
    // --- END OF PLACEHOLDER LOGIC ---

    cout << "Solver finished." << endl;
    return solution;
}
