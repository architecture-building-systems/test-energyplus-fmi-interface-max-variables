test-energyplus-fmi-interface-max-variables
===========================================

A small program to see if there is a limit to the number of variables that can
be shared with the Export FMI interface in EnergyPlus.

The reason for writing this is that we experience crashes in a co-simulation with
models bigger than simple testcases.

The method used here is to take a simple EnergyPlus model (e.g. Exercise1A.idf
from the EnergyPlus example files) and adds output variables for simple EMS
variables. The script takes a number N of interface variables to add and appends
the necessary code, compiles an FMU using EnergyPlusToFMU.py and then runs it with
a sample co-simulation master using the fmilib from JModelica.

Installation
------------

- download the source

    git clone https://github.com/architecture-building-systems/test-energyplus-fmi-interface-max-variables.git

- edit the testepfmi.py file and set the paths in the top four lines depending on your system 

- compile the TestEpFmiMaster.exe program

Execution
----------

I ran this in IPython, since the testepfmi.py script sets two variables: `cwd` and `fmu_path` that can be used for
running the `TestEpFmiMaster.exe` program. But otherwise you can use Windows Explorer to find these paths. Check
the temp folder for a path like `2014.12.22.13.41.13_test-energyplus-fmi-interface-max-variables_oxhcn9`:

    python testepfmi.py 42
    TestEpFmiMaster/Debug/TestEpFmiMaster.exe 42 "$fmu_path" "$cwd"

Results
-------

I have tried various settings for N and found the highest N that runs without crashing to be 41.

Files
-----

- testepfmi.py
  Creates the FMU based on Exercise1A.idf for a give N output variables

- testepfmi-globalvars.py
  Based on testepfmi.py, but lumps all EMS global variables in one 
  ENERGYMANAGEMENTSYSTEM:GLOBALVARIABLE object, just in case that was the issue.

- TestEpFmiMaster/TestEpFmiMaster/TestEpFmiMaster.cpp
  co-simulates the FMU (tightly coupled by output variable names)
  usage: TestEpFmiMaster.exe <N> <FMUPATH> <WORKINGDIRECTORY

Dependencies
------------

- Windows 7 x64 EN, up-to-date
- fmilib from JModelica (FMILibrary-2.0b2-win32 used, because the .lib files
  from the released binaries don't work.)
- eppy
- EnergyPlus (8.2 used here)
- EnergyPlusToFMI.py


License
-------

The project is licensed under the BSD license.
