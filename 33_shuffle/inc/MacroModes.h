#pragma once

struct MacroMode
{
    unsigned int sleepChance;
    unsigned int pitchChance;
    unsigned int repeatChance;
    unsigned int repeatCount;
};

static constexpr MacroMode kMacroModes[] =
{
    // sleepChance, pitchChance, repeatChance, repeatCount
    { 512, 0, 2048, 0 },      // Mode 0: steady, normal pitch, occasional repeats
    { 2048, 1024, 1024, 1 },  // Mode 1: more sleep, some pitch variation, more repeats
    { 3072, 2048, 512, 2 },   // Mode 2: high sleep, lots of pitch variation, frequent repeats
    { 4095, 3072, 256, 3 }    // Mode 3: maximum sleep, extreme pitch variation, constant repeats
};

static constexpr uint16_t kNumMacroModes = sizeof(kMacroModes) / sizeof(MacroMode);
