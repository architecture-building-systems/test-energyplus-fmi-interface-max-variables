PATH_TO_ENERGYPLUSTOFMU = r"C:\EnergyPlusToFMU-1.0.3\Scripts\EnergyPlusToFMU.py"  # noqa
PATH_TO_IDF = r"C:\EnergyPlusV8-2-0\ExampleFiles\BasicsFiles\Exercise1A.idf"
PATH_TO_IDD = r"C:\EnergyPlusV8-2-0\Energy+.idd"
PATH_TO_EPW = r"C:\EnergyPlusV8-2-0\WeatherData\USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw"  # noqa

import eppy.modeleditor as modeleditor
import subprocess
import sys
import os
import tempfile
import datetime

# load the original idf file
try:
    modeleditor.IDF.setiddname(PATH_TO_IDD)
except modeleditor.IDDAlreadySetError:
    pass
idf = modeleditor.IDF(PATH_TO_IDF)


# add a number of variables to the output interface
num_variables = int(sys.argv[1])
idf.newidfobject('RUNPERIOD',
                 Begin_Month=1,
                 Begin_Day_of_Month=1,
                 End_Month=12,
                 End_Day_of_Month=31)
idf.idfobjects['TIMESTEP'][0].Number_of_Timesteps_per_Hour = 1
idf.idfobjects['SIMULATIONCONTROL'][0].Run_Simulation_for_Weather_File_Run_Periods = 'YES'  # noqa
idf.newidfobject('EXTERNALINTERFACE',
                 Name_of_External_Interface='FunctionalMockupUnitExport')
gvars = idf.newidfobject('ENERGYMANAGEMENTSYSTEM:GLOBALVARIABLE')

for i in range(num_variables):
    setattr(gvars, 'Erl_Variable_%i_Name' % (i + 1), 'dummy%i' % i)
    idf.newidfobject('ENERGYMANAGEMENTSYSTEM:OUTPUTVARIABLE',
                     Name='DummyVariable%i' % i,
                     EMS_Variable_Name='dummy%i' % i,
                     Type_of_Data_in_Variable='Averaged',
                     Update_Frequency='ZoneTimestep')
    idf.newidfobject('EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITEXPORT:FROM:VARIABLE',  # noqa
                     OutputVariable_Index_Key_Name='EMS',
                     OutputVariable_Name='DummyVariable%i' % i,
                     FMU_Variable_Name='DummyVariable%i' % i)

# save modified file to disc
cwd = tempfile.mkdtemp(
    prefix=datetime.datetime.now().strftime('%Y.%m.%d.%H.%M.%S')
    + "_test-energyplus-fmi-interface-max-variables_")
idf_path = os.path.join(cwd, 'test%i.idf' % num_variables)
idf.saveas(idf_path)

# create the fmu
subprocess.check_call(['python',
                       PATH_TO_ENERGYPLUSTOFMU,
                       '-i', PATH_TO_IDD,
                       '-d', '-L',
                       '-w', PATH_TO_EPW,
                       idf_path],
                      cwd=cwd)

# run the co-simulation
fmu_path = os.path.splitext(idf_path)[0] + '.fmu'
