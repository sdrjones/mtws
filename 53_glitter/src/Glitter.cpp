#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"
#include "Glitter.h"
#include "Utils.h"
#include "HannWindow.h"
#include "PowerPan.h"

#define SLEEP_CHANCE 1
#define REPEAT_CHANCE 1

uint32_t __not_in_flash_func(rnd8)()
{
    static uint32_t lcg_seed = 1;
    lcg_seed = 1664525 * lcg_seed + 1013904223;
    return lcg_seed >> 24;
}

uint32_t __not_in_flash_func(rnd12)()
{
    static uint32_t lcg_seed = 1;
    lcg_seed = 1664525 * lcg_seed + 1013904223;
    return lcg_seed >> 20;
}
uint32_t __not_in_flash_func(rnd16)()
{
    static uint32_t lcg_seed = 1;
    lcg_seed = 1664525 * lcg_seed + 1013904223;
    return lcg_seed >> 16;
}
uint32_t __not_in_flash_func(rnd24)()
{
    static uint32_t lcg_seed = 1;
    lcg_seed = 1664525 * lcg_seed + 1013904223;
    return lcg_seed >> 8;
}

int32_t __not_in_flash_func(rndi32)()
{
    static uint32_t lcg_seed = 1;
    lcg_seed = 1664525 * lcg_seed + 1013904223;
    return lcg_seed;
}

uint32_t __not_in_flash_func(distance_in_circular_buffer)(uint32_t a, uint32_t b, uint32_t buffer_size)
{
    if (a <= b)
    {
        return b - a;
    }
    else
    {
        return buffer_size - a + b;
    }
}

Glitter::Glitter()
{
    clearBuffers();
    resetPointers();
    ReadKnobs();
    halfTime_ = false;
    startupCounter_ = 400;
    recordStateHannIndex_ = 0;
    recordState_ = RecordStateOff;
    curSwitch_ = SwitchVal();
    clockCount_ = 0;
    samplesPerPulse_ = 0;
    samplesMultiplier_ = 0;
    clockLed_ = 0;
    clockState_ = ClockOff;
    maxClockShiftDown_ = 1;
    maxClockShiftUp_ = 1;
    pitchChance_ = 0;

    
    for (int g = 0; g < kMaxGrains; ++g)
    {
        grains_[g].currentIndex_ = 0;
        grains_[g].sizeSamples_ = kMaxGrainSize;
        grains_[g].startIndex_ = rand() % kBufSize;
        grains_[g].level_ = 0;
        grains_[g].pan_ = 128;
        grains_[g].pitch_ = Normal;
        grains_[g].intendedPitch_ = Normal;
        grains_[g].subIndex_ = 0;
        grains_[g].sleepCounter_ = 400;
    }
}

