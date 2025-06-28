/*

Created by: Fabian MARTINEZ
I-Ching_RND.cpp is a random value generator for the Disting NT module.
It generates random values based on the I Ching hexagrams and their associated values.
There is a quantized output for the generated values, which can be used in various musical applications and quantized with a variety of scales.
It has a built-in integer sequence generator that can be used to generate sequences of random values.
It has a noise output that can be used to generate random values based on noise. The noise type can be selected from a variety of options.
Every 64 clocks, you will have seen every hexagram exactly once, in a random order.
No repeats until all have been used.
After all 64, the order is reshuffled and the process repeats.
============
MIT License
============

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*/





#include <math.h>
#include <new>
#include <distingnt/api.h>

#include <cstdio> // for snprintf

#ifndef kNT_shapeLine
#define kNT_shapeLine 1
#endif
// Fix for M_PI not being defined on some toolchains
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
// --- Parameter indices ---
enum {
    kParamClockIn,
    kParamIntSeqTrigIn,
    kParamCVOut,
    kParamQuantOut,
    kParamIntSeqOut,
    kParamNoiseOut,
    kParamClockThruOut,
    kParamClockDivOut,
    kParamScale,
    kParamRoot,
    kParamTranspose,
    kParamMaskRotate,
    kParamIntSeqSelect,
    kParamIntSeqMod,
    kParamIntSeqStart,
    kParamIntSeqLen,
    kParamIntSeqDir,
    kParamIntSeqStride,
    kParamNoiseType,
    kParamClockDiv,
    kNumParams
};

// --- Scale definitions ---
#define NUM_STANDARD_SCALES 16
#define NUM_EXOTIC_SCALES 117
#define NUM_SCALES (NUM_STANDARD_SCALES + NUM_EXOTIC_SCALES)
#define SCALE_MAX_LEN 20

static const char* intseq_dir_names[] = { "loop", "pendulum" };

// --- All scale names (standard + exotic) ---
static const char* all_scale_names[NUM_SCALES] = {
    // Standard scales
    "Major", "Minor", "Harmonic Minor", "Melodic Minor", "Mixolydian", "Dorian", "Lydian", "Phrygian",
    "Aeolian", "Locrian", "Maj Pent", "Min Pent", "Whole Tone", "Octatonic HW", "Octatonic WH", "Ionian",

    // Exotic scales (names must match the order and count of your exotic scales)
    "Blues Major", "Blues Minor", "Folk", "Japanese", "Gamelan", "Gypsy", "Arabian", "Flamenco",
    "Whole Tone (Exotic)", "Pythagorean", "1/4-EB", "1/4-E", "1/4-EA", "Bhairav", "Gunakri", "Marwa",
    "Shree", "Purvi", "Bilawal", "Yaman", "Kafi", "Bhimpalasree", "Darbari", "Rageshree",
    "Khamaj", "Mimal", "Parameshwari", "Rangeshwari", "Gangeshwari", "Kameshwari", "Pa_Kafi", "Natbhairav",
    "M_Kauns", "Bairagi", "B_Todi", "Chandradeep", "Kaushik_Todi", "Jogeshwari",
    "Tartini-Vallotti", "13/22-tET", "13/19-tET", "Magic145", "Quartaminorthirds", "Armodue",
    "Hirajoshi", "Scottish Bagpipes", "Thai Ranat",
    "Sevish 31-EDO", "11TET Machine", "13TET Father", "15TET Blackwood", "16TET Mavila", "16TET Mavila9", "17TET Superpyth",
    "22TET Orwell", "22TET Pajara", "22TET Pajara2", "22TET Porcupine", "26TET Flattone", "26TET Lemba", "46TET Sensi",
    "53TET Orwell", "72TET Prent", "Zeus Trivalent", "202TET Octone", "313TET Elfmadagasgar", "Marvel Glumma", "TOP Parapyth",
    "16ED", "15ED", "14ED", "13ED", "11ED", "10ED", "9ED", "8ED", "7ED", "6ED", "5ED",
    "16HD2", "15HD2", "14HD2", "13HD2", "12HD2", "11HD2", "10HD2", "9HD2", "8HD2", "7HD2", "6HD2", "5HD2",
    "32-16SD2", "30-15SD2", "28-14SD2", "26-13SD2", "24-12SD2", "22-11SD2", "20-10SD2", "18-9SD2", "16-8SD2", "14-7SD2", "12-6SD2", "10-5SD2", "8-4SD2",
    "BP Equal", "BP Just", "BP Lambda",
    "8-24HD3", "7-21HD3", "6-18HD3", "5-15HD3", "4-12HD3", "24-8HD3", "21-7HD3", "18-6HD3", "15-5HD3", "12-4HD3"
    // Add more names if you have more exotic scales, up to NUM_EXOTIC_SCALES
};


