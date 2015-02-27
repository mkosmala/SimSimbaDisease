# SimSimbaDisease
Modification of the individual-based stochastic lion simulation model SimSimba to include a SEI disease component.

The code consists of:

(1) the SimSimba-disease model, written in C++ and compilable on any Unix/linux-based machine; a Makefile is included

(2) an example parameters file (text format) that is read by the SimSimba-disease model and contains all lion demographic and disease parameters

(3) an example .simba file (text format) used in Kosmala et al "Estimating wildlife disease dynamics in complex systems using an Approximate Bayesian Computation framework", which contains information about the landscape structure and lion population structure. In this particular file, the population is at carrying capacity.

(4) an R file called process-data.r, which performs the ABC algorithm steps described in Kosmala et al "Estimating wildlife disease dynamics in complex systems using an Approximate Bayesian Computation framework" 