void Glitter::ProcessSample()
{

    halfTime_ = !halfTime_;

    if (startupCounter_)
    {
        startupCounter_--;
    }

    if (startupCounter_ == 0)
    {


        ReadAudio();

        if (halfTime_)
        {
            ReadKnobs();
            ReadCV();
            curSwitch_ = SwitchVal();
        }

        uint16_t grainCount = 6;
        uint32_t startRange = kBufSize;
        uint32_t sizeRange = kMaxGrainSize;
        uint16_t repeatChance = cv1_;
        uint16_t sleepChance = cv2_;
        
        uint16_t inputShift = 0;

        if (Connected(Input::Audio1) && Connected(Input::Audio2))
        {
            inputShift = 1;
        }

        if (Connected(Input::Pulse1))
        {
            switch (clockState_){
                case ClockOff:
                {   
                    clockState_ = ClockWaitingFirstPulse;
                    samplesPerPulse_ = 0;
                }
                break;

                case ClockWaitingFirstPulse:
                {
                    if (PulseIn1RisingEdge())
                    {
                        clockCount_ = 0;
                        clockState_ = ClockWaitingSecondPulse;
                    }
                }
                break;

                case ClockWaitingSecondPulse:
                case ClockRunning:
                {
                    clockCount_++;
                    if (PulseIn1RisingEdge())
                    {
                        if (abs(clockCount_ - samplesPerPulse_) > kClockChangeThreshold)
                        {
                            //Use this to debug clock changes
                            //LedBrightness(5, clockLed_ * 2048);
                            //clockLed_ = (clockLed_ + 1) % 2;

                            samplesPerPulse_ = clockCount_;
                            maxClockShiftDown_ = 0;
                            maxClockShiftUp_ = 0;

                            uint64_t samplesPerQuaver_ = samplesPerPulse_ << 1;

                            if ((samplesPerQuaver_ * 6) <= kBufSize)
                            {
                                samplesMultiplier_ = samplesPerQuaver_;
                            }
                            else if ((samplesPerQuaver_ * 3) <= kBufSize)
                            {
                                samplesMultiplier_ = samplesPerPulse_;
                            }
                            else
                            {
                                samplesMultiplier_ = samplesPerPulse_ >> 1;
                            }
                            
                            uint32_t tmp = kMinGrainSize;
                            while ((tmp < samplesPerPulse_) && (maxClockShiftDown_ <= kAbsMaxClockShift))
                            {
                                tmp = tmp << 1;
                                maxClockShiftDown_++;
                            }

                            tmp = kMaxGrainSize;
                            while ((tmp  >= samplesPerPulse_) && (maxClockShiftUp_ <= kAbsMaxClockShift))
                            {
                                tmp = tmp << 1;
                                maxClockShiftUp_++;
                            }
                        }
                        clockCount_ = 0;
                        clockState_ = ClockRunning;
                    }
                    else if (clockCount_ > kMaxSamplesBetweenClocks)
                    {
                        clockCount_ = 0;
                        clockState_ = ClockWaitingFirstPulse;
                    }

                }
                break;

                default:
                    break;

            }

        }
        else
        {
            clockState_ = ClockOff;
        }

        int16_t audioM = (audioL_ + audioR_) >> inputShift;

        int16_t maxWet = 0;

        int16_t wetL = 0;
        int16_t wetR = 0;

        for (unsigned int g = 0; g < kMaxGrains; ++g)
        {
            Grain &grain = grains_[g];

#ifdef SLEEP_CHANCE
            if (grain.sleepCounter_ > 0)
            {
                grain.sleepCounter_--;


                if (grain.sleepCounter_ == 0)
                {
 
                    uint16_t sleepRand = rnd12() >> 1;
                    if (sleepRand > sleepChance)
                    {
                        uint32_t sleepSize = grain.sizeSamples_;
                        if (sleepSize < kMinSleepSize)
                        {
                            sleepSize = kMinSleepSize;
                        }
                        grain.sleepCounter_ = sleepSize;
                    }
                }
                continue;
            }
#endif

            // Randomize grain start position and size if the grain has finished
            if ((grain.currentIndex_ >= grain.sizeSamples_) && (g < grainCount))
            {
                grain.currentIndex_ = 0;
#ifdef REPEAT_CHANCE
                uint16_t repeatRnd = rnd12() >> 1;
                if (repeatRnd < repeatChance)
                {
                    // Get a new set of attributes

#endif
                    // put the grain's old level back
                    // in the headroom
                    headRoom_ += grain.level_;

                    uint32_t maxSize = (sizeRange * yKnob_) >> 12;
                    if (maxSize < kMinGrainSize)
                    {
                        maxSize = kMinGrainSize;
                    }

                    uint64_t nextSize = 0;
                    if (clockState_ != ClockRunning)
                    {
                        nextSize = (rndi32() % (maxSize - kMinGrainSize)) + kMinGrainSize;
                    }
                    else
                    {
                        uint32_t shift = 0;
                        shift = ((maxClockShiftDown_ + maxClockShiftUp_ + 1) * (4095 - yKnob_)) >> 12;


                        nextSize = (samplesPerPulse_ << maxClockShiftUp_) >> shift;
    

                        if (nextSize < kMinGrainSize)
                        {
                            // This will be out of beat so
                            // indicates that something has gone wrong
                            // with the maxClockShiftDown_ logic
                            nextSize = kMinGrainSize;
                        }
                        else if (nextSize > kMaxGrainSize)
                        {
                            nextSize = kMaxGrainSize;
                        }
                    }

                    if (headRoom_ > 4096)
                    {
                        headRoom_ = 4096;
                    }

                    uint64_t offset = 0;
                    if (clockState_ != ClockRunning)
                    {
                        offset = rndi32() % startRange;
                    }
                    else
                    {
                        offset = (samplesMultiplier_) * g;
                    }

                    grain.startIndex_ = (lastRecordedWriteI_ + kBufSize - (offset)) % kBufSize;
                    
                    grain.sizeSamples_ = nextSize;

                    grain.pan_ = rnd8(); // pan indexes the power pan_ array that has 256 entries
                    if (grain.pan_ > 255)
                    {
                        grain.pan_ = 255;
                    }
                    uint32_t rndLevel = rnd12();
                    uint32_t pitchRand = rnd12();

                    grain.subIndex_ = 0;

                    // Calculate the distance from the write pointer to figure out if we can play the grain
                    // at a different rate from the rate at which we are writing to audioBuf
                    uint16_t distBehind = distance_in_circular_buffer(grain.startIndex_, lastRecordedWriteI_, kBufSize);
                    uint16_t distAhead = kBufSize - distBehind;

                    if ((pitchRand < (pitchChance_ >> 1)) && ((distBehind >> 1) > grain.sizeSamples_))
                    {
                        grain.pitch_ = OctaveHigh;
                    }
                    else if ((pitchRand < (pitchChance_)) && ((distAhead >> 1) > grain.sizeSamples_))
                    {
                        grain.pitch_ = OctaveLow;
                        grain.subIndex_ = 1;
                    }
                    else
                    {
                        grain.pitch_ = Normal;
                    }

                    grain.intendedPitch_ = grain.pitch_;

                    // Set the level to the random level
                    grain.level_ = rndLevel;

                    if (grain.level_ < kGrainSilenceThreshold)
                    {
                        grain.level_ = 0;
                    }

                    // Check there is enough headroom to take that level
                    if (grain.level_ > headRoom_)
                    {
                        grain.level_ = headRoom_;
                        headRoom_ = 0;
                    }
                    else
                    {
                        // Remove the level from the headroom
                        headRoom_ -= grain.level_;
                    }
#ifdef REPEAT_CHANCE
                }
                else
                {
                    // We want to repeat the grain but that might be problematic due to the 
                    // changed write position so double check that and revert to pitch normal
                    // (where we will never overtake or get undertaken by the record head)
                    uint16_t distBehind = distance_in_circular_buffer(grain.startIndex_, lastRecordedWriteI_, kBufSize);
                    uint16_t distAhead = kBufSize - distBehind;
                    // Reset the pitch to its original selection
                    grain.pitch_ = grain.intendedPitch_;
                    if ((grain.pitch_ == OctaveHigh) && ((distBehind >> 1) > grain.sizeSamples_))
                    {
                        grain.pitch_ = OctaveHigh;
                    }
                    else if ((grain.pitch_ == OctaveLow) && ((distAhead >> 1) > grain.sizeSamples_))
                    {
                        grain.pitch_ = OctaveLow;
                        grain.subIndex_ = 1;
                    }
                    else
                    {
                        grain.pitch_ = Normal;
                    }
                }
#endif
            }

            // Save some processing if the grain is silent
            int16_t grainSample = 0;
            if (grain.level_ > 0)
            {
                unsigned int grainReadIndex = (grain.startIndex_ + grain.currentIndex_) % kBufSize;
                grainSample = audioBuf_[grainReadIndex];

                // Apply level
                grainSample = static_cast<uint16_t>((grainSample * grain.level_) >> 12);

                // If we're near the start of end of the grain then we fade in/out
                // using a Hann window lookup table
                // As the hann window is symmetrical I'm using half of one to save
                // on space

                uint32_t hannIndex = grain.currentIndex_;
                if (hannIndex > kHalfHannSize)
                {
                    hannIndex = grain.sizeSamples_ - hannIndex;
                }
                if (hannIndex < kHalfHannSize)
                {
                    uint32_t faded32 = grainSample * kHannWindowFirstHalf[hannIndex];
                    grainSample = static_cast<uint16_t>(faded32 >> 15);
                }

                // Pan the grain into wet left and right signals
                wetL += static_cast<int16_t>(grainSample * kLeftGains[grain.pan_] >> 12);
                wetR += static_cast<int16_t>(grainSample * kLeftGains[255 - grain.pan_] >> 12);
            }
            // Use grain sub index if necessary to pitch shift
            if (grain.pitch_ == OctaveLow)
            {
                grain.subIndex_ = !grain.subIndex_;
            }

            if (grain.subIndex_ == 0)
            {
                grain.currentIndex_++;

                if (grain.pitch_ == OctaveHigh)
                {
                    grain.currentIndex_++;
                }
            }

            if (grain.currentIndex_ >= grain.sizeSamples_)
            {

#ifdef SLEEP_CHANCE

                uint16_t sleepRand = rnd12() >> 1;
                if (sleepRand > sleepChance)
                {
                    uint32_t sleepSize = grain.sizeSamples_;
                    if (sleepSize < kMinSleepSize)
                    {
                        sleepSize = kMinSleepSize;
                    }
                    grain.sleepCounter_ = sleepSize;
                }
#endif
            }

            if (g < kMaxGrains)
            {
                LedBrightness(g, abs(grainSample) << 2);
            }
        }

        // Right shift recorded signal volume by more in order to match the perceived
        // level of the wet signal
    
        int16_t mixOutL = (audioBuf_[readI_] * (mainKnob_) >> 13) + (wetL * (4095 - (mainKnob_)) >> 12);
        int16_t mixOutR = (audioBuf_[readI_] * (mainKnob_) >> 13) + (wetR * (4095 - (mainKnob_)) >> 12);

        bool shouldRecord = false;

        // Currently Middle is don't record,
        // either up or down is record
        if (curSwitch_ != Switch::Middle)
        {
            shouldRecord = true;
            if (oldSignalLevel_ < xKnob_)
            {
                oldSignalLevel_++;
            }
            else if (oldSignalLevel_ > xKnob_)
            {
                oldSignalLevel_--;
            }
        }
        else
        {
            pitchChance_ = xKnob_;
        }


        // The record state machine aims to crossfade between the buffer and
        // live input when starting/stopping recording so as not to
        // get a glitch
        // I'm using the  hann


        audioM = (audioBuf_[writeI_] * (oldSignalLevel_) >> 12) + (audioM * (4095 - (oldSignalLevel_)) >> 12);

        switch (recordState_)
        {
        case RecordStateOn:
        {
            audioBuf_[writeI_] = audioM;
            //audioBuf_[writeI_] = (audioBuf_[writeI_] * (oldSignalLevel_) >> 13) + (audioM * (4095 - (oldSignalLevel_)) >> 12);
            if (!shouldRecord)
            {
                recordState_ = RecordStateEnteringOff;
            }
            lastRecordedWriteI_ = writeI_;
        }
        break;

        case RecordStateEnteringOn:
        {

            uint32_t fadedIn = audioM * kHannWindowFirstHalf[recordStateHannIndex_];
            uint32_t fadedBuf = audioBuf_[writeI_] * (kHannWindowFirstHalf[(kHalfHannSize-1) - recordStateHannIndex_]);

            // Because this is a crossfade there is no need to right shift the
            // sum result
            uint32_t fadedSum = (fadedIn + fadedBuf);
            uint16_t fadedSum16 = static_cast<uint16_t>(fadedSum >> 15);

            audioBuf_[writeI_] = fadedSum16;
            
            recordStateHannIndex_++;
            if (!shouldRecord)
            {
                recordState_ = RecordStateEnteringOff;
            }
            else if (recordStateHannIndex_ >= (kHalfHannSize-1))
            {
                recordState_ = RecordStateOn;
            }
            lastRecordedWriteI_ = writeI_;
        }
        break;

        case RecordStateEnteringOff:
        {
            uint32_t fadedIn = audioM * kHannWindowFirstHalf[recordStateHannIndex_];
            uint32_t fadedBuf = audioBuf_[writeI_] * (kHannWindowFirstHalf[kHalfHannSize - recordStateHannIndex_]);

            uint32_t fadedSum = (fadedIn + fadedBuf);
            uint16_t fadedSum16 = static_cast<uint16_t>(fadedSum >> 15);
            audioBuf_[writeI_] = fadedSum16;
            
            recordStateHannIndex_--;
            if (shouldRecord)
            {
                recordState_ = RecordStateEnteringOn;
            }
            else if (recordStateHannIndex_ == 0)
            {
                recordState_ = RecordStateOff;
            }
            lastRecordedWriteI_ = writeI_;
        }
        break;

        case RecordStateOff:
        {
            if (shouldRecord)
            {
                recordState_ = RecordStateEnteringOn;
            }
        }
        break;

        default:
            break;
        }

        writeI_ = (writeI_ + 1) % kBufSize;
        readI_ = (readI_ + 1) % kBufSize;

        AudioOut1(mixOutL);
        AudioOut2(mixOutR);
    }
}

