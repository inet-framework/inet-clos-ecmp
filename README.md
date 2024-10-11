In this file, we will explain how to obtain the same results that were used in the Bachelor's thesis because, due to the size of the fileS, containing the raw data from the simulations being too big (around 50 GB), they were deleted.

The first step is to install OMNeT++ 6.0.1. To do this, we can follow the official installation guide that can be found here: https://doc.omnetpp.org/omnetpp/InstallGuide.pdf

Next step is to import the inet4.4 project to OMNeT.

Once this is done, the last thing is to compile INET and run the simulations that can be found in the folder inet4.4/examples/ecmp. The Clos network is inside the ClosExample folder, the Fat-Tree is inside the FatTreExample folder, and the Dragonfly is inside the MegaFlyExmaple folder.
*It is recommended to run the simulations using Cmdenv and multiple threads if possible.*

Inside these folders, the anf files are configured to automatically import the results of the simulations and to generate all the graphics that have been used in the thesis document.
