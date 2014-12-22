// TestEpFmiMaster.cpp : load an EnergyPlus FMU and simulate it.
// expects N amount of output variables.

#include <iostream>
#include <fmilib.h>
#include <string>

using namespace std;

// helper function for printing debug info
void importlogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    printf("module = %s, log level = %s: %s\n", module, jm_log_level_to_string(log_level), message);
}

void do_exit(int code)
{
	printf("Press 'Enter' to exit\n");
	getchar();
	exit(code);
}

// drive the simulation
void simulate(int n, char* fmu_path, char* working_directory)
{	
    // copying a lot of stuff from the fmi_import_cs_test.c program included with fmilib
    fmi1_callback_functions_t callBackFunctions;
    jm_callbacks callbacks;
    fmi_import_context_t* context;
    fmi_version_enu_t version;
    jm_status_enu_t jmstatus;

    fmi1_import_t* fmu;    

    callbacks.malloc = malloc;
    callbacks.calloc = calloc;
    callbacks.realloc = realloc;
    callbacks.free = free;
    callbacks.logger = importlogger;
    callbacks.log_level = jm_log_level_debug;
    callbacks.context = 0;

    callBackFunctions.logger = fmi1_log_forwarding;
    callBackFunctions.allocateMemory = calloc;
    callBackFunctions.freeMemory = free;

    context = fmi_import_allocate_context(&callbacks);

    // this actually unzips the fmu to the working_directory and then reads in the xml file
    version = fmi_import_get_fmi_version(context, fmu_path, working_directory);

    if (version != fmi_version_1_enu)
	{
        printf("Only version 1.0 is supported so far\n");
        do_exit(1);
	}

    fmu = fmi1_import_parse_xml(context, working_directory);

    if (!fmu) 
	{
        printf("Error parsing XML, exiting\n");
        do_exit(1);
	}

    jmstatus = fmi1_import_create_dllfmu(fmu, callBackFunctions, 1); // FIXME: try setting registerGlobally to 0 for thread safety
    if (jmstatus == jm_status_error)
	{
        printf("Could not create the DLL loading mechanism(C-API) (error: %s).\n",
            fmi1_import_get_last_error(fmu));
        do_exit(1);
	}

    // simulate the fmu a bit...
    fmi1_status_t fmistatus;
    fmi1_string_t instanceName = "TestEpFmiMaster";
    fmi1_string_t fmuLocation = 0;
    fmi1_string_t mimeType = "";
	fmi1_real_t timeout = 0.0;
    fmi1_boolean_t visible = fmi1_false;
    fmi1_boolean_t interactive = fmi1_false;
    fmi1_boolean_t loggingOn = fmi1_true;

	// Simulation setup: Simulation starts at midnight (this cannot be changed)
    fmi1_real_t tstart = 0.0;
    fmi1_real_t tcur = tstart;
    fmi1_real_t hstep = 60 * 60; // one hour, must match timestep in IDF, so for TIMESTEP = 4, use 15*60
    fmi1_real_t tend = 365 * 24.0 * 60 * 60; // 365 days
    fmi1_boolean_t StopTimeDefined = fmi1_false;

    jmstatus = fmi1_import_instantiate_slave(fmu, instanceName, fmuLocation, mimeType, timeout, visible, interactive);
    if (jmstatus == jm_status_error)
	{
        printf("fmi1_import_instantiate_slave failed\n");
        do_exit(1);
	}

	// this line starts up EnergyPlus and does the warm up
    fmistatus = fmi1_import_initialize_slave(fmu, tstart, StopTimeDefined, tend);
    if (fmistatus != fmi1_status_ok)
	{
        printf("fmi1_import_initialize_slave failed\n");
        do_exit(1);
	}
	

    tcur = tstart;
    while (tcur < tend)
	{
		fmistatus = fmi1_import_do_step(fmu, tcur, hstep, fmi1_true);

		// read the building
		for (int i = 0; i < n; ++i)
		{
			// defines the values to take out of the simulation
			fmi1_real_t realValue;
			fmi1_import_variable_t* pVariable;
			fmi1_value_reference_t valueReference;
			
			char buffer[100];
			sprintf(buffer, "DummyVariable%i", i);			
			pVariable = fmi1_import_get_variable_by_name(fmu, buffer);
			valueReference = fmi1_import_get_variable_vr(pVariable);
			if (pVariable == NULL) { throw(string("Error: Variable not found in EnergyPlus simulation.")); }
			fmistatus = fmi1_import_get_real(fmu, &valueReference, 1, &realValue);
			printf("get vr=%d: %10g\n", valueReference, realValue);		
		}      		
        tcur += hstep;
	}

    printf("Simulation finished.\n");

    fmistatus = fmi1_import_terminate_slave(fmu);
    fmi1_import_free_slave_instance(fmu);

    // clean up
    fmi1_import_destroy_dllfmu(fmu);
    fmi1_import_free(fmu);
    fmi_import_free_context(context);

    printf("Everything seems to be OK, or else the program would have crashed by now!\n");	
}

int main(int argc, char* argv[])
{
	if (argc > 3)
	{
		cout << "N=" << argv[1] << '\n';
		cout << "FMUPATH=" << argv[2] << '\n';
		cout << "WORKINGDIRECTORY=" << argv[3] << '\n';
		int n = atoi(argv[1]);
		char* fmu_path = argv[2];
		char* working_directory = argv[3];
		simulate(n, fmu_path, working_directory);
		return 0;
	}
	else
	{
		std::cerr << "usage: TestEpFmiMaster.exe <N> <FMUPATH> <WORKINGDIRECTORY\n";
		return 1;
	}
}

