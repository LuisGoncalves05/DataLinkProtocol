#ifndef _ALARM_H_
#define _ALARM_H_

struct AlarmState {
    int alarmOn;
    int alarmCount;
};

// Global alarm state intitialized in alarm.c.
extern struct AlarmState alarmState;

// Handler called when SIGALRM is received.
void alarmHandler(int signal);

// Sets up the handler for SIGALRM.
int setupAlarm();

// Starts the alarm.
void setAlarm(int timeout);

// Resets the alarm state.
void resetAlarm();

// Removes the current alarm.
void removeAlarm();

#endif