// --- Exotic scale intervals (static) ---
static const float exotic_scales[NUM_EXOTIC_SCALES][SCALE_MAX_LEN] = {


   // Blues major (From midipal/BitT source code)
    { 0.0f, 3.0f, 4.0f, 7.0f, 9.0f, 10.0f },
    // Blues minor (From midipal/BitT source code)
    { 0.0f, 3.0f, 5.0f, 6.0f, 7.0f, 10.0f },

    // Folk (From midipal/BitT source code)
    { 0.0f, 1.0f, 3.0f, 4.0f, 5.0f, 7.0f, 8.0f, 10.0f },
    // Japanese (From midipal/BitT source code)
    { 0.0f, 1.0f, 5.0f, 7.0f, 8.0f },
    // Gamelan (From midipal/BitT source code)
    { 0.0f, 1.0f, 3.0f, 7.0f, 8.0f },
    // Gypsy
    { 0.0f, 2.0f, 3.0f, 6.0f, 7.0f, 8.0f, 11.0f },
    // Arabian
    { 0.0f, 1.0f, 4.0f, 5.0f, 7.0f, 8.0f, 11.0f },
    // Flamenco
    { 0.0f, 1.0f, 4.0f, 5.0f, 7.0f, 8.0f, 10.0f },
    // Whole tone (From midipal/BitT source code)
    { 0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f },
    // pythagorean (From yarns source code)
    { 0.0f, 0.898f, 2.039f, 2.938f, 4.078f, 4.977f, 6.117f, 7.023f, 7.922f, 9.062f, 9.961f, 11.102f },
    // 1_4_eb (From yarns source code)
    { 0.0f, 1.0f, 2.0f, 3.0f, 3.5f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 10.5f },
    // 1_4_e (From yarns source code)
    { 0.0f, 1.0f, 2.0f, 3.0f, 3.5f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f },
    // 1_4_ea (From yarns source code)
    { 0.0f, 1.0f, 2.0f, 3.0f, 3.5f, 5.0f, 6.0f, 7.0f, 8.0f, 8.5f, 10.0f, 11.0f },
    // bhairav (From yarns source code)
    { 0.0f, 0.898f, 3.859f, 4.977f, 7.023f, 7.922f, 10.883f },
    // gunakri (From yarns source code)
    { 0.0f, 1.117f, 4.977f, 7.023f, 8.141f },
    // marwa (From yarns source code)
    { 0.0f, 1.117f, 3.859f, 5.898f, 8.844f, 10.883f },
    // shree (From yarns source code)
    { 0.0f, 0.898f, 3.859f, 5.898f, 7.023f, 7.922f, 10.883f },
    // purvi (From yarns source code)
    { 0.0f, 1.117f, 3.859f, 5.898f, 7.023f, 8.141f, 10.883f },
    // bilawal (From yarns source code)
    { 0.0f, 2.039f, 3.859f, 4.977f, 7.023f, 9.062f, 10.883f },
    // yaman (From yarns source code)
    { 0.0f, 2.039f, 4.078f, 6.117f, 7.023f, 9.062f, 11.102f },
    // kafi (From yarns source code)
    { 0.0f, 1.820f, 2.938f, 4.977f, 7.023f, 8.844f, 9.961f },
    // bhimpalasree (From yarns source code)
    { 0.0f, 2.039f, 3.156f, 4.977f, 7.023f, 9.062f, 10.180f },
    // darbari (From yarns source code)
    { 0.0f, 2.039f, 2.938f, 4.977f, 7.023f, 7.922f, 9.961f },
    // rageshree (From yarns source code)
    { 0.0f, 2.039f, 3.859f, 4.977f, 7.023f, 8.844f, 9.961f },
    // khamaj (From yarns source code)
    { 0.0f, 2.039f, 3.859f, 4.977f, 7.023f, 9.062f, 9.961f, 11.102f },
    // mimal (From yarns source code)
    { 0.0f, 2.039f, 2.938f, 4.977f, 7.023f, 8.844f, 9.961f, 10.883f },
    // parameshwari (From yarns source code)
    { 0.0f, 0.898f, 2.938f, 4.977f, 8.844f, 9.961f },
    // rangeshwari (From yarns source code)
    { 0.0f, 2.039f, 2.938f, 4.977f, 7.023f, 10.883f },
    // gangeshwari (From yarns source code)
    { 0.0f, 3.859f, 4.977f, 7.023f, 7.922f, 9.961f },
    // kameshwari (From yarns source code)
    { 0.0f, 2.039f, 5.898f, 7.023f, 8.844f, 9.961f },
    // pa__kafi (From yarns source code)
    { 0.0f, 2.039f, 2.938f, 4.977f, 7.023f, 9.062f, 9.961f },
    // natbhairav (From yarns source code)
    { 0.0f, 2.039f, 3.859f, 4.977f, 7.023f, 7.922f, 10.883f },
    // m_kauns (From yarns source code)
    { 0.0f, 2.039f, 4.078f, 4.977f, 7.922f, 9.961f },
    // bairagi (From yarns source code)
    { 0.0f, 0.898f, 4.977f, 7.023f, 9.961f },
    // b_todi (From yarns source code)
    { 0.0f, 0.898f, 2.938f, 7.023f, 9.961f },
    // chandradeep (From yarns source code)
    { 0.0f, 2.938f, 4.977f, 7.023f, 9.961f },
    // kaushik_todi (From yarns source code)
    { 0.0f, 2.938f, 4.977f, 5.898f, 7.922f },
    // jogeshwari (From yarns source code)
    { 0.0f, 2.938f, 3.859f, 4.977f, 8.844f, 9.961f },

    // Tartini-Vallotti [12]
    { 0.0f, 0.9375f, 1.9609f, 2.9766f, 3.9219f, 5.0234f, 5.9219f, 6.9766f, 7.9609f, 8.9375f, 10.0f, 10.8984f },
    // 13 out of 22-tET, generator = 5 [13]
    { 0.0f, 1.0938f, 2.1797f, 3.2734f, 3.8203f, 4.9063f, 6.0f, 6.5469f, 7.6328f, 8.7266f, 9.2734f, 10.3672f, 11.4531f },
    // 13 out of 19-tET, Mandelbaum [13]
    { 0.0f, 1.2656f, 1.8984f, 3.1563f, 3.7891f, 5.0547f, 5.6875f, 6.9453f, 7.5781f, 8.8438f, 9.4766f, 10.7344f, 11.3672f },
    // Magic[16] in 145-tET [16]
    { 0.0f, 1.4922f, 2.0703f, 2.6484f, 3.2266f, 3.8047f, 4.3828f, 5.8750f, 6.4531f, 7.0313f, 7.6172f, 8.1953f, 9.6797f, 10.2656f, 10.8438f, 11.4219f },
    // g=9 steps of 139-tET. Gene Ward Smith "Quartaminorthirds" 7-limit temperament [16]
    { 0.0f, 0.7734f, 1.5547f, 2.3281f, 3.1094f, 3.8828f, 4.6641f, 5.4375f, 6.2188f, 6.9922f, 7.7734f, 8.5469f, 9.3203f, 10.1016f, 10.8750f, 11.6563f },
    // Armodue semi-equalizzato [16]
    { 0.0f, 0.7734f, 1.5469f, 2.3203f, 3.0938f, 3.8672f, 4.6484f, 5.4219f, 6.1953f, 6.9688f, 7.7422f, 8.5156f, 9.2891f, 9.6797f, 10.4531f, 11.2266f },

    // Hirajoshi[5]
    { 0.0f, 1.8516f, 3.3672f, 6.8281f, 7.8984f },
    // Scottish bagpipes[7]
    { 0.0f, 1.9688f, 3.4063f, 4.9531f, 7.0313f, 8.5313f, 10.0938f },
    // Thai ranat[7]
    { 0.0f, 1.6094f, 3.4609f, 5.2578f, 6.8594f, 8.6172f, 10.2891f },

    // Sevish quasi-12-equal mode from 31-EDO
    { 0.0f, 1.1641f, 2.3203f, 3.0938f, 4.2578f, 5.0313f, 6.1953f, 7.3516f, 8.1328f, 9.2891f, 10.0625f, 11.2266f },
    // 11 TET Machine[6]
    { 0.0f, 2.1797f, 4.3672f, 5.4531f, 7.6328f, 9.8203f },
    // 13 TET Father[8]
    { 0.0f, 1.8438f, 3.6953f, 4.6172f, 6.4609f, 8.3047f, 9.2344f, 11.0781f },
    // 15 TET Blackwood[10]
    { 0.0f, 1.6016f, 2.3984f, 4.0f, 4.7969f, 6.3984f, 7.2031f, 8.7969f, 9.6016f, 11.2031f },
    // 16 TET Mavila[7]
    { 0.0f, 1.5f, 3.0f, 5.25f, 6.75f, 8.25f, 9.75f },
    // 16 TET Mavila[9]
    { 0.0f, 0.75f, 2.25f, 3.75f, 5.25f, 6.0f, 7.5f, 9.0f, 10.5f },
    // 17 TET Superpyth[12]
    { 0.0f, 0.7031f, 1.4141f, 2.8203f, 3.5313f, 4.9375f, 5.6484f, 6.3516f, 7.7578f, 8.4688f, 9.8828f, 10.5859f },

    // 22 TET Orwell[9]
    { 0.0f, 1.0938f, 2.7266f, 3.8203f, 5.4531f, 6.5469f, 8.1797f, 9.2734f, 10.9063f },
    // 22 TET Pajara[10] Static Symmetrical Maj
    { 0.0f, 1.0938f, 2.1797f, 3.8203f, 4.9063f, 6.0f, 7.0938f, 8.1797f, 9.8203f, 10.9063f },
    // 22 TET Pajara[10] Std Pentachordal Maj
    { 0.0f, 1.0938f, 2.1797f, 3.8203f, 4.9063f, 6.0f, 7.0938f, 8.7266f, 9.8203f, 10.9063f },
    // 22 TET Porcupine[7]
    { 0.0f, 1.6328f, 3.2734f, 4.9063f, 7.0938f, 8.7266f, 10.3672f },
    // 26 TET Flattone[12]
    { 0.0f, 0.4609f, 1.8438f, 2.3047f, 3.6953f, 5.0781f, 5.5391f, 6.9219f, 7.3828f, 8.7656f, 9.2266f, 10.6172f },
    // 26 TET Lemba[10]
    { 0.0f, 1.3828f, 2.3047f, 3.6953f, 4.6172f, 6.0f, 7.3828f, 8.3047f, 9.6875f, 10.6172f },
    // 46 TET Sensi[11]
    { 0.0f, 1.3047f, 2.6094f, 3.9141f, 4.4375f, 5.7422f, 7.0469f, 8.3516f, 8.8672f, 10.1719f, 11.4766f },
    // 53 TET Orwell[9]
    { 0.0f, 1.1328f, 2.7188f, 3.8516f, 5.4375f, 6.5625f, 8.1484f, 9.2813f, 10.8672f },
    // 12 out of 72-TET scale by Prent Rodgers
    { 0.0f, 2.0f, 2.6641f, 3.8359f, 4.3359f, 5.0f, 5.5f, 7.0f, 8.8359f, 9.6641f, 10.5f, 10.8359f },
    // Trivalent scale in zeus temperament[7]
    { 0.0f, 1.5781f, 3.8750f, 5.4531f, 7.0313f, 9.3359f, 10.9063f },
    // 202 TET tempering of octone[8]
    { 0.0f, 1.1875f, 3.5078f, 3.8594f, 6.1797f, 7.0078f, 9.3281f, 9.6797f },
    // 313 TET elfmadagasgar[9]
    { 0.0f, 2.0313f, 2.4922f, 4.5234f, 4.9844f, 7.0156f, 7.4766f, 9.5078f, 9.9688f },
    // Marvel woo version of glumma[12]
    { 0.0f, 0.4922f, 2.3281f, 3.1719f, 3.8359f, 5.4922f, 6.1641f, 7.0078f, 8.8359f, 9.3281f, 9.6797f, 11.6563f },
    // TOP Parapyth[12]
    { 0.0f, 0.5859f, 2.0703f, 2.6563f, 4.1406f, 4.7266f, 5.5469f, 7.0469f, 7.6172f, 9.1094f, 9.6875f, 11.1797f },

    // 16-ED (ED2 or ED3)
    { 0.0f, 0.75f, 1.5f, 2.25f, 3.0f, 3.75f, 4.5f, 5.25f, 6.0f, 6.75f, 7.5f, 8.25f, 9.0f, 9.75f, 10.5f, 11.25f },
    // 15-ED (ED2 or ED3)
    { 0.0f, 0.7969f, 1.6016f, 2.3984f, 3.2031f, 4.0f, 4.7969f, 5.6016f, 6.3984f, 7.2031f, 8.0f, 8.7969f, 9.6016f, 10.3984f, 11.2031f },
    // 14-ED (ED2 or ED3)
    { 0.0f, 0.8594f, 1.7109f, 2.5703f, 3.4297f, 4.2891f, 5.1484f, 6.0f, 6.8594f, 7.7188f, 8.5781f, 9.4375f, 10.2969f, 11.1563f },
    // 13-ED (ED2 or ED3)
    { 0.0f, 0.9219f, 1.8438f, 2.7656f, 3.6953f, 4.6328f, 5.6328f, 6.5703f, 7.4922f, 8.4141f, 9.3359f, 10.2578f, 11.1797f },
    // 11-ED (ED2 or ED3)
    { 0.0f, 1.0938f, 2.1797f, 3.2734f, 4.3672f, 5.4531f, 6.5469f, 7.6328f, 8.7266f, 9.8203f, 10.9063f },
    // 10-ED (ED2 or ED3)
    { 0.0f, 1.2031f, 2.3984f, 3.6016f, 4.7969f, 6.0f, 7.2031f, 8.3984f, 9.6016f, 10.7969f },
    // 9-ED (ED2 or ED3)
    { 0.0f, 1.3359f, 2.6641f, 4.0f, 5.3359f, 6.6641f, 8.0f, 9.3359f, 10.6641f },
    // 8-ED (ED2 or ED3)
    { 0.0f, 1.5f, 3.0f, 4.5f, 6.0f, 7.5f, 9.0f, 10.5f },
    // 7-ED (ED2 or ED3)
    { 0.0f, 1.7109f, 3.4297f, 5.1484f, 6.8594f, 8.5781f, 10.2969f },
    // 6-ED (ED2 or ED3)
    { 0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f },
    // 5-ED (ED2 or ED3)
    { 0.0f, 2.3984f, 4.7969f, 7.2031f, 9.6016f },

    // 16-HD2 (16 step harmonic series scale on the octave)
    { 0.0f, 1.0469f, 2.0391f, 2.9766f, 3.8594f, 4.7109f, 5.5156f, 6.2813f, 7.0234f, 7.7266f, 8.4063f, 9.0625f, 9.6875f, 10.2969f, 10.8906f, 11.4531f },
    // 15-HD2 (15 step harmonic series scale on the octave)
    { 0.0f, 1.1172f, 2.1641f, 3.1563f, 4.0938f, 4.9766f, 5.8203f, 6.6328f, 7.4141f, 8.1641f, 8.8828f, 9.5703f, 10.2266f, 10.852f, 11.4453f },
    // 14-HD2 (14 step harmonic series scale on the octave)
    { 0.0f, 1.1953f, 2.3125f, 3.3594f, 4.3516f, 5.2891f, 6.1797f, 7.0313f, 7.8516f, 8.6406f, 9.3984f, 10.125f, 10.8203f, 11.4844f },
    // 13-HD2 (13 step harmonic series scale on the octave)
    { 0.0f, 1.2813f, 2.4766f, 3.5938f, 4.6406f, 5.6328f, 6.5703f, 7.4609f, 8.3125f, 9.125f, 9.9063f, 10.6484f, 11.3594f },
    // 12-HD2 (12 step harmonic series scale on the octave)
    { 0.0f, 1.3828f, 2.6719f, 3.8594f, 5.0078f, 6.0313f, 6.9922f, 7.9531f, 8.8438f, 9.6875f, 10.4844f, 11.2656f },
    // 11-HD2 (11 step harmonic series scale on the octave)
    { 0.0f, 1.5078f, 2.8906f, 4.1719f, 5.3672f, 6.4844f, 7.5391f, 8.5234f, 9.4688f, 10.3672f, 11.2109f },
    // 10-HD2 (10 step harmonic series scale on the octave)
    { 0.0f, 1.6484f, 3.1563f, 4.5391f, 5.8672f, 7.0234f, 8.0703f, 9.1875f, 10.1797f, 11.1094f },
    // 9-HD2 (9 step harmonic series scale on the octave)
    { 0.0f, 1.8203f, 3.4766f, 5.0938f, 6.6797f, 8.2422f, 9.7891f, 11.3203f, 12.0f },
    // 8-HD2 (8 step harmonic series scale on the octave)
    { 0.0f, 2.0391f, 3.8594f, 5.5156f, 7.0234f, 8.4063f, 9.6875f, 10.8906f },
    // 7-HD2 (7 step harmonic series scale on the octave)
    { 0.0f, 2.3125f, 4.3516f, 6.1797f, 7.8516f, 9.3984f, 10.8203f },
    // 6-HD2 (6 step harmonic series scale on the octave)
    { 0.0f, 3.0313f, 6.0313f, 9.0625f, 12.0f, 15.0f },
    // 5-HD2 (5 step harmonic series scale on the octave)
    { 0.0f, 4.0f, 8.0f, 12.0f, 16.0f },

    // 32-16-SD2 (16 step subharmonic series scale on the octave)
    { 0.0f, 0.5469f, 1.1172f, 1.7031f, 2.3125f, 2.9375f, 3.5938f, 4.2734f, 4.9766f, 5.7188f, 6.4844f, 7.2891f, 8.0234f, 8.9297f, 9.9609f, 10.9531f },
    // 30-15-SD2 (15 step subharmonic series scale on the octave)
    { 0.0f, 0.5859f, 1.1953f, 1.8203f, 2.4766f, 3.1563f, 3.8594f, 4.6016f, 5.3672f, 6.1797f, 7.0313f, 7.9063f, 8.8438f, 9.8359f, 10.8828f },
    // 28-14-SD2 (14 step subharmonic series scale on the octave)
    { 0.0f, 0.6328f, 1.2813f, 1.9609f, 2.6719f, 3.4063f, 4.1719f, 4.977f, 5.8203f, 6.6953f, 7.6328f, 8.6328f, 9.6875f, 10.8047f, 12.0f },
    // 26-13-SD2 (13 step subharmonic series scale on the octave)
    { 0.0f, 0.6797f, 1.3828f, 2.125f, 2.8906f, 3.6953f, 4.5391f, 5.4219f, 6.3516f, 7.3203f, 8.3281f, 9.375f, 10.4609f },
    // 24-12-SD2 (12 step subharmonic series scale on the octave)
    { 0.0f, 0.7344f, 1.5078f, 2.3125f, 3.1563f, 4.0469f, 4.9766f, 5.9531f, 6.9688f, 8.0234f, 9.1172f, 10.25f },
    // 22-11-SD2 (11 step subharmonic series scale on the octave)
    { 0.0f, 0.8047f, 1.6484f, 2.5391f, 3.4766f, 4.4609f, 5.4922f, 6.5703f, 7.6953f, 8.8672f, 10.0859f },
    // 20-10-SD2 (10 step subharmonic series scale on the octave)
    { 0.0f, 0.8906f, 1.8203f, 2.8125f, 3.8594f, 4.9609f, 6.1172f, 7.3281f, 8.5938f, 9.9141f },
    // 18-9-SD2 (9 step subharmonic series scale on the octave)
    { 0.0f, 0.9922f, 2.0391f, 3.1563f, 4.3359f, 5.5781f, 6.8828f, 8.25f, 9.6797f },
    // 16-8-SD2 (8 step subharmonic series scale on the octave)
    { 0.0f, 1.1172f, 2.3125f, 3.5938f, 4.9609f, 6.4141f, 7.9531f, 9.5781f },
    // 14-7-SD2 (7 step subharmonic series scale on the octave)
    { 0.0f, 1.2813f, 2.6719f, 4.1719f, 5.7891f, 7.5234f, 9.375f },
    // 12-6-SD2 (6 step subharmonic series scale on the octave)
    { 0.0f, 1.5078f, 3.1563f, 4.9609f, 6.9219f, 9.0391f },
    // 10-5-SD2 (5 step subharmonic series scale on the octave)
    { 0.0f, 1.8203f, 3.8594f, 6.1719f, 8.8438f },
    // 8-4-SD2 (4 step subharmonic series scale on the octave)
    { 0.0f, 2.3125f, 4.9766f, 8.1406f },

    // Bohlen-Pierce (equal)
    { 0.0f, 0.9219f, 1.8438f, 2.7656f, 3.6953f, 4.6172f, 5.5391f, 6.4609f, 7.3828f, 8.3047f, 9.2344f, 10.1563f, 11.0781f },
    // Bohlen-Pierce (just)
    { 0.0f, 0.8438f, 1.9063f, 2.7422f, 3.6719f, 4.6484f, 5.5781f, 6.4219f, 7.3516f, 8.3281f, 9.2578f, 10.0938f, 11.1563f },
    // Bohlen-Pierce (lambda)
    { 0.0f, 1.9063f, 2.7422f, 3.6719f, 5.5781f, 6.4219f, 8.3281f, 9.2578f, 11.1563f },

    // 8-24-HD3 (16 step harmonic series scale on the tritave)
    { 0.0f, 1.2891f, 2.4375f, 3.4766f, 4.4297f, 5.3047f, 6.1172f, 6.8828f, 7.6172f, 8.3203f, 9.0f, 9.6641f, 10.3125f, 10.9453f, 11.5625f, 12.1563f },
    // 7-21-HD3 (14 step harmonic series scale on the tritave)
    { 0.0f, 1.4609f, 2.7422f, 3.8984f, 4.9375f, 5.8672f, 6.6953f, 7.4297f, 8.0781f, 8.6484f, 9.1484f, 9.5859f, 9.9688f, 10.3047f },
    // 6-18-HD3 (12 step harmonic series scale on the tritave)
    { 0.0f, 1.6875f, 3.1406f, 4.4297f, 5.5703f, 6.5703f, 7.4375f, 8.1797f, 8.8047f, 9.3203f, 9.7344f, 10.0547f },
    // 5-15-HD3 (10 step harmonic series scale on the tritave)
    { 0.0f, 1.9922f, 3.6719f, 5.1328f, 6.3828f, 7.4297f, 8.2813f, 8.9453f, 9.4297f, 9.7422f },
    // 4-12-HD3 (8 step harmonic series scale on the tritave)
    { 0.0f, 2.4375f, 4.4297f, 6.1172f, 7.6172f, 9.0f, 10.3125f, 11.5625f },

    // 24-8-HD3 (16 step subharmonic series scale on the tritave)
    { 0.0f, 0.4688f, 0.9531f, 1.4609f, 1.9922f, 2.5469f, 3.125f, 3.7266f, 4.3516f, 5.0f, 5.6719f, 6.3672f, 7.0859f, 7.8281f, 8.5938f, 9.3828f },
    // 21-7-HD3 (14 step subharmonic series scale on the tritave)
    { 0.0f, 0.5313f, 1.0938f, 1.6875f, 2.3047f, 2.9453f, 3.6094f, 4.2969f, 5.0078f, 5.7422f, 6.5f, 7.2813f, 8.0859f, 8.9141f },
    // 18-6-HD3 (12 step subharmonic series scale on the tritave)
    { 0.0f, 0.625f, 1.2891f, 1.9922f, 2.7344f, 3.5156f, 4.3359f, 5.1953f, 6.0938f, 7.0313f, 8.0078f, 9.0234f },
    // 15-5-HD3 (10 step subharmonic series scale on the tritave)
    { 0.0f, 0.75f, 1.5625f, 2.4375f, 3.375f, 4.375f, 5.4375f, 6.5625f, 7.75f, 9.0f },
    // 12-4-HD3 (8 step subharmonic series scale on the tritave)
    { 0.0f, 0.9531f, 1.9922f, 3.125f, 4.3516f, 5.6719f, 7.0859f, 8.5938f }
};


