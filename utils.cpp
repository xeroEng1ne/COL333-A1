#include "utils.h"

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

int random_number(int l, int r){
    uniform_int_distribution<int> dist(l,r);
    int x=dist(rng);
    return x;
}
