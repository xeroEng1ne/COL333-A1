# COL333-A1


**INPUT**
- Time in minutes
- DMax
- 6 values -> {w(d),v(d)}, {w(p),v(p)}, {w(o),v(o)}
- c, vpi of size c {{x,y}}
- v, vvi of size v {{x,y,n}}
- H , vvi of size H {{home,wcap,dcap,F,alpha}}

**OUTPUT**
- For each helicopter:
    - hi , number of trips
    - For each trip
        - number of packages of d,p and o, number of villages, village id, number of packages to each type per village
    - -1 


## BIG LEVEL IDEA

*Objective function* : `Total Value gained - Total Trip cost`

*Top Level*
There will be an array representing the plan for all helicopters
`Solution State` = (i=1 -> H){HelicopterPlan_i}

*Mid Level*
For each helicopter, what is the plan for all trips
`HelicopterPlan` = { trips: (trip1, trip2, ...)}

*Low Level*
For a single trip, what load it takes and the ordered sequence of villages it visits
`trip`
*Low Level*
For a single trip, what load it takes and the ordered sequence of villages it visits
`trip` = {(d,p,o), route:(villageA, villageB, ....)}

**Neighbourhood function**

`TYPE 1: Changes within a single trip`
ex: home -> v1 -> v2 -> home ==> home -> v2 -> v1 -> home

`TYPE 2: Changes between 2 trips`
*Move a village* Take one village from trip1 and move it to trip2. (if possible)
This makes trip1 shorter and trip2 longer

*Swap villages* Take one village from trip1 and swap it with a village in trip2

`TYPE 3: Change the number of trips`
Create a trip of Delete a trip


## Structure

Model
- Makefile
- data/
    - input.txt
- include/
    - types.h                 // Basic, common data types (Point, PacketType)
    - entities.h              // Structs for static problem entities (Village, Helicopter)
    - solution.h              // Classes for the dynamic solution state (Trip, Plan)
    - parser.h                // Handles reading and parsing the input file
    - evaluator.h             // Implements the objective/score function
    - initial_solution.h      // Logic for generating the first greedy solution
    - local_search.h          // The main local search algorithm engine
    - utils.h                 // Helper functions like distance calculation
- src/
    - main.cpp
    - parser.cpp
    - evaluator.cpp
    - initial_solution.cpp
    - local_search.cpp
    - utils.cpp

## TODOS

- [x] evaluator
- [x] deciding heuristic
- [ ] Neighbourhood function
- [ ] Validation file
- [ ] Initial greedy state
- [ ] Greedy hill climbing
- [ ] Random Restarts



