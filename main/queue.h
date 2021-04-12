 /**
 * queue.h: this file contains the queue component
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>

#include "FIFO.h"
#include "transaction.h"
#include "performance.h"
#include "generic_methods.h"

// Node component: "TypeII" represents components that are aware of the existence of the simulated time.
component Queue : public TypeII{

	// Methods
	public:

		// COST
		void Setup();
		void Start();
		void Stop();

		// Generic
		void InitializeVariables();
		
		// Mining operations
		void CheckMining();
		void SetMiningTimeout();


	// Public items (entered by nodes constructor in queue_main)
	public:

		double simulation_time;	 ///> Total simulation time
		int logs_enabled;	 ///> Flag indicating whether logs are enabled/disabled
		int queue_size; 	 ///> Size of the queue
		int batch_size; 	 ///> Size of the batch
		double departures_rate;  ///> Departures rate
		double timeout_mining;   ///> Mining timeout
		int n_miners; 		 ///> Number of miners (to reproduce forks)
		double capacity_p2p; 	 ///> Capacity of P2P links (to reproduce forks)
		
		Performance performance;

		FIFO buffer;		///> FIFO buffer (contains Transaction objects)
	
	// Private items
	private: 

		int mining_active; 			///> Keep track of mining activity

		double delay_measurements;		///> To compute the queue delay
		int num_mined_transactions;		///> Total number of mined transactions 

		int *occupancy_array;			///> Array to store the occupancy at different times
		int occupancy_measurements;		///> Measurements done to compute the queue occupancy	
		int num_occupancy_measurements;		///> Number of queue occupancy measurements (to compute queue occupancy)

		int *departure_array;			///> Array to store the occupancy at departures
		int num_departure_measurements;		///> Number of departure measurements (to compute departures' distribution)

		int current_state;
		int previous_state;

		int current_d_state;
		int previous_d_state;		
	
		double timestamp_last_state;
		double timestamp_last_d_state;
		double **queue_status_from_departure;
		double *time_in_d_state;
		double *time_in_k_from_d_state;

		double *time_per_state;

		int *counter_queue_departure; 

		double *sum_time_spent_per_d_state;
		int *measurements_time_per_d_state;
		
		int num_transactions_generated;		///> Total number of generated transactions
		int num_transactions_dropped;		///> Total number of dropped transactions
		int transaction_counter;		///> Counter to keep track of transactions
		int num_transaction_to_be_mined; 	///> Auxiliary variable to aid the mining procedure (num. of transactions to be mined)

		int timeout_active;			///> Flag indicating whether the timeout is active or not
		int times_timeout_expired;		///> Time at which the next timeout expires
		int *times_timeout_expired_from_d_state;
		int *times_timeout_expired_from_state;

		int n_blocks;		
		int n_forks;
		double *delay_miners;		///> Array to check if forks occur based on the delay of each miner for mining a block
		double propagation_time;	///> Propagation time (to reproduce forks)

		const int HEADER_LENGTH = 640;
		const int TRANSACTION_LENGTH = 3000;

		double timestamp_current_epoch;
		double sum_duration_epoch;
		int counter_sum_duration_epoch;

		int timer_expired;
		int arrivals_from_state;
		int *arrivals_from_state_timer;
		int *arrivals_from_state_no_timer;
		int *counter_arrivals_timer;
		int *counter_arrivals_no_timer;	

		double *expected_arrivals_timer;
		double *expected_arrivals_no_timer;		
		
		///// ADD TIME TO GO

	// Connections and timers
	public:

		// Inport for the Traffic generator
		inport void inline InportNewPacketGenerated();

		// Triggers
		Timer <trigger_t> trigger_toStartMining;
		Timer <trigger_t> trigger_toFinishMining;

		// Every time a timer expires execute this
		inport inline void StartMining(trigger_t& t1);
		inport inline void MiningFinished(trigger_t& t1);

		// Connect timers to methods
		Queue () {
			connect trigger_toStartMining.to_component,StartMining;
			connect trigger_toFinishMining.to_component,MiningFinished;
		}
};

/**
 * Setup()
 */
void Queue :: Setup(){
	// Do nothing
};

/**
 * Start()
 */
