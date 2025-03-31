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
    static constexpr uint16_t kDefaultSleepChance = 1500; // Lower = more sleep, 0-2000
    static constexpr uint16_t kDefaultRepeatChance = 2000; // Lower = more repeats, 0-2000
    static constexpr uint64_t kMaxSamplesBetweenClocks = kBufSize / 2;
    static constexpr uint64_t kClockChangeThreshold = 48; // Ignore clock jitter lower than this
    static constexpr uint64_t kAbsMaxClockShift = 1;
    static constexpr uint8_t kDontShiftBelow = 128;

    
    
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

    enum ClockState
    {
        ClockOff,
        ClockWaitingFirstPulse,
        ClockWaitingSecondPulse,
        ClockRunning
    };
    
    struct Grain
    {
        unsigned int startIndex_;
        unsigned int sizeSamples_;
        unsigned int currentIndex_;
        unsigned int pan_;
        unsigned int level_;
        Pitch pitch_;
        Pitch intendedPitch_;
        unsigned int subIndex_;
        unsigned int sleepCounter_;
    };


    int16_t audioBuf_[kBufSize];
    
    uint32_t writeI_ = 1;
    uint32_t lastRecordedWriteI_ = 1;
    uint32_t readI_ = 0;
    Grain grains_[kMaxGrains];
    bool halfTime_ = false;    // Some controls are only read every other Process call
    uint32_t startupCounter_ = 400;
    uint16_t headRoom_ = 4096; // The "level" available for all grains
    uint64_t clockCount_ = 0; // samples between pulse 1 rising
    uint64_t samplesPerPulse_ = 0; 
    uint64_t samplesMultiplier_ = 0;
    uint64_t minClockedBeat_;
    enum ClockState clockState_ = ClockOff;
    uint16_t maxClockShiftDown_ = 1;
    uint16_t maxClockShiftUp_ = 1;
    uint32_t clockLed_ = 0;
    int32_t oldSignalLevel_ = 0;
    int16_t pitchChance_ = 0;

    

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
