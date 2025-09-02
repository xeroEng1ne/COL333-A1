#include "evaluator.h"

// util functions for the trip struct
double getTotalValue(Trip& trip, ProblemData& probData){
    double d=trip.dry_food_pickup*probData.packages[0].value;
    double p=trip.perishable_food_pickup*probData.packages[1].value;
    double o=trip.other_supplies_pickup*probData.packages[2].value;

    return d*p*o;
}

double getTotalDistance(Trip& trip, ProblemData& probData, Point& homeCity){

    if(trip.drops.empty()) return 0.0;

    double totDist=0.0;
    Point currPos = homeCity;
    for(auto& drop: trip.drops){
        //todo: make this faster by using map
        auto& village=probData.villages[drop.village_id];
        totDist+=distance(currPos,village.coords);
        currPos=village.coords;
    }

    totDist+=distance(currPos,homeCity);
    return totDist;
}

double evaluate(Solution& solution, ProblemData& probData){
    double tot_value=0.0;
    double tot_trip_cost=0.0;

    for(auto& plan : solution){
        Helicopter* curr_heli=nullptr;
        //todo: make searching faster
        for(auto& h : probData.helicopters){
            if(h.id==plan.helicopter_id){
                curr_heli=&h;
                break;
            }
        }

        if(curr_heli==nullptr){
            throw runtime_error("Helicopter with ID "+to_string(plan.helicopter_id)+" not found in problem data");
        }

        Point& homeCity = probData.cities[curr_heli->home_city_id];

        for(auto& trip : plan.trips){
            tot_value += getTotalValue(trip,probData);
            double tripDistance=getTotalDistance(trip,probData,homeCity);
            double tripCost=curr_heli->fixed_cost+(curr_heli->alpha*tripDistance);

            tot_trip_cost+=tripCost;
        }
    }

    return tot_value-tot_trip_cost;
}
