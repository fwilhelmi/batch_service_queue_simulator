/**
 * generic_methods.h:
 *
 * - This file contains generic methods to support the main simulator's operations
 */

#include <stdlib.h>

#ifndef _GENERIC_METHODS_
#define _GENERIC_METHODS_

int compare (const void * a, const void * b)
{
  if (*(double*)a > *(double*)b) return 1;
  else if (*(double*)a < *(double*)b) return -1;
  else return 0;  
}

#endif
