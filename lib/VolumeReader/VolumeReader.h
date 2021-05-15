#ifndef VOLUME_READER_H
#define VOLUME_READER_H

#include "PressureReader.h"

class VolumeReader : public PressureReader
{
    public:
        VolumeReader(IOInterface* io, float pressureFactor, float volumeFactor);

        float getValue() override;
        float getPressureValue();

        float setZeroValue(float value);

    private:
        float volumeFactor;
        float zeroValue;
};

#endif
