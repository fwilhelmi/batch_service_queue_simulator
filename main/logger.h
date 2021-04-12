/**
 * logger.h: this file defines a LOGGER to generate logs
 */

#ifndef _AUX_LOGGER_
#define _AUX_LOGGER_

struct Logger
{
	int save_logs;					///> Flag for activating the log writting
	FILE *file;					///> File for writting logs
	char head_string[8];			///> Header string (to be passed as argument when it is needed to write info from other class or component)

	/**
	 * Creates an empty head string
	 */
	void SetVoidHeadString(){
		sprintf(head_string, "%s", " ");
	}

};

#endif