void Glitter::clearBuffers(void)
{
    for (int i = 0; i < kBufSize; ++i)
    {
        audioBuf_[i] = 0;
    }
}

void Glitter::resetPointers(void)
{
    writeI_ = 1;
    readI_ = 0;
}

void Glitter::ReadKnobs(void)
{
    // Virtual detent the knob values
    mainKnob_ = virtualDetentedKnob(KnobVal(Knob::Main));
    xKnob_ = virtualDetentedKnob(KnobVal(Knob::X));
    yKnob_ = virtualDetentedKnob(KnobVal(Knob::Y));
}

void Glitter::ReadAudio(void)
{
    audioL_ = AudioIn1(); // -2048 to 2047
    audioR_ = AudioIn2(); // -2048 to 2047
}

void Glitter::ReadCV(void)
{
    cv1_ = Connected(ComputerCard::Input(Input::CV1)) ? CVIn1() : kDefaultRepeatChance; // -2048 to 2047
    cv2_ = Connected(ComputerCard::Input(Input::CV2)) ? CVIn2() : kDefaultSleepChance; // -2048 to 2047

    if (cv1_ > 2000)
        cv1_ = 2000;
    if (cv1_ < 0)
        cv1_ = 0;

    if (cv2_ > 2000)
        cv2_ = 2000;
    if (cv2_ < 0)
        cv2_ = 0;
}