// --- Standard scale intervals (algorithmic) ---
void get_standard_scale_intervals(int scaleIdx, int* out, int* outLen) {
    switch (scaleIdx) {
        case 0: out[0]=0; out[1]=2; out[2]=4; out[3]=5; out[4]=7; out[5]=9; out[6]=11; out[7]=12; *outLen=8; break; // Major
        case 1: out[0]=0; out[1]=2; out[2]=3; out[3]=5; out[4]=7; out[5]=8; out[6]=10; out[7]=12; *outLen=8; break; // Minor
        case 2: out[0]=0; out[1]=2; out[2]=3; out[3]=5; out[4]=7; out[5]=8; out[6]=11; out[7]=12; *outLen=8; break; // Harmonic Minor
        case 3: out[0]=0; out[1]=2; out[2]=3; out[3]=5; out[4]=7; out[5]=9; out[6]=11; out[7]=12; *outLen=8; break; // Melodic Minor
        case 4: out[0]=0; out[1]=2; out[2]=4; out[3]=5; out[4]=7; out[5]=9; out[6]=10; out[7]=12; *outLen=8; break; // Mixolydian
        case 5: out[0]=0; out[1]=2; out[2]=3; out[3]=5; out[4]=7; out[5]=9; out[6]=10; out[7]=12; *outLen=8; break; // Dorian
        case 6: out[0]=0; out[1]=2; out[2]=4; out[3]=6; out[4]=7; out[5]=9; out[6]=11; out[7]=12; *outLen=8; break; // Lydian
        case 7: out[0]=0; out[1]=1; out[2]=3; out[3]=5; out[4]=7; out[5]=8; out[6]=10; out[7]=12; *outLen=8; break; // Phrygian
        case 8: out[0]=0; out[1]=2; out[2]=3; out[3]=5; out[4]=7; out[5]=8; out[6]=10; out[7]=12; *outLen=8; break; // Aeolian
        case 9: out[0]=0; out[1]=1; out[2]=3; out[3]=5; out[4]=6; out[5]=8; out[6]=10; out[7]=12; *outLen=8; break; // Locrian
        case 10: out[0]=0; out[1]=2; out[2]=4; out[3]=7; out[4]=9; out[5]=12; *outLen=5; break; // Maj Pent
        case 11: out[0]=0; out[1]=3; out[2]=5; out[3]=7; out[4]=10; out[5]=12; *outLen=5; break; // Min Pent
        case 12: out[0]=0; out[1]=2; out[2]=4; out[3]=6; out[4]=8; out[5]=10; out[6]=12; *outLen=7; break; // Whole Tone
        case 13: out[0]=0; out[1]=1; out[2]=3; out[3]=4; out[4]=6; out[5]=7; out[6]=9; out[7]=10; *outLen=8; break; // Octatonic HW
        case 14: out[0]=0; out[1]=2; out[2]=3; out[3]=5; out[4]=6; out[5]=8; out[6]=9; out[7]=11; *outLen=8; break; // Octatonic WH
        case 15: out[0]=0; out[1]=2; out[2]=4; out[3]=5; out[4]=7; out[5]=9; out[6]=11; out[7]=12; *outLen=8; break; // Ionian
        default: for (int i = 0; i < SCALE_MAX_LEN; ++i) out[i] = 0; *outLen = 0;
    }
}

