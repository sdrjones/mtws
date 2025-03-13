#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"
#include "TriggaHappy.h"
#include "NotchFilter.h"
#include "Utils.h"
#include "HannWindow.h"
#include "PowerPan.h"


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


uint32_t __not_in_flash_func(distance_in_circular_buffer)(uint32_t a, uint32_t b, uint32_t buffer_size) {
    if (a <= b) {
        return b - a;
    } else {
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
        ReadInputs();

        uint16_t inputShift = 0;

        if (Connected(Input::Audio1) && Connected(Input::Audio2))
        {
          inputShift = 1;
        }
        
        int16_t audioM = (audioL + audioR) >> inputShift;
        

        audioM = notchFilter.ProcessSample(audioM);

        int16_t maxWet= 0;

        int16_t wetL = 0;
        int16_t wetR = 0;



        for (unsigned int g = 0; g < kMaxGrains; ++g)
        {
            Grain &grain = grains[g];

            // Randomize grain start position and size if the grain has finished
            if (grain.currentIndex >= grain.sizeSamples)
            {
                // When the grain has finished then we add it's old level back to
                // the headRoom
                headRoom += grain.level;

                if (headRoom > 4096)
                {
                    headRoom = 4096;
                }
                grain.startIndex = rand() % kBufSize;
                //grain.sizeSamples = static_cast<unsigned int>((0.01f + static_cast<float>(rand()) / RAND_MAX 0.49f) context->audioSampleRate);
                uint32_t maxSize = (kMaxGrainSize * yKnob) >> 12;
                if (maxSize < kMinGrainSize)
                {
                    maxSize = kMinGrainSize;
                }
                grain.sizeSamples = (rndi32() % (maxSize - kMinGrainSize)) + kMinGrainSize;
                grain.currentIndex = 0;
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
            }
            
            unsigned int grainReadIndex = (grain.startIndex + grain.currentIndex) % kBufSize;
            int16_t grainSample = audioBuf[grainReadIndex];

            // Apply level
            grainSample = static_cast<uint16_t>((grainSample * grain.level) >> 12);

            // If we're near the start of end of the grain then we fade in/out
            // using a Hann window lookup table
            // As the hann window is symmetrical I'm using half of one to save
            // on space
            
            int16_t hannIndex = grain.currentIndex; 
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
            
        }

        // audio buffer record debug
        // this "debug" signal sounds nice when mixed in with the granulated
        // playback so maybe worth mixing this into tehe output somehow?
        //wetL = audioBuf[readI];

        // Right shift dry signal volume by more in order to match the perceived
        // level of the wet signal
        int16_t mixOutL = (audioL * (mainKnob) >> 13) + (wetL * (4095 - (mainKnob)) >> 12);
        int16_t mixOutR = (audioR * (mainKnob) >> 13) + (wetR * (4095 - (mainKnob)) >> 12);


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
        switch  (recordState)
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
    writeI = 0;
}

void TriggaHappy::ReadKnobs(void)
{
    // Virtual detent the knob values
    mainKnob = virtualDetentedKnob(KnobVal(Knob::Main));
    xKnob = virtualDetentedKnob(KnobVal(Knob::X));
    yKnob = virtualDetentedKnob(KnobVal(Knob::Y));
}

void TriggaHappy::ReadInputs(void)
{
    cv1 = CVIn1();               // -2048 to 2047
    cv2 = CVIn2();               // -2048 to 2047
    audioL = AudioIn1(); // -2048 to 2047
    audioR = AudioIn2(); // -2048 to 2047
}