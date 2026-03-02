#ifndef CORES3_ALERTS_H
#define CORES3_ALERTS_H

#include "config.h"

/**
 * Alarm state tracker. Handles timing for repeat alarms and snooze.
 */
struct AlarmState {
    time_t lastAlarmTime = 0;
    time_t snoozeUntil   = 0;
    int    snoozeMult    = 0;

    /** Snooze alarms for cfg.snooze_timeout minutes (stacks on repeat press). */
    void snooze(int timeout_min);

    /** Returns seconds remaining in snooze, or 0. */
    int snoozeRemaining() const;

    /** Returns true if enough time has passed to fire alarm again. */
    bool shouldFire(int alarm_repeat_min) const;

    /** Record that an alarm just fired. */
    void recordFired();
};

/**
 * Check alarm conditions and play sound if needed.
 * Call every loop iteration after Nightscout data is updated.
 */
void checkAlarms(const Config &cfg, const NSinfo &ns, AlarmState &alarm);

#endif // CORES3_ALERTS_H
