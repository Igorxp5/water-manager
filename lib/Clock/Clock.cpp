#include "Clock.h"

#ifdef TEST
unsigned long Clock::clockOffset = 0L;
#endif

Clock::Clock() {

}

unsigned long Clock::currentMillis() {
    #ifndef TEST
    return millis();
    #else
    return Clock::clockOffset + millis();
    #endif
}

#ifdef TEST
void Clock::setClockOffset(unsigned long milliseconds) {
    Clock::clockOffset = milliseconds;
}
#endif

void Clock::startTimer() {
    this->startTime = Clock::currentMillis();
    this->started = true;
}

void Clock::stopTimer() {
    this->started = false;
}

bool Clock::hasStarted() {
    return this->started;
}

unsigned long Clock::getElapsedTime() {
    unsigned long currentTime = Clock::currentMillis();
    unsigned long elapsedTime = 0L;
    if (currentTime < this->startTime) {
        //Long overflow
        elapsedTime = UINT32_MAX - this->startTime;
        elapsedTime += currentTime;
    } else {
        elapsedTime = currentTime - this->startTime;
    }
    return elapsedTime;
}