// --- Quantize function ---
float quantize(float v, int scaleIdx, int root, int transpose, int maskRotate) {
    int scale[SCALE_MAX_LEN];
    int scaleLen = 0;
    if (scaleIdx < NUM_STANDARD_SCALES) {
        get_standard_scale_intervals(scaleIdx, scale, &scaleLen);
    } else if (scaleIdx < NUM_STANDARD_SCALES + NUM_EXOTIC_SCALES) {
        int exoticIdx = scaleIdx - NUM_STANDARD_SCALES;
        for (int i = 0; i < SCALE_MAX_LEN; ++i) scale[i] = (int)exotic_scales[exoticIdx][i];
        scaleLen = SCALE_MAX_LEN;
    } else {
        for (int i = 0; i < SCALE_MAX_LEN; ++i) scale[i] = 0;
        scaleLen = 0;
    }
    float note = v * 12.0f;
    int n = static_cast<int>(roundf(note));
    n += root + transpose;
    int scaleDegree = 0;
    int minDist = 128;
    for (int i = 0; i < scaleLen; ++i) {
        int deg = (scale[i] + maskRotate) % 12;
        int dist = abs((n % 12) - deg);
        if (dist < minDist) {
            minDist = dist;
            scaleDegree = i;
        }
    }
    int quantized = (n / 12) * 12 + scale[scaleDegree];
    return quantized / 12.0f;
}

