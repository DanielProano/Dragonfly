#ifndef BAROMETER_H
#define BAROMETER_H

#include <stdbool.h>
#include "protocol.h"

bool barometer_init(void);
bool barometer_poll(barometer *out);

#endif
