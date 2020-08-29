# Meta-Scheduling
## MiniEvents
Implementation of the Mininet Evaluations.
The Evaluation Script is located in MiniEvents/mininet/ and is called mininet_progmp_evaluate.py .


## ns3-mptcp
Implementation of the approach in ns-3.



Before ns-3 is usable it needs to be configured with this command:

`$ CXXFLAGS=“-std-c++0x” ./waf configure --build-profile=optimized`

To generate schedulers, the experiment needs to be defined in scratch/Neat/experiments.cc under cc_test and cc_evaluate.

The corresponding command is like any other ns-3 simulation with the parameter file:

`$ ./waf --run “Neat p2nv.ne”`

Then the experiment 5 needs to be chosen and the evolution starts.

****

The evaluation is located in scratch/evaluation/ and called like this:

`$ ./waf --run “evaluation”`