// --- Integer Sequence definitions ---
#define NUM_INTSEQ 10
#define INTSEQ_MAX_LEN 128

// Integer sequences from Quantermain (O_C firmware)
static const int intseq_pi[INTSEQ_MAX_LEN] = {
    3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5,
    0,2,8,8,4,1,9,7,1,6,9,3,9,9,3,7,5,1,0,5,8,2,0,9,7,4,9,4,4,5,9,2,
    3,0,7,8,1,6,4,0,6,2,8,6,2,0,8,9,9,8,6,2,8,0,3,4,8,2,5,3,4,2,1,1,
    7,0,6,7,9,8,2,1,4,8,0,8,6,5,1,3,2,8,2,3,0,6,6,4,7,0,9,3,8,4,4,6
};
static const int intseq_vanEck[INTSEQ_MAX_LEN] = {
    0,0,1,0,2,0,2,2,1,6,5,5,7,6,7,9,8,8,10,9,11,10,12,11,13,12,14,13,15,14,16,15,
    17,16,18,17,19,18,20,19,21,20,22,21,23,22,24,23,25,24,26,25,27,26,28,27,29,28,30,29,31,30,32,31,
    33,32,34,33,35,34,36,35,37,36,38,37,39,38,40,39,41,40,42,41,43,42,44,43,45,44,46,45,47,46,48,47,
    49,48,50,49,51,50,52,51,53,52,54,53,55,54,56,55,57,56,58,57,59,58,60,59,61,60,62,61,63,62,64,63
};
static const int intseq_ssdn[INTSEQ_MAX_LEN] = {
    0,1,4,9,1,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,
    2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,
    10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,
    5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10,2,5,10
};
static const int intseq_dress[INTSEQ_MAX_LEN] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
    32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
    64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
    96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127
};
static const int intseq_pninf[INTSEQ_MAX_LEN] = {
    0,1,-1,2,0,1,-2,3,1,0,-1,2,-3,4,2,1,0,-1,3,-4,5,3,2,1,0,-1,4,-5,6,4,3,2,
    1,0,5,-6,7,5,4,3,2,1,6,-7,8,6,5,4,3,2,7,-8,9,7,6,5,4,3,8,-9,10,8,7,6,
    5,4,9,-10,11,9,8,7,6,5,10,-11,12,10,9,8,7,6,11,-12,13,11,10,9,8,7,12,-13,14,12,11,10,
    9,8,13,-14,15,13,12,11,10,9,14,-15,16,14,13,12,11,10,15,-16,17,15,14,13,12,11,16,-17,18,16,15,14
};
static const int intseq_dsum[INTSEQ_MAX_LEN] = {
    0,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,
    5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,
    1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,
    6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1
};
static const int intseq_dsum4[INTSEQ_MAX_LEN] = {
    0,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,
    2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,
    1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,
    3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1
};
static const int intseq_dsum5[INTSEQ_MAX_LEN] = {
    0,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,
    4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,
    4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,
    4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3
};
static const int intseq_cdn2[INTSEQ_MAX_LEN] = {
    0,-2,-4,-6,-8,-10,-12,-14,-16,-18,-20,-22,-24,-26,-28,-30,-32,-34,-36,-38,-40,-42,-44,-46,-48,-50,-52,-54,-56,-58,-60,-62,
    -64,-66,-68,-70,-72,-74,-76,-78,-80,-82,-84,-86,-88,-90,-92,-94,-96,-98,-100,-102,-104,-106,-108,-110,-112,-114,-116,-118,-120,-122,-124,-126,
    -128,-130,-132,-134,-136,-138,-140,-142,-144,-146,-148,-150,-152,-154,-156,-158,-160,-162,-164,-166,-168,-170,-172,-174,-176,-178,-180,-182,-184,-186,-188,-190,
    -192,-194,-196,-198,-200,-202,-204,-206,-208,-210,-212,-214,-216,-218,-220,-222,-224,-226,-228,-230,-232,-234,-236,-238,-240,-242,-244,-246,-248,-250,-252,-254
};
static const int intseq_frcti[INTSEQ_MAX_LEN] = {
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,7,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,8
};
static const int* intseq_tables[NUM_INTSEQ] = {
    intseq_pi, intseq_vanEck, intseq_ssdn, intseq_dress, intseq_pninf,
    intseq_dsum, intseq_dsum4, intseq_dsum5, intseq_cdn2, intseq_frcti
};