void Queue :: Start(){

	if(logs_enabled) printf("%.12f; Queue Start()\n", SimTime());

	// Set the first timeout
	SetMiningTimeout();	
	
};

/**
 * Stop()
 */
void Queue :: Stop(){

	if(logs_enabled) printf("%.12f; Queue Stop()\n", SimTime());

	if(logs_enabled) printf("\n---------------------------\n");
	if(logs_enabled) printf("------- STATISTICS --------\n");
	if(logs_enabled) printf("---------------------------\n");
	if(logs_enabled) printf("- Total transactions generated/dropped: %d/%d (drop ratio=%.2f%%)\n", 
		num_transactions_generated, num_transactions_dropped, 100*(double)num_transactions_dropped/(double)num_transactions_generated);
	//if(logs_enabled) printf("- Total transactions dropped: %d\n", num_transactions_dropped);
	if(logs_enabled) printf("- Blocks mined by timeout: %d/%d (%.2f%%)\n", times_timeout_expired, n_blocks, 100*(double)times_timeout_expired/(double)n_blocks);
	if(logs_enabled) printf("- Average queue delay: %f s\n", delay_measurements/num_mined_transactions);
	if(logs_enabled) printf("- Average queue occupancy: %f transactions\n", (double)occupancy_measurements/(double)num_occupancy_measurements);
	if(logs_enabled) printf("- Average inter-departure time: %f\n", sum_duration_epoch/counter_sum_duration_epoch);

	if(logs_enabled) printf("- Times timeout expired from d state: ");
	for (int i=0; i < queue_size+1; ++i) {
		if(logs_enabled) printf("%d (%.2f%%) ", times_timeout_expired_from_d_state[i], 100*(double)times_timeout_expired_from_d_state[i]/(double)times_timeout_expired);
	}
	if(logs_enabled) printf("\n");
	if(logs_enabled) printf("- Times timeout expired from state: ");
	for (int i=0; i < queue_size+1; ++i) {
		if(logs_enabled) printf("%d (%.2f%%) ", times_timeout_expired_from_state[i], 100*(double)times_timeout_expired_from_state[i]/(double)times_timeout_expired);
	}
	if(logs_enabled) printf("\n");

	if(logs_enabled) printf("- Forks vs Blocks: %d - %d (%.2f%%)\n", n_forks, n_blocks, 100*(double)n_forks/(double)n_blocks);

	performance.total_transactions = num_transactions_generated;
	performance.transactions_dropped = num_transactions_dropped;
	performance.drop_percentage = (double)num_transactions_dropped/(double)num_transactions_generated;
	performance.num_blocks_mined = n_blocks;
	performance.num_blocks_mined_by_timeout = times_timeout_expired;
	performance.mean_occupancy = (double)occupancy_measurements/(double)num_occupancy_measurements;
	performance.mean_delay = delay_measurements/num_mined_transactions;
	performance.fork_probability = (double)n_forks/(double)n_blocks;

	if (queue_size <= 10) { // Print info. only for small queue size values

		if(logs_enabled) printf("\n       + + + + + + + + +\n");		
		if(logs_enabled) printf("       +  Model match  +\n");
		if(logs_enabled) printf("       + + + + + + + + +\n");	
		
		// Probability of reaching each state after a system departure
		if(logs_enabled) printf("\n+ Departure Probabilities (pi_d):\n\t");
		double *pi_d = new double[queue_size+1];
		for(int i = 0; i < queue_size+1; ++i){
			pi_d[i] = (double)departure_array[i]/(double)num_departure_measurements;
			if(logs_enabled)printf("%f ", pi_d[i]);
		}
		if(logs_enabled) printf("\n");

		// General probability of being in each state
		if(logs_enabled) printf("\n+ Steady-state Probabilities (pi_s):\n\t");
		double *pi_s = new double[queue_size+1];
		for(int i = 0; i < queue_size+1; ++i){
			pi_s[i] = (double)occupancy_array[i]/(double)num_occupancy_measurements;
			if(logs_enabled) printf("%f ", pi_s[i]);
		}
		if(logs_enabled) printf("\n");

		if(logs_enabled) printf("\n[Cross-validation] Steady-state distr. (pi_s) from time measured in each state:\n\t");
		for(int i = 0; i < queue_size+1; ++i){
			if(logs_enabled) printf("%f ", time_per_state[i]/simulation_time);
		}
		if(logs_enabled) printf("\n");

		// Measured blocking probability (discarded arrivals)
		if(logs_enabled) printf("\n+ Blocking probability (pb): %f\n", (double)num_transactions_dropped/(double)num_transactions_generated);


		printf("arrivals_from_state_timer:\n");
		for (int i=0; i < queue_size+1; ++i) {
			if (counter_arrivals_timer[i] == 0) {
				expected_arrivals_timer[i] = 0;
			} else {
				expected_arrivals_timer[i] = (double)arrivals_from_state_timer[i]/counter_arrivals_timer[i];
			}
			printf("%f ", expected_arrivals_timer[i]);
		}
		printf("\n");

		printf("arrivals_from_state_no_timer:\n");
		for (int i=0; i < queue_size+1; ++i) {
			if (counter_arrivals_no_timer[i] == 0) {
				expected_arrivals_no_timer[i] = 0;
			} else {
				expected_arrivals_no_timer[i] = (double)arrivals_from_state_no_timer[i]/counter_arrivals_no_timer[i];
			}
			printf("%f ", expected_arrivals_no_timer[i]);
		}
		printf("\n");

		double mean_arrivals_timer = 0;
		double mean_arrivals_no_timer = 0;
		for (int i=0; i < queue_size+1; ++i) {
			mean_arrivals_timer += pi_d[i]*expected_arrivals_timer[i];
			mean_arrivals_no_timer += pi_d[i]*expected_arrivals_no_timer[i];
		}
		
		printf("- mean_arrivals_timer = %f\n", mean_arrivals_timer);
		printf("- mean_arrivals_no_timer = %f\n", mean_arrivals_no_timer);
		
	}
	if(logs_enabled) printf("---------------------------\n");
	if(logs_enabled) printf("\n");
};

