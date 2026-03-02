#ifndef CORES3_NIGHTSCOUT_H
#define CORES3_NIGHTSCOUT_H

#include "config.h"

/**
 * Poll Nightscout API and populate NSinfo struct.
 * Makes up to 2 HTTP requests:
 *   1. /api/v1/entries.json?count=10 — SGV data
 *   2. /api/v2/properties/... — delta (skipped for Sugarmate)
 *
 * Returns 0 on success, error code on failure.
 * Errors are also logged to errLog.
 */
int readNightscout(const Config &cfg, NSinfo &ns, ErrorLog &errLog);

#endif // CORES3_NIGHTSCOUT_H