static const char* intseq_names[NUM_INTSEQ] = {
    "pi", "vnEck", "ssdn", "Dress", "PNinf", "Dsum", "Dsum4", "Dsum5", "CDn2", "Frcti"
};

// --- I Ching random hexagram generator ---
static uint32_t random = 0x12345678; // You may want to seed this differently

inline __attribute__((always_inline)) uint32_t advanceRandom(void)
{
    uint32_t x = random;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    random = x;
    return x;
}


// Generate a random bit (0 or 1)
inline int randomBit() {
    return advanceRandom() & 1;
}

// Generate a hexagram (6 coin tosses)
void generateHexagram(int hexagram[6]) {
    for (int i = 0; i < 6; ++i) {
        hexagram[i] = randomBit();
    }
}

// Convert hexagram to index (0-63)
int hexagramToIndex(const int hexagram[6]) {
    int idx = 0;
    for (int i = 0; i < 6; ++i) {
        idx |= (hexagram[i] << i);
    }
    return idx;
}

// --- State struct ---
struct IChingRndState {
    int lastClock = 0;
    int hexagram[6] = {0};
    int intseq_pos = 0;
    int lastIntSeqTrig = 0;
   

    int hexagramOrder[64];
    int hexagramStep = 0;
};
struct NoiseState {
    float pink[3] = {0.0f, 0.0f, 0.0f};  // Zustände für pinkNoise()
    float brown = 0.0f;                 // Zustand für brownNoise()
    float blueLast = 0.0f;              // Letzter Wert für blueNoise()
};

