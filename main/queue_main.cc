 /**
 * queue_main.cc: this is the main file.
 */
#include <stdio.h>
#include <string>     // std::string, std::to_string
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include ".././COST/cost.h"


#include "queue.h"
#include "traffic_generator.h"
#include "logger.h"
#include "output_processing.h"
#include "performance.h"

template <typename T>
std::string ToString(T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

/* Sequential simulation engine from where the system to be simulated is derived. */
component QueueSimulator : public CostSimEng {

	// Methods
	public:

		void Setup(double simulation_time_console, int logs_enabled_console, int queue_size_console, 
			int batch_size_console, double lambda_console, double mu_console, double timeout_mining_console, 
			int seed_console, int forks_enabled, int n_miners, double capacity_p2p, const char *script_output_filename);
		void Stop();
		void Start();

		void SetupEnvironment();
		void PrintSystemInfo();

	// Public items (to shared with the queue and the traffic generator)
	public:
		
		// Parameters entered per console
		double simulation_time;	///> Simulation time [s]
		int logs_enabled;	///> Flag to enable/disable logs
		int queue_size;		///> Queue length [# of transactions]
		int batch_size;		///> Batch size / block size [# of transactions]
		double lambda;		///> User arrivals [# of transactions per second]
		double mu;		///> Departures rate (mining) [# of blocks per second]
		double timeout_mining;	///> Timeout for mining a block [s]
		// Parameters related to forks
		int n_miners;		///> Number of miners
		double capacity_p2p;	///> Capacity of P2P links [bps]

		Queue queue;			    ///> Batch service queue representing the pool of transactions
		TrafficGenerator traffic_generator; ///> Object creating user requests		

	// Private items
	private:

		// Parameters entered per console
		int seed; ///> Simulation seed number

		// Forks-related parameters
		int forks_enabled;	

		// Script-related variables
		FILE *script_output_file;			///> File for writing the results of an execution by script
		Logger logger_script;				///> Logger for the script file 
		
};

/**
 * Setup()
 * @param "simulation_time_console" [type double]: simulation observation time [s]
 * @param "logs_enabled_console" [type int]: flag to enable/disable logs
 * @param "queue_size_console" [type int]: queue size (max. number of transactions to be stored)
 * @param "batch_size_console" [type int]: batch size (number of transactions to be delivered in bulk)
 * @param "lambda_console" [type double]: arrivals rate [transactions per second]
 * @param "mu_console" [type double]: departures rate [blocks per second]
 * @param "timeout_mining_console" [type double]: timeout for mining a block [s]
 * @param "seed_console" [type int]: random seed
 */
void QueueSimulator :: Setup(double simulation_time_console, int logs_enabled_console, 
	int queue_size_console, int batch_size_console, double lambda_console, 
	double mu_console, double timeout_mining_console, int seed_console,
	int forks_enabled_console, int n_miners_console, double capacity_p2p_console, 
	const char *script_output_filename){

	// Setup variables corresponding to the console's input
	simulation_time = simulation_time_console;
	logs_enabled = logs_enabled_console;
	queue_size = queue_size_console;
	batch_size = batch_size_console;
	seed = seed_console;
	lambda = lambda_console;
	mu = mu_console;
	timeout_mining = timeout_mining_console;
	forks_enabled = forks_enabled_console;
	n_miners = n_miners_console;
	capacity_p2p = capacity_p2p_console;

	// Configure environment based on input params.
	SetupEnvironment();

	// Display simulation information
	PrintSystemInfo();

	// Script output (Readable)
	script_output_file = fopen(script_output_filename, "at");	// Script output is removed when script is executed
	logger_script.save_logs = 1;
	logger_script.file = script_output_file;
	fprintf(logger_script.file, "QUEUE SIMULATION (seed %d)", seed);

};

/**
 * Start()
 */
void QueueSimulator :: Start(){
	// Do nothing
};

/**
 * Stop(): called when the simulation finishes
 */
void QueueSimulator :: Stop(){

	if(logs_enabled) printf(" SIMULATION FINISHED\n");
	if(logs_enabled) printf("------------------------------------------\n");

	GenerateScriptOutput(logger_script, simulation_time, queue.performance,
		lambda, mu, timeout_mining, queue_size, batch_size);
	
	fclose(script_output_file);

};


/**
 * Set up the Queue environment
 */
void QueueSimulator :: SetupEnvironment() {

	// Connect the traffic generator with the queue
	connect traffic_generator.outportNewPacketGenerated,queue.InportNewPacketGenerated;

	// Initialize traffic generator
	traffic_generator.traffic_load = lambda; 
	traffic_generator.logs_enabled = logs_enabled;

	// Initialize the queue
	queue.simulation_time = simulation_time;
	queue.logs_enabled = logs_enabled;
	queue.queue_size = queue_size;
	queue.batch_size = batch_size;
	queue.departures_rate = mu;	
	queue.timeout_mining = timeout_mining;
	queue.n_miners = n_miners;
	queue.capacity_p2p = capacity_p2p;
	queue.InitializeVariables();

}

/**
 * Print information of the simulation
 */
void QueueSimulator :: PrintSystemInfo(){

	if(logs_enabled) printf("\n");
	if(logs_enabled) printf("*************************************************************************************\n");
	if(logs_enabled) printf(" BATCH-SERVICE QUEUE SIMULATOR FOR BLOCKCHAIN\n");
	if(logs_enabled) printf(" - Authors: Lorenza Giupponi & Francesc Wilhelmi\n");
	if(logs_enabled) printf(" - GitHub repository: https://bitbucket.org/francesc_wilhelmi/batch_service_queue_simulator\n");
	if(logs_enabled) printf("*************************************************************************************\n");
	if(logs_enabled) printf("\n");
	
	if(logs_enabled) printf(" - simulation_time = %f\n", simulation_time);
	if(logs_enabled) printf(" - queue_size = %d\n", queue_size);
	if(logs_enabled) printf(" - batch_size = %d\n", batch_size);
	if(logs_enabled) printf(" - lambda = %f\n", lambda);
	if(logs_enabled) printf(" - mu = %f\n", mu);
	if(logs_enabled) printf(" - timeout_mining = %f\n", timeout_mining);
	if(forks_enabled) {
		if(logs_enabled) printf(" - n_miners = %d\n", n_miners);
		if(logs_enabled) printf(" - capacity_p2p = %f\n", capacity_p2p);	
	}
	if(logs_enabled) printf(" - seed = %d\n", seed);
	if(logs_enabled) printf("\n");
}

/**********/
/* main() */
/**********/
int main(int argc, char *argv[]){

	// Input variables
	double simulation_time;
	int logs_enabled;
	int queue_size;
	int batch_size;
	double lambda;
	double mu;
	double timeout_mining;
	int seed;
	int forks_enabled;
	int n_miners;
	double capacity_p2p;
	std::string script_output_filename;

	// Get input variables per console
	if(argc == 9){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		seed = atoi(argv[8]);
		// Default params.
		n_miners = 1;
		capacity_p2p = 0;
		forks_enabled = 0;
		script_output_filename.append("../output/script_output_default");
	} else if(argc == 10){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		seed = atoi(argv[8]);
		script_output_filename = ToString(argv[9]);
		// Default params.
		n_miners = 1;
		capacity_p2p = 0;
		forks_enabled = 0;
	}else if(argc == 11){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		n_miners = atoi(argv[8]);
		capacity_p2p = atof(argv[9]);
		seed = atoi(argv[1]);
		forks_enabled = 1;
		// Default params.
		script_output_filename.append("../output/script_output_default");
	} else if(argc == 12){
		simulation_time = atof(argv[1]);
		logs_enabled = atoi(argv[2]);
		queue_size = atoi(argv[3]);
		batch_size = atoi(argv[4]);
		lambda = atof(argv[5]);
		mu = atof(argv[6]);
		timeout_mining = atof(argv[7]);
		n_miners = atoi(argv[8]);
		capacity_p2p = atof(argv[9]);
		seed = atoi(argv[10]);
		script_output_filename = ToString(argv[11]);
		forks_enabled = 1;
	} else {
		printf("- ERROR: The arguments provided were not properly set!\n "
			" + To execute the program, please introduce:\n"
			"    ./queue_main -simulation_time -logs_enabled -queue_size -batch_size -lambda -mu -timeout_mining -seed\n");
		return(-1);
	}

	// Generate a simulation object to start the running the queue
	QueueSimulator queue_simulation;
	queue_simulation.Seed = seed;
	srand(seed);
	queue_simulation.StopTime(simulation_time);
	queue_simulation.Setup(simulation_time, logs_enabled, queue_size, 
		batch_size, lambda, mu, timeout_mining, seed, forks_enabled,
		n_miners, capacity_p2p, script_output_filename.c_str());

	if(logs_enabled) printf("------------------------------------------\n");
	if(logs_enabled) printf("SIMULATION STARTED\n");

	queue_simulation.Run();

	return(0);
};
