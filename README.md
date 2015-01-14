Disclaimer: I am not responsible for Honor code violations resulting from this public repository.
# Superscalar-Simulator--Tomasulos-Algorithm
A simulator for a superscalar processor that implements Tomasuloa’s Algorithm for out-of-order execution.
type : make 
then type:
./procsim < trace_file
Example : ./procsim < gcc.100k.trace
The above command runs the program with default parameters defined inside procism.cpp.
To override with command line arguments type the following:
./procsim –r R –f F –j J –k K –l L < trace_file
where R F J K L correspong to number of result buses, no. of instructions fetched per cycle and no. of function units available of three different types respectively. 