// --- Algorithm struct ---
struct _IChingRndAlgorithm : public _NT_algorithm {
    IChingRndState* state;
};

// --- Parameters array ---
static const _NT_parameter parameters[] = {
    { .name = "Clock In", .min = 1, .max = 28, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "IntSeqTrigIn", .min = 1, .max = 28, .def = 2, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "CV Out", .min = 1, .max = 28, .def = 13, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "Quant Out", .min = 1, .max = 28, .def = 14, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "IntSeq Out", .min = 1, .max = 28, .def = 15, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "Noise Out", .min = 1, .max = 28, .def = 16, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "Clock Thru Out", .min = 1, .max = 28, .def = 17, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "Clock Div Out", .min = 1, .max = 28, .def = 18, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "Scale", .min = 0, .max = NUM_SCALES-1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = all_scale_names },
    { .name = "Root", .min = 0, .max = 11, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "Transpose", .min = -24, .max = 24, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "MaskRot", .min = 0, .max = 15, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "IntSeq", .min = 0, .max = NUM_INTSEQ-1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = intseq_names },
    { .name = "IntSeqMod", .min = 1, .max = 32, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "IntSeqStart", .min = 0, .max = 126, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "IntSeqLen", .min = 1, .max = 128, .def = 16, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "IntSeqDir", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = intseq_dir_names },
    { .name = "IntSeqStride", .min = 1, .max = 16, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },   
    { .name = "Noise Type", .min = 0, .max = 3, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = (const char*[]){"White", "Pink", "Brown", "Blue"} },  
    { .name = "Clock Div", .min = 2, .max = 512, .def = 2, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
};

// static integer counters for clock mult/div




// noise functions
float whiteNoise();
float pinkNoise(float in, float* state);
float brownNoise(float in, float* state);
float blueNoise(float in, float* last);
NoiseState state;

// Noise functions
float whiteNoise() {
    return ((advanceRandom() >> 8) & 0xFFFF) / 32768.0f - 1.0f;
}

// Pink noise (simple filter, Paul Kellet Pink Noise Filter)
float pinkNoise(float in, float* state) {
    state[0] = 0.99886f * state[0] + 0.0555179f * in;
    state[1] = 0.99332f * state[1] + 0.0750759f * in;
    state[2] = 0.96900f * state[2] + 0.1538520f * in;
    return 0.5362f * (state[0] + state[1] + state[2]) + 0.1f * in;
}

// Brown noise (integrator)
float brownNoise(float in, float* state) {
    // Constants for integration and damping
    const float integration = 0.05f;
    const float damping = 0.0005f; // Avoid DC-Drift

    state[0] += integration * in;
    state[0] -= damping * state[0]; // soft DC-Offset-elimination

    // Optional: Soft Clipping (similar to tanh)
    float out = state[0] ; 
    //if (out > 1.0f) out = 1.0f;
    //else if (out < -1.0f) out = -1.0f;

    return out;
}
// Blue noise (differentiator)
float blueNoise(float in, float* state) {
    float sr = NT_globals.sampleRate;
    float fc = 100.0f; // Highpass at 100 Hz
    float alpha = sr / (sr + 2.0f * M_PI * fc);
    float out = alpha * (*state + in - *state);
    *state = in;
    return out;
}





// Shuffle hexagrams
void shuffleHexagrams(IChingRndState* state) {
    for (int i = 0; i < 64; ++i) state->hexagramOrder[i] = i;
    // Fisher-Yates shuffle
    for (int i = 63; i > 0; --i) {
        int j = advanceRandom() % (i + 1);
        int tmp = state->hexagramOrder[i];
        state->hexagramOrder[i] = state->hexagramOrder[j];
        state->hexagramOrder[j] = tmp;
    }
    state->hexagramStep = 0;
}

// --- Step function ---
void step(_NT_algorithm* self, float* busFrames, int numFramesBy4)
{
    _IChingRndAlgorithm* alg = (_IChingRndAlgorithm*)self;
    IChingRndState* state = alg->state;
    auto* ns = reinterpret_cast<NoiseState*>(NT_globals.workBuffer); // <<< NoiseState in WorkBuffer

    int numFrames = numFramesBy4 * 4;

    float* clockIn = busFrames + (alg->v[kParamClockIn] - 1) * numFrames;
    float* clockThruOut = busFrames + (alg->v[kParamClockThruOut] - 1) * numFrames;
    float* clockDivOut  = busFrames + (alg->v[kParamClockDivOut]  - 1) * numFrames;
    float* cvOut = busFrames + (alg->v[kParamCVOut] - 1) * numFrames;
    float* quantOut = busFrames + (alg->v[kParamQuantOut] - 1) * numFrames;
    float* intseqOut = busFrames + (alg->v[kParamIntSeqOut] - 1) * numFrames;
    float* intseqTrigIn = busFrames + (alg->v[kParamIntSeqTrigIn] - 1) * numFrames;
    float* noiseOut = busFrames + (alg->v[kParamNoiseOut] - 1) * numFrames;

    int clockDiv  = alg->v[kParamClockDiv];
    int noiseType = alg->v[kParamNoiseType];
    int scale = alg->v[kParamScale];
    int root = alg->v[kParamRoot];
    int transpose = alg->v[kParamTranspose];
    int maskRotate = alg->v[kParamMaskRotate];
    int intseqSel = alg->v[kParamIntSeqSelect];
    int intseqMod = alg->v[kParamIntSeqMod];
    int intseqStart = alg->v[kParamIntSeqStart];
    int intseqLen = alg->v[kParamIntSeqLen];
    int intseqDir = alg->v[kParamIntSeqDir];
    int intseqStride = alg->v[kParamIntSeqStride];

    static int div_counter = 0;
    static int div_state = 0;

    for (int i = 0; i < numFrames; ++i) {
        int clock = clockIn[i] > 1.0f ? 1 : 0;
        clockThruOut[i] = clock ? 5.0f : 0.0f;

        // Clock Divider
        if (clock && !state->lastClock) {
            div_counter++;
            if (div_counter >= clockDiv) {
                div_state = 1;
                div_counter = 0;
            } else {
                div_state = 0;
            }
        }
        clockDivOut[i] = div_state ? 5.0f : 0.0f;

        // Rising Edge: Hexagram Update
        if (clock && !state->lastClock) {
            int idx = state->hexagramOrder[state->hexagramStep];
            for (int b = 0; b < 6; ++b)
                state->hexagram[b] = (idx >> b) & 1;
            state->hexagramStep++;
            if (state->hexagramStep >= 64)
                shuffleHexagrams(state);
        }
        state->lastClock = clock;

        int idx = hexagramToIndex(state->hexagram);
        float semitones = (idx < 60) ? float((idx % 12) * 5) : 0.0f;
        float v_per_oct = semitones / 12.0f;
        cvOut[i] = v_per_oct;
        quantOut[i] = quantize(idx / 12.0f, scale, root, transpose, maskRotate);

        // Integer Sequence
        int offset = intseqStart + (state->intseq_pos * intseqStride);
        if (intseqDir == 1) {
            int cycle = intseqLen * 2 - 2;
            int posInCycle = state->intseq_pos % cycle;
            if (posInCycle >= intseqLen)
                offset = intseqStart + ((cycle - posInCycle) * intseqStride);
            else
                offset = intseqStart + (posInCycle * intseqStride);
        } else {
            offset = intseqStart + ((state->intseq_pos * intseqStride) % intseqLen);
        }

        int value = intseq_tables[intseqSel][offset % INTSEQ_MAX_LEN];
        if (intseqMod > 1) value %= intseqMod;
        int degree = value % 12;
        if (degree < 0) degree += 12;
        intseqOut[i] = quantize(degree / 12.0f, scale, root, transpose, maskRotate);

        int intseqTrig = intseqTrigIn[i] > 1.0f ? 1 : 0;
        if (intseqTrig && !state->lastIntSeqTrig)
            state->intseq_pos = (state->intseq_pos + 1) % intseqLen;
        state->lastIntSeqTrig = intseqTrig;

        // Noise Generation
        float wn = whiteNoise();
        float n = 0.0f;
        switch (noiseType) {
            case 0: n = wn; break;
            case 1: n = pinkNoise(wn, ns->pink); break;
            case 2: n = brownNoise(wn, &ns->brown); break;
            case 3: n = blueNoise(wn, &ns->blueLast); break;
            default: n = wn; break;
        }
        noiseOut[i] = n * 5.0f;
    }
}


// --- Requirements calculation ---
void calculateRequirements(_NT_algorithmRequirements& req, const int32_t*) {
    req.numParameters = kNumParams;
    req.sram = sizeof(_IChingRndAlgorithm);
    req.dram = sizeof(IChingRndState);
    req.dtc = 0;
    req.itc = 0;
}

// --- Construction ---

static _NT_algorithm* construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications)
{
    // SRAM 
    auto* alg = new(ptrs.sram) _IChingRndAlgorithm;

    // DRAM 
    alg->state = new(ptrs.dram) IChingRndState;

    //  WorkBuffer 
    if (NT_globals.workBufferSizeBytes < sizeof(NoiseState))
        return NULL;

    // NoiseState init (WorkBuffer)
    auto* ns = reinterpret_cast<NoiseState*>(NT_globals.workBuffer);
    *ns = NoiseState{};  // setzt alles auf 0.0f

    // Algorithm initialisation
    alg->parameters = parameters;
    alg->parameterPages = NULL; 
    alg->v = NULL; 

    return alg;
}





