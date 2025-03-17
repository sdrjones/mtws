#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"
#include "TriggaHappy.h"
#include "NotchFilter.h"
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

TriggaHappy::TriggaHappy() : notchFilter(NotchFilter::Q100)
{
    clearBuffers();
    resetPointers();
    halftime = false;
    startupCounter = 400;

    for (int g = 0; g < kMaxGrains; ++g)
    {
        grains[g].currentIndex = 0;
        grains[g].sizeSamples = kMaxGrainSize;
        grains[g].startIndex = rand() % kBufSize;
        grains[g].level = 0;
        grains[g].pan = 128;
        grains[g].pitch = Normal;
        grains[g].subIndex = 0;
        grains[g].sleepCounter = 400;
    }
}

void TriggaHappy::ProcessSample()
{

    halftime = !halftime;

    if (startupCounter)
    {
        startupCounter--;
    }

    if (startupCounter == 0)
    {
        ReadKnobs();
        Switch s = SwitchVal();
        ReadAudio();

        if (halftime)
        {
            ReadCV();
        }

        uint16_t grainCount = 6;
        uint32_t startRange = kBufSize;
        uint32_t sizeRange = kMaxGrainSize;
        uint16_t repeatChance = cv1;
        uint16_t sleepChance = cv2;
        
        uint16_t inputShift = 0;

        if (Connected(Input::Audio1) && Connected(Input::Audio2))
        {
            inputShift = 1;
        }

        int16_t audioM = (audioL + audioR) >> inputShift;

        audioM = notchFilter.ProcessSample(audioM);

        int16_t maxWet = 0;

        int16_t wetL = 0;
        int16_t wetR = 0;

        for (unsigned int g = 0; g < kMaxGrains; ++g)
        {
            Grain &grain = grains[g];

#ifdef SLEEP_CHANCE
            if (grain.sleepCounter > 0)
            {
                grain.sleepCounter--;


                if (grain.sleepCounter == 0)
                {
 
                    uint16_t sleepRand = rnd12() >> 1;
                    if (sleepRand > sleepChance)
                    {
                        uint32_t sleepSize = grain.sizeSamples;
                        if (sleepSize < kMinSleepSize)
                        {
                            sleepSize = kMinSleepSize;
                        }
                        grain.sleepCounter = sleepSize;
                    }
                }
                continue;
            }
#endif

            // Randomize grain start position and size if the grain has finished
            if ((grain.currentIndex >= grain.sizeSamples) && (g < grainCount))
            {
                grain.currentIndex = 0;
#ifdef REPEAT_CHANCE
                uint16_t repeatRnd = rnd12() >> 1;
                if (repeatRnd < repeatChance)
                {
                    // Get a new set of attributes

#endif
                    // put the grain's old level back
                    // in the headroom
                    headRoom += grain.level;

                    uint32_t maxSize = (sizeRange * yKnob) >> 12;
                    if (maxSize < kMinGrainSize)
                    {
                        maxSize = kMinGrainSize;
                    }

                    uint32_t nextSize = (rndi32() % (maxSize - kMinGrainSize)) + kMinGrainSize;

                    if (headRoom > 4096)
                    {
                        headRoom = 4096;
                    }

                    grain.startIndex = (writeI + kBufSize - (rndi32() % startRange)) % kBufSize;
                    // grain.sizeSamples = static_cast<unsigned int>((0.01f + static_cast<float>(rand()) / RAND_MAX 0.49f) context->audioSampleRate);

                    grain.sizeSamples = nextSize;

                    grain.pan = rnd8(); // pan indexes the power pan array that has 256 entries
                    if (grain.pan > 255)
                    {
                        grain.pan = 255;
                    }
                    uint32_t rndLevel = rnd12();
                    uint32_t pitchRand = rnd12();

                    grain.subIndex = 0;

                    // Calculate the distance from the write pointer to figure out if we can play the grain
                    // at a different rate from the rate at which we are writing to audioBuf
                    uint16_t distBehind = distance_in_circular_buffer(grain.startIndex, writeI, kBufSize);
                    uint16_t distAhead = kBufSize - distBehind;

                    if ((pitchRand < (xKnob >> 1)) && ((distBehind >> 1) > grain.sizeSamples))
                    {
                        grain.pitch = OctaveHigh;
                    }
                    else if ((pitchRand < (xKnob)) && ((distAhead >> 1) > grain.sizeSamples))
                    {
                        grain.pitch = OctaveLow;
                        grain.subIndex = 1;
                    }
                    else
                    {
                        grain.pitch = Normal;
                    }

                    // Set the level to the random level
                    grain.level = rndLevel;

                    if (grain.level < kGrainSilenceThreshold)
                    {
                        grain.level = 0;
                    }

                    // Check there is enough headroom to take that level
                    if (grain.level > headRoom)
                    {
                        grain.level = headRoom;
                        headRoom = 0;
                    }
                    else
                    {
                        // Remove the level from the headroom
                        headRoom -= grain.level;
                    }
#ifdef REPEAT_CHANCE
                }
                else
                {
                    // We want to repeat the grain but that might be problematic due to the 
                    // changed write position so double check that and revert to pitch normal
                    // (where we will never overtake or get undertaken by the record head)
                    uint16_t distBehind = distance_in_circular_buffer(grain.startIndex, writeI, kBufSize);
                    uint16_t distAhead = kBufSize - distBehind;
                    if ((grain.pitch == OctaveHigh) && ((distBehind >> 1) > grain.sizeSamples))
                    {
                        grain.pitch = OctaveHigh;
                    }
                    else if ((grain.pitch == OctaveLow) && ((distAhead >> 1) > grain.sizeSamples))
                    {
                        grain.pitch = OctaveLow;
                        grain.subIndex = 1;
                    }
                    else
                    {
                        grain.pitch = Normal;
                    }
                }
#endif
            }

            // Save some processing if the grain is silent
            int16_t grainSample = 0;
            if (grain.level > 0)
            {
                unsigned int grainReadIndex = (grain.startIndex + grain.currentIndex) % kBufSize;
                grainSample = audioBuf[grainReadIndex];

                // Apply level
                grainSample = static_cast<uint16_t>((grainSample * grain.level) >> 12);

                // If we're near the start of end of the grain then we fade in/out
                // using a Hann window lookup table
                // As the hann window is symmetrical I'm using half of one to save
                // on space

                uint32_t hannIndex = grain.currentIndex;
                if (hannIndex > kHalfHannSize)
                {
                    hannIndex = grain.sizeSamples - hannIndex;
                }
                if (hannIndex < kHalfHannSize)
                {
                    uint32_t faded32 = grainSample * kHannWindowFirstHalf[hannIndex];
                    grainSample = static_cast<uint16_t>(faded32 >> 15);
                }

                // Pan the grain into wet left and right signals
                wetL += static_cast<int16_t>(grainSample * kLeftGains[grain.pan] >> 12);
                wetR += static_cast<int16_t>(grainSample * kLeftGains[255 - grain.pan] >> 12);
            }
            // Use grain sub index if necessary to pitch shift
            if (grain.pitch == OctaveLow)
            {
                grain.subIndex = !grain.subIndex;
            }

            if (grain.subIndex == 0)
            {
                grain.currentIndex++;

                if (grain.pitch == OctaveHigh)
                {
                    grain.currentIndex++;
                }
            }

            if (grain.currentIndex >= grain.sizeSamples)
            {
                // When the grain has finished then we add it's old level back to
                // the headRoom
                // headRoom += grain.level;
                // grain.level = 0;
#ifdef SLEEP_CHANCE

                uint16_t sleepRand = rnd12() >> 1;
                if (sleepRand > sleepChance)
                {
                    uint32_t sleepSize = grain.sizeSamples;
                    if (sleepSize < kMinSleepSize)
                    {
                        sleepSize = kMinSleepSize;
                    }
                    grain.sleepCounter = sleepSize;
                }
#endif
            }

            if (g < 6)
            {
                LedBrightness(g, abs(grainSample) << 2);
            }
        }

        // audio buffer record debug
        // this "debug" signal sounds nice when mixed in with the granulated
        // playback so maybe worth mixing this into tehe output somehow?
        // wetL = audioBuf[readI];

        // Right shift dry signal volume by more in order to match the perceived
        // level of the wet signal

        int16_t dryL = 0;
        int16_t dryR = 0;

        // dryL = audioL;
        // dryR = audioR;

        // if (s!= Switch::Up)
        //{
        dryL = dryR = audioBuf[readI];
        //}

        int16_t mixOutL = (dryL * (mainKnob) >> 13) + (wetL * (4095 - (mainKnob)) >> 12);
        int16_t mixOutR = (dryR * (mainKnob) >> 13) + (wetR * (4095 - (mainKnob)) >> 12);

        bool shouldRecord = false;

        // Currently Middle is don't record,
        // either up or down is record
        if (s != Switch::Middle)
        {
            shouldRecord = true;
        }

        // The record state machine aims to crossfade between the buffer and
        // live input when starting/stopping recording so as not to
        // get a glitch
        // I'm using the  hann
        switch (recordState)
        {
        case RecordStateOn:
            audioBuf[writeI] = audioM;
            if (!shouldRecord)
            {
                recordState = RecordStateEnteringOff;
            }
            break;

        case RecordStateEnteringOn:
        {

            uint32_t fadedIn = audioM * kHannWindowFirstHalf[recordStateHannIndex];
            uint32_t fadedBuf = audioBuf[writeI] * (kHannWindowFirstHalf[kHalfHannSize - recordStateHannIndex]);

            // Because this is a crossfade there is no need to right shift the
            // sum result
            uint32_t fadedSum = (fadedIn + fadedBuf);

            audioBuf[writeI] = static_cast<uint16_t>(fadedSum >> 15);
            recordStateHannIndex++;
            if (!shouldRecord)
            {
                recordState = RecordStateEnteringOff;
            }
            else if (recordStateHannIndex >= kHalfHannSize)
            {
                recordState = RecordStateOn;
            }
        }
        break;

        case RecordStateEnteringOff:
        {
            uint32_t fadedIn = audioM * kHannWindowFirstHalf[recordStateHannIndex];
            uint32_t fadedBuf = audioBuf[writeI] * (kHannWindowFirstHalf[kHalfHannSize - recordStateHannIndex]);

            uint32_t fadedSum = (fadedIn + fadedBuf);

            audioBuf[writeI] = static_cast<uint16_t>(fadedSum >> 15);
            recordStateHannIndex--;
            if (shouldRecord)
            {
                recordState = RecordStateEnteringOn;
            }
            else if (recordStateHannIndex == 0)
            {
                recordState = RecordStateOff;
            }
        }
        break;

        case RecordStateOff:
            if (shouldRecord)
            {
                recordState = RecordStateEnteringOn;
            }

        default:
            break;
        }

        lastSwitch = s;

        writeI = (writeI + 1) % kBufSize;
        readI = (readI + 1) % kBufSize;

        AudioOut1(mixOutL);
        AudioOut2(mixOutR);
    }
}

