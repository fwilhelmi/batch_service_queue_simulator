 /**
 * traffic_generator.h: this file contains the traffic generator component
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include <iostream>
#include <stdlib.h>

// Traffic generator component: "TypeII" represents components that are aware of the existence of the simulated time.
component TrafficGenerator : public TypeII{

	// Methods
	public:
		// COST
		void Setup();
		void Start();
		void Stop();

		// Generic
		void InitializeTrafficGenerator();
		void GenerateTraffic();

	// Public items
	public:

		int traffic_model;	///> Traffic model
		double traffic_load;	///> Average traffic load [transactions/s]

		int logs_enabled;	///> Flag to enable/disable logs
		
	// Connections and timers
	public:

		// INPORT connections to receive packets being generated
		inport inline void NewPacketGenerated(trigger_t& t1);

		// OUTPORT connections for sending notifications
		outport void outportNewPacketGenerated();

		// Timer ruled by the packet generation ratio
		Timer <trigger_t> trigger_new_packet_generated;

		// Connect the timer with the inport method
		TrafficGenerator () {
			connect trigger_new_packet_generated.to_component,NewPacketGenerated;
		}

};

/**
 * Setup()
 */
void TrafficGenerator :: Setup(){
	// Do nothing
};

/**
 * Start()
 */
void TrafficGenerator :: Start(){
	InitializeTrafficGenerator();
};

/**
 * Stop()
 */
void TrafficGenerator :: Stop(){
	//printf("%.12f; TrafficGenerator Stop()\n", SimTime());
};

/**
 * Main method for generating traffic
 */
void TrafficGenerator :: GenerateTraffic() {

	double time_for_next_packet (0);
	double time_to_trigger (0);

	switch(traffic_model) {

		// TRAFFIC_POISSON
		case 1:{
			time_for_next_packet = Exponential(1/traffic_load);
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(time_to_trigger);
			break;
		}

		// TRAFFIC_DETERMINISTIC
		case 2:{
			time_for_next_packet = 1/traffic_load;
			time_to_trigger = SimTime() + time_for_next_packet;
			trigger_new_packet_generated.Set(time_to_trigger);
			break;
		}


		default:{
			printf("Wrong traffic model!\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
}

/**
 * Generate a new packet upon trigger-based activation
 */
void TrafficGenerator :: NewPacketGenerated(trigger_t &){
	outportNewPacketGenerated();
	GenerateTraffic();
}

/**
 * Initialize variables and start generating traffic
 */
void TrafficGenerator :: InitializeTrafficGenerator() {
	traffic_model = 1;
	GenerateTraffic();
}