/**
 * Called when a new transaction is generated by the traffic generator (refer to "traffic_generator.h")
 */
void Queue :: InportNewPacketGenerated(){

	//if(logs_enabled) printf("%.12f; New packet received from the traffic generator\n", SimTime());

	++ num_transactions_generated;
	++ arrivals_from_state;

	// Save the state before the arrival
	previous_state = current_state;

	// Update occupancy measurements
	occupancy_measurements += buffer.QueueSize();
	occupancy_array[buffer.QueueSize()] += 1;
	++ num_occupancy_measurements;

	// If the queue is not full, add the transaction
	if (buffer.QueueSize() < queue_size) {
		// Include new packet
		Transaction new_transaction;
		new_transaction.timestamp_generated = SimTime();
		new_transaction.transaction_id = transaction_counter;
		buffer.PutPacket(new_transaction);
	} else { // Otherwise, discard the new transaction		
		//if(logs_enabled) printf("%.12f; A new packet has been dropped! (queue: %d/%d)\n", SimTime(), buffer.QueueSize(), queue_size);
		++ num_transactions_dropped;
	}

	// Increase the transactions counter
	++ transaction_counter;

	current_state = buffer.QueueSize();

	if (previous_state != current_state) {		
		// Update prob. of watching "i" packets from departure state "j"
		time_in_k_from_d_state[previous_state] = SimTime() - timestamp_last_state;
		// Update the total time in state "i"
		time_per_state[previous_state] += SimTime() - timestamp_last_state;
		timestamp_last_state = SimTime();
	}

	// Check if mining is possible
	CheckMining();
	
}

/**
 * Called to start mining
 */
void Queue :: SetMiningTimeout(){
	// Set the timeout if it is not active
	if (!trigger_toStartMining.Active()) {
		timeout_active = 1;
		double time_to_trigger = SimTime()+timeout_mining;
		trigger_toStartMining.Set(time_to_trigger);
		//if(logs_enabled) printf("%.12f; - Next mining timeout in %f s (%f)\n", SimTime(), timeout_mining, time_to_trigger);
	}
}

/**
 * Check if mining is possible
 */
