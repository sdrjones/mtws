#pragma once

#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"
#include "NotchFilter.h"
#include "Random.h"


class Glitter : public ComputerCard
{
public:
    Glitter();
    void __not_in_flash_func(ProcessSample)();

private:
    // private methods
    void clearBuffers(void);
    void resetPointers(void);
    void ReadKnobs(void);
    void ReadAudio(void);
    void ReadCV(void);
    

    static constexpr uint32_t kBufSize = 2 * 48000;
    static const uint32_t kMaxGrains = 6;
    static constexpr uint32_t kMaxGrainSize = kBufSize;
    static constexpr uint32_t kMinGrainSize = 2048;
    static constexpr uint32_t kMinSleepSize = 24000;
    static constexpr uint32_t kMaxUnsigned = 4095;
    static constexpr uint32_t kMaxSigned = 2047;
    static constexpr uint32_t kGrainSilenceThreshold = 16;
    
    
    
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
        unsigned int startIndex_;
        unsigned int sizeSamples_;
        unsigned int currentIndex_;
        unsigned int pan_;
        unsigned int level_;
        Pitch pitch_;
        unsigned int subIndex_;
        unsigned int sleepCounter_;
    };


    int16_t audioBuf_[kBufSize];
    
    uint32_t writeI_ = 1;
    uint32_t readI_ = 0;
    Grain grains_[kMaxGrains];
    bool halfTime_ = false;
    uint32_t startupCounter_ = 400;
    uint16_t headRoom_ = 4096; // The "level" available for all grains
    
    Switch curSwitch_;
    enum RecordState recordState_ = RecordStateOff;
    uint16_t recordStateHannIndex_ = 0;

    // ui elements
    int xKnob_;
    int yKnob_;
    int mainKnob_;

    // inputs
    int16_t cv1_;
    int16_t cv2_;
    int16_t audioL_;
    int16_t audioR_;
};