// --- Draw function ---

bool draw(_NT_algorithm* self) {
    _IChingRndAlgorithm* alg = (_IChingRndAlgorithm*)self;
    IChingRndState* state = alg->state;

    NT_drawText(0, 0, "I Ching Hexagram", 15);

    // Draw the hexagram ...
    int yStart = 2;
    int yStep = 4;
    int xStart = 2;
    int lineLen = 19;
    for (int i = 0; i < 6; ++i) {
        int lineIdx = 5 - i;
        int line = state->hexagram[lineIdx];
        int y = yStart + i * yStep;
        if (line) {
            NT_drawShapeI((_NT_shape)kNT_shapeLine, xStart, y, xStart + lineLen, y, 15);
            NT_drawShapeI((_NT_shape)kNT_shapeLine, xStart, y+1, xStart + lineLen, y+1, 15);
        } else {
            NT_drawShapeI((_NT_shape)kNT_shapeLine, xStart, y, xStart + 6, y, 15);
            NT_drawShapeI((_NT_shape)kNT_shapeLine, xStart + 13, y, xStart + 19, y, 15);
            NT_drawShapeI((_NT_shape)kNT_shapeLine, xStart, y+1, xStart + 6, y+1, 15);
            NT_drawShapeI((_NT_shape)kNT_shapeLine, xStart + 13, y+1, xStart + 19, y+1, 15);
        }
    }

    return true;
}

// --- Factory definition ---
static const _NT_factory factory = {
    .guid = NT_MULTICHAR('F','M','I','X'),
    .name = "IChing Random",
   
    .description = "64 hexagram step sequencer (random, clocked)",
    .numSpecifications = 0,
    .calculateRequirements = calculateRequirements,
    .construct = construct,
    .parameterChanged = NULL,
    .step = step,
    .draw = draw,
    .midiMessage = NULL,
};

// --- Plugin entry point ---
uintptr_t pluginEntry(_NT_selector selector, uint32_t data) {
    switch (selector) {
        case kNT_selector_version: return kNT_apiVersionCurrent;
        case kNT_selector_numFactories: return 1;
        case kNT_selector_factoryInfo: return (uintptr_t)((data == 0) ? &factory : NULL);
    }
    return 0;
}