void Queue :: CheckMining() {

	//if(logs_enabled) printf("%.12f; Check mining (mining active: %d)\n", SimTime(), mining_active);
	
	// Mine only if there is not an ongoing mining operation and there are enough transactions in the queue
	if (!mining_active && buffer.QueueSize() >= batch_size) {
		// Cancel previous trigger (timeout)
		if (trigger_toStartMining.Active()) {
			//if(logs_enabled) printf("%.12f; - Cancelling timeout for mining in %f s\n", SimTime(), trigger_toStartMining.GetTime());
			trigger_toStartMining.Cancel();
			timeout_active = 0;
		}
		// Start mining now
		trigger_toStartMining.Set(SimTime());	
	}

};

/**
 * Triggered to start mining
 */
void Queue :: StartMining(trigger_t &){

	// Enable the mining flag
	mining_active = 1;	

	// Check if mining was triggered by a timeout
	if (timeout_active) {
		timer_expired = 1;
		++times_timeout_expired;	
		++times_timeout_expired_from_d_state[current_d_state];
		++times_timeout_expired_from_state[current_state];
	}
	
	// Disable the timeout flag
	timeout_active = 0;	

	// Set mining trigger according to an exponential random variable
	for (int i = 0; i < n_miners; ++i) {
		delay_miners[i] = Exponential(1/departures_rate);
	}
	qsort(delay_miners, n_miners, sizeof(double), compare);
	double mining_time = delay_miners[0];
	double time_to_trigger = SimTime() + mining_time;
	trigger_toFinishMining.Set(time_to_trigger);

	// Set the number of transactions to be mined
	if (buffer.QueueSize() < batch_size) {
		num_transaction_to_be_mined = buffer.QueueSize();
	} else {
		num_transaction_to_be_mined = batch_size;
	}

	// In case of forks, check if packets need to remain in the queue
	propagation_time = (HEADER_LENGTH + num_transaction_to_be_mined*TRANSACTION_LENGTH)/capacity_p2p;
	//(HEADER_LENGTH + block_size(m)*TRANSACTION_LENGTH)/mean(C_p2p) * n_hops;
	if (n_miners > 1) {
		if (delay_miners[1]-delay_miners[0] < propagation_time) {
			num_transaction_to_be_mined = 0;
			++n_forks;		
		}
	}
		
	//if(logs_enabled) printf("%.12f; Mining a block in %f s (%d transactions) \n", SimTime(), mining_time, num_transaction_to_be_mined);

};


/**
 * Called when mining finishes
 */
void Queue :: MiningFinished(trigger_t &){

	++n_blocks;
	
	//if(logs_enabled) printf("%.12f; S%d Mining finished - Removing %d transactions from queue\n", 
	//	SimTime(), current_d_state, num_transaction_to_be_mined);

	// Save the state before mining
	previous_state = current_state;	

	// Update prob. of watching "i" packets from departure state "j"
	time_in_k_from_d_state[current_state] = SimTime() - timestamp_last_state;
	// Update measurements
	double time_in_state = 0;
	for (int i = 0; i < queue_size + 1; ++i) {
	//	printf("Time in s%d with k=%d = %f\n",current_state,i,time_in_k_from_d_state[i]);
		time_in_state += time_in_k_from_d_state[i]; 
	}
	
	double time_since_last_departure = SimTime() - timestamp_last_d_state;
	//printf("Time in departure state %d = %f\n",current_d_state,SimTime()-timestamp_last_d_state);
	for (int i = 0; i < queue_size + 1; ++i) {
		//printf("Time k=%d from dep. state %d = %f\n",i,current_d_state,time_in_k_from_d_state[i]);
		if (time_since_last_departure >= 0) {
			queue_status_from_departure[current_d_state][i] += time_in_k_from_d_state[i]; 
		}	
		time_in_k_from_d_state[i] = 0;	
	}
	//printf("------------------\n");
	time_in_d_state[current_d_state] += time_since_last_departure;

	sum_time_spent_per_d_state[current_d_state] += time_since_last_departure;
	++measurements_time_per_d_state[current_d_state];
	
	// Remove transactions that have been mined
	for (int i = 0; i < num_transaction_to_be_mined; ++ i) {
		++ num_mined_transactions;
		delay_measurements += (SimTime() - buffer.GetFirstPacket().timestamp_generated);
		buffer.DelFirstPacket();
	}
	//if(logs_enabled) printf("%.12f; - Updated queue status: %d/%d\n", SimTime(), buffer.QueueSize(), queue_size);


	// 
	if (timer_expired) {
		arrivals_from_state_timer[current_d_state] += arrivals_from_state;
		++counter_arrivals_timer[current_d_state];
	} else {
		arrivals_from_state_no_timer[current_d_state] += arrivals_from_state;
		++counter_arrivals_no_timer[current_d_state];
	}
	arrivals_from_state = 0;
	timer_expired = 0;
	
	current_state = buffer.QueueSize();
	current_d_state = buffer.QueueSize();	
	timestamp_last_d_state = SimTime();

	if (previous_state != current_state) {		
		time_per_state[previous_state] += SimTime() - timestamp_last_state;
		timestamp_last_state = SimTime();
	}

	// Update departure status
	departure_array[buffer.QueueSize()] += 1;
	++num_departure_measurements;

	// Update the measurements to compute the average inter-departure duration
	sum_duration_epoch += (SimTime()-timestamp_current_epoch);
	++counter_sum_duration_epoch;

	// Update the timestamp at which the next epoch begins
	timestamp_current_epoch = SimTime();

	// Disable the mining flag
	mining_active = 0;

	// Set a new mining timeout
	SetMiningTimeout();

	// Check if mining is possible
	CheckMining();
	
};

