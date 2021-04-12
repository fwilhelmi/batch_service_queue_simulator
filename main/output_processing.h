/**
 * output_processing.h:
 *
 * - This file contains the methods for generating the output of a given simulation (process simulation results and provide statistics)
 */

#include <math.h>
#include <stddef.h>
#include <string>
#include <sstream>

#ifndef _OUT_METHODS_
#define _OUT_METHODS_

/**
* Prints and writes simulation results
*/
void PrintAndWriteSimulationStatistics() {

}

/**
* Generates the script's output (.txt) with results
* @param "logger_script" [type Logger]: pointer to the logger that writes logs into a file
* @param "simulation_time" [type double]: total simulation time
*/
void GenerateScriptOutput(Logger &logger_script, double simulation_time, Performance performance,
	double lambda, double mu, double waiting_timeout, int queue_size, int batch_size) {

	fprintf(logger_script.file, " Output (sim_time=%f-lambda=%f-mu=%f-waiting_timeout=%f-queue_size=%d-batch_size=%d)\n", 
		simulation_time, lambda, mu, waiting_timeout, queue_size, batch_size);
	fprintf(logger_script.file, "%d;%d;%f;%f;%f;%f;%f\n", performance.total_transactions, performance.transactions_dropped, 
		performance.drop_percentage, (double)performance.num_blocks_mined_by_timeout/(double)performance.num_blocks_mined,
		performance.mean_occupancy, performance.mean_delay, performance.fork_probability);

}

#endif
