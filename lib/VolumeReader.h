#ifndef VOLUME_READER_H
#define VOLUME_READER_H

#include "PressureReader.h"

class VolumeReader : public PressureReader
{
    public:
        VolumeReader(IOInterface* io, double pressureFactor, double volumeFactor);

        double getValue() override;
        double getPressureValue();

        double setZeroValue(double value);

    private:
        double volumeFactor;
        double zeroValue;
};

#endif
