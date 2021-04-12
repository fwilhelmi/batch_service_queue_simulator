
 /**
 * performance.h: this file defines the performance object used for statistics and the agents operation
 */

#ifndef _AUX_PERFORMANCE_
#define _AUX_PERFORMANCE_

struct Performance
{

	int total_transactions;
	int transactions_dropped;
	double drop_percentage;
	int num_blocks_mined;
	int num_blocks_mined_by_timeout;

	double mean_occupancy;
	double mean_delay;

	double fork_probability;

	/**
	 * Set the size of the array containing the RSSI in each STA from the same WLAN
	 * @param "num_stas" [type int]: number of STAs in the WLAN
	 */
	//void SetSizeOfRssiPerStaList(int num_stas){
	//	rssi_list_per_sta = new double[num_stas];
	//	for(int i = 0; i < num_stas; ++i){
	//		rssi_list_per_sta[i] = 0;
	//	}
	//}

};

#endif
