#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>

//WARNING: The max elapsed time calculated by this class is 50 days (4,294,967,295 milliseconds)
class Clock
{
    public:
        Clock();

        static unsigned long currentMillis();

        void startTimer();
        void stopTimer();
        bool hasStarted();
        unsigned long getElapsedTime();
        
        #ifdef TEST
        static void setClockOffset(unsigned long milliseconds);
        #endif
    
    private:
        unsigned long startTime = 0;
        bool started = false;
        #ifdef TEST
        static unsigned long clockOffset;
        #endif
};

#endif