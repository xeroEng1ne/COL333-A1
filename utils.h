#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <random>
#include <iostream>
#include "structures.h"

using namespace std;

int random_number(int l, int r);
double getTripValue(Trip& trip,const ProblemData& problem,vector<double>& food_delivered_so_far,vector<double>& other_delivered_so_far);
double getTripDistance(Trip& trip, const ProblemData& problem,const Point& homeCity);
double getTripWeight(Trip& trip, const ProblemData& problem);
vector<double> getPackagesNumber(Trip& trip, const ProblemData& problem);
vector<Village> giveVillages(Trip& trip, const ProblemData& problem);
#endif
