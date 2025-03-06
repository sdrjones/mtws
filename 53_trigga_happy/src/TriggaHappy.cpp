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

        int16_t audioM = (audioL + audioR) >> 1;

        audioM = notchFilter.ProcessSample(audioM);

        int16_t maxWet= 0;

        int16_t wetL = 0;
        int16_t wetR = 0;

        int16_t maxLev = 4096;

        for (unsigned int g = 0; g < kMaxGrains; ++g)
        {
            Grain &grain = grains[g];

            // Randomize grain start position and size if the grain has finished
            if (grain.currentIndex >= grain.sizeSamples)
            {
                grain.startIndex = rand() % kBufSize;
                //grain.sizeSamples = static_cast<unsigned int>((0.01f + static_cast<float>(rand()) / RAND_MAX 0.49f) context->audioSampleRate);
                grain.sizeSamples = (rand() % (kMaxGrainSize - kMinGrainSize)) + kMinGrainSize;
                grain.currentIndex = 0;
                grain.pan = rnd8();
                uint32_t fullLevel = rnd12();


                //grain.level = (fullLevel >> 1) + 2048;
                grain.level = fullLevel >> 1;
                if (grain.level > maxLev)
                {
                    grain.level = maxLev;
                    maxLev = 0;
                }
                else
                {
                    maxLev -= grain.level;
                }
                //grain.level = 4095;
            }
            
            // Linear interpolation for reading from the delay buffer
            unsigned int readIndex1 = (grain.startIndex + grain.currentIndex) % kBufSize;
            int16_t grainSample = audioBuf[readIndex1];

            // Apply level
            grainSample = static_cast<uint16_t>((grainSample * grain.level) >> 12);

            // Apply Hann window to the grain using the wavetable
            //int16_t windowValue = kHannWindow[static_cast<unsigned int>(static_cast<float>(grain.currentIndex) / grain.sizeSamples * (kMaxGrainSize - 1))];
            //grainSample *= windowValue;

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
            

            wetL += static_cast<int16_t>(grainSample * kLeftGains[grain.pan] >> 12);
            wetR += static_cast<int16_t>(grainSample * kRightGains[grain.pan] >> 12);
 

            grain.currentIndex++;
        }

        
        int16_t mixOutL = (audioL * (mainKnob) >> 12) + (wetL * (4095 - (mainKnob)) >> 12);
        int16_t mixOutR = (audioR * (mainKnob) >> 12) + (wetR * (4095 - (mainKnob)) >> 12);



        if (s != Switch::Up)
        {
            audioBuf[writeI] = audioM;
        }
        
        writeI = (writeI + 1) % kBufSize;

        AudioOut1(mixOutL);
        AudioOut2(mixOutR);
    
    }
    //     // do stuff
    //     ReadKnobs();

    //     // Read Switch
    //     Switch s = SwitchVal();
        


    //     ReadInputs();
    //     audioL = notchFilter.ProcessSample(audioL);
    //     AudioOut1(audioL);
    //     int16_t wet = 0;

    //     for (unsigned int g = 0; g < kMaxGrains; ++g)
    //     {
    //         Grain &grain = grains[g];

    //         // Randomize grain start position and size if the grain has finished
    //         if (grain.currentIndex >= grain.sizeSamples)
    //         {
    //             grain.startIndex = rand() % kBufSize;
    //             //grain.sizeSamples = static_cast<unsigned int>((0.01f + static_cast<float>(rand()) / RAND_MAX 0.49f) context->audioSampleRate);
    //             grain.sizeSamples = kMaxGrainSize;
    //             grain.currentIndex = 0;
    //         }
            
    //         // Linear interpolation for reading from the delay buffer
    //         unsigned int readIndex1 = (grain.startIndex + grain.currentIndex) % kBufSize;
    //         int16_t grainSample = audioBuf[readIndex1];

    //         // Apply Hann window to the grain using the wavetable
    //         //int16_t windowValue = kHannWindow[static_cast<unsigned int>(static_cast<float>(grain.currentIndex) / grain.sizeSamples * (kMaxGrainSize - 1))];
    //         //grainSample *= windowValue;

    //         wet += grainSample;

    //         grain.currentIndex++;


    //         audioBuf[writeI] = audioL;
    //         int16_t outSample = wet + audioL;
    //         AudioOut(0, audioL);
    //         AudioOut(1, outSample);
    //         writeI = (writeI + 1) % kBufSize;
    //     }
    // }
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
    x = virtualDetentedKnob(KnobVal(Knob::X));
    y = virtualDetentedKnob(KnobVal(Knob::Y));
}

void TriggaHappy::ReadInputs(void)
{
    cv1 = CVIn1();               // -2048 to 2047
    cv2 = CVIn2();               // -2048 to 2047
    audioL = AudioIn1(); // -2048 to 2047
    audioR = AudioIn2(); // -2048 to 2047
}