#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class Utils
{
    public:
        static void sendDebugResponse(String message);
        static void sendDebugResponse(char* message);
};

#endif
