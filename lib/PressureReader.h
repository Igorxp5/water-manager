#ifndef PRESSURE_READER_H
#define PRESSURE_READER_H

class PressureReader
{
    public:
        PressureReader(unsigned int pin, double pressureFactor);

        virtual double getValue();

    protected:
        unsigned int pin;
        double pressureFactor;
};

#endif
