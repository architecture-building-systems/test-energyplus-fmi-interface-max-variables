test-energyplus-fmi-interface-max-variables
===========================================

A small program to see if there is a limit to the number of variables that can
be shared with the Export FMI interface.

The reason for writing this is that we experience crashes in a co-simulation with
models bigger than simple testcases.

The method used here is to take a simple EnergyPlus model (e.g. Exercise1A.idf
from the EnergyPlus example files) and adds output variables for simple EMS
variables. The script takes a number N of interface variables to add and appends
the necessary code, compiles an FMU using EnergyPlusToFMU.py and then runs it with
pyFMI.

The python package `eppy` is used to parse and write the idf files.

Files
-----

- testepfmi.py
- testepfmi-globalvars.py

Dependencies
------------

- pyFMI
- eppy
- EnergyPlus (8.2 used here)
- EnergyPlusToFMI.py


License
-------

The project is licensed under the BSD license.
