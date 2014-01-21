#ifndef _TESTER_H_
#define _TESTER_H

#include "Aithon.h"

typedef struct {
	int (*start)(void);
	char *name;
} AithonTest;
extern AithonTest tests[];


// Test function prototypes
int test_digital_pins(void);
int test_sd_init(void);
int test_buzzer(void);

#endif