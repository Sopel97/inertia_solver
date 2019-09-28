A solver for game "Inertia".
https://www.chiark.greenend.org.uk/~sgtatham/puzzles/js/inertia.html

It can be specified in the source code whether starting position is a to be considered hole or not.

This is a project for algorithms class at AGH University of Science and Technology.
For this reason most of the customisation options are made constants in the Solver class.

Output consists of a string of digits 0-7. They encode subsequent moves, 0 means north, 1 north-east, and so on in clock-wise direction.

Currently most of the configuration (including maximum time taken by certain algorithm parts) can only be specified in the source code by changing the values of constexpr variable in class Solver.

Whole program is just one cpp file for ease of compilation. Requires at least C++17 compiler.

input folder contains randomly generated input boards for testing as well as a conversion script.

bench folder contains example output from running rate_min_all.bat

The description of the algorithms used can be found in docs folder (Polish)