void TriggaHappy::clearBuffers(void)
{
    for (int i = 0; i < kBufSize; ++i)
    {
        audioBuf[i] = 0;
    }
}

void TriggaHappy::resetPointers(void)
{
    writeI = 1;
    readI = 0;
}

void TriggaHappy::ReadKnobs(void)
{
    // Virtual detent the knob values
    mainKnob = virtualDetentedKnob(KnobVal(Knob::Main));
    xKnob = virtualDetentedKnob(KnobVal(Knob::X));
    yKnob = virtualDetentedKnob(KnobVal(Knob::Y));
}

void TriggaHappy::ReadAudio(void)
{
    audioL = AudioIn1(); // -2048 to 2047
    audioR = AudioIn2(); // -2048 to 2047
}

void TriggaHappy::ReadCV(void)
{
    cv1 = Connected(ComputerCard::Input(Input::CV1)) ? CVIn1() : 2000; // -2048 to 2047
    cv2 = Connected(ComputerCard::Input(Input::CV2)) ? CVIn2() : 2000; // -2048 to 2047

    if (cv1 > 2000)
        cv1 = 2000;
    if (cv1 < 0)
        cv1 = 0;

    if (cv2 > 2000)
        cv2 = 2000;
    if (cv2 < 0)
        cv2 = 0;
}