/*
 * Initialize variables
 */
void Queue :: InitializeVariables() {

	mining_active = 0;

	delay_measurements = 0;
	occupancy_measurements = 0;
	num_occupancy_measurements = 0;
	num_departure_measurements = 0;	

	num_transactions_generated = 0;
	num_transactions_dropped = 0;
	transaction_counter = 0;
	num_transaction_to_be_mined = 0;
	num_mined_transactions = 0;

	timeout_active = 0;
	times_timeout_expired = 0;
	times_timeout_expired_from_d_state = new int[queue_size+1];
	times_timeout_expired_from_state = new int[queue_size+1];

	departure_array = new int[queue_size+1];
	occupancy_array = new int[queue_size+1];

	time_per_state = new double[queue_size+1];

	queue_status_from_departure = new double*[queue_size+1]; 
	time_in_d_state = new double[queue_size+1]; 
	time_in_k_from_d_state = new double[queue_size+1]; 

	sum_time_spent_per_d_state = new double[queue_size+1]; 
	measurements_time_per_d_state = new int[queue_size+1];
	
	current_state = 0;
	timestamp_last_state = 0;
	current_d_state = 0;
	timestamp_last_d_state = 0;

	delay_miners = new double[n_miners];
	for (int i = 0; i < n_miners; ++i){
		delay_miners[i] = 0;
	}

	n_blocks = 0;
	n_forks = 0;

	timer_expired = 0;
	arrivals_from_state = 0; 
	arrivals_from_state_timer = new int[queue_size+1]; 
	arrivals_from_state_no_timer = new int[queue_size+1]; 
	counter_arrivals_timer = new int[queue_size+1];
	counter_arrivals_no_timer = new int[queue_size+1];

	expected_arrivals_timer = new double[queue_size+1];
	expected_arrivals_no_timer = new double[queue_size+1];

	for(int i = 0; i < queue_size+1; ++i){
		departure_array[i] = 0;
		occupancy_array[i] = 0;
		time_per_state[i] = 0;
		time_in_k_from_d_state[i] = 0;
		sum_time_spent_per_d_state[i] = 0;
		measurements_time_per_d_state[i] = 0;
		queue_status_from_departure[i] = new double[queue_size+1];
		time_in_d_state[i] = 0;
		times_timeout_expired_from_d_state[i] = 0;
		times_timeout_expired_from_state[i] = 0;
		arrivals_from_state_timer[i] = 0;
		arrivals_from_state_no_timer[i] = 0;
		counter_arrivals_timer[i] = 0;
		counter_arrivals_no_timer[i] = 0;
		expected_arrivals_timer[i] = 0;
		expected_arrivals_no_timer[i] = 0;
		for(int j = 0; j < queue_size; ++j){
			queue_status_from_departure[i][j] = 0;
		}
	}

	timestamp_current_epoch = 0;
	sum_duration_epoch = 0;
	counter_sum_duration_epoch = 0;

}
