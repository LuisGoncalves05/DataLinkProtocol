#include "alarm.h"
#include <signal.h>
#include <unistd.h>

struct AlarmState alarmState = {
    .alarmOn = 0,
    .alarmCount = 0};

void alarmHandler(int signal) {
    alarmState.alarmOn = 0;
    alarmState.alarmCount++;
}

int setupAlarm() {
    struct sigaction act = {0};
    act.sa_handler = &alarmHandler;
    if (-1 == sigaction(SIGALRM, &act, NULL)) {
        return -1;
    }

    return 0;
}

void setAlarm(int timeout) {
    alarmState.alarmOn = 1;
    alarm(timeout);
}

void resetAlarm() {
    alarmState.alarmOn = 0;
    alarmState.alarmCount = 0;
}

void removeAlarm() {
    resetAlarm();
    alarm(0);
}