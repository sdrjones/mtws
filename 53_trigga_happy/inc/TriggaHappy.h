#pragma once

#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"
#include "NotchFilter.h"
#include "Random.h"


class TriggaHappy : public ComputerCard
{
public:
    TriggaHappy();
    virtual void ProcessSample();

private:
    // private methods
    void clearBuffers(void);
    void resetPointers(void);
    void ReadKnobs(void);
    void ReadInputs(void);

    static const uint32_t kMaxGrains = 3;
    static constexpr uint32_t kMaxGrainSize = 24000;
    static constexpr uint32_t kMinGrainSize = 2048;
    static constexpr uint32_t kMaxUnsigned = 4095;
    static constexpr uint32_t kMaxSigned = 2047;
    
    
    enum Pitch
    {
        OctaveHigh,
        Normal,
        OctaveLow
    };

    enum RecordState
    {
        RecordStateOn,
        RecordStateEnteringOn,
        RecordStateOff,
        RecordStateEnteringOff
    };
    
    struct Grain
    {
        unsigned int startIndex;
        unsigned int sizeSamples;
        unsigned int currentIndex;
        unsigned int pan;
        unsigned int level;
        Pitch pitch;
        unsigned int subIndex;

    };

    static constexpr uint32_t kBufSize = 2 * 48000;
    int16_t audioBuf[kBufSize];
    
    uint32_t writeI = 0;
    uint32_t readI = 2000;
    Grain grains[kMaxGrains];
    bool halftime = false;
    uint32_t startupCounter = 400;
    uint16_t headRoom = 4096;

    Switch lastSwitch;
    enum RecordState recordState = RecordStateOff;
    uint16_t recordStateHannIndex = 0;

    // Notch Filter
    NotchFilter notchFilter;

    // ui elements
    int xKnob;
    int yKnob;
    int mainKnob;

    // inputs
    int16_t cv1;
    int16_t cv2;
    int16_t audioL;
    int16_t audioR;
};
