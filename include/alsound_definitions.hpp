/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_DEFINITIONS_HPP__
#define __ALSOUND_DEFINITIONS_HPP__

#include <cinttypes>

#define ALSYS_LIBRARY_ALURE 0
#define ALSYS_LIBRARY_FMOD 1

#ifndef ALSYS_LIBRARY_TYPE
#define ALSYS_LIBRARY_TYPE ALSYS_LIBRARY_ALURE
#endif

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#define ALSYS_STEAM_AUDIO_SUPPORT_ENABLED 0
#define ALSYS_INTERNAL_AUDIO_SAMPLE_RATE 44'100u
#define ALSYS_DECODER_FREQUENCY ALSYS_INTERNAL_AUDIO_SAMPLE_RATE
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#define ALSYS_STEAM_AUDIO_SUPPORT_ENABLED 0
#define ALSYS_INTERNAL_AUDIO_SAMPLE_RATE 48'000u
#define ALSYS_DECODER_FREQUENCY 44'100u
#endif

#ifdef ALSYS_STATIC
#define DLLALSYS
#elif ALSYS_DLL
#ifdef __linux__
#define DLLALSYS __attribute__((visibility("default")))
#else
#define DLLALSYS __declspec(dllexport)
#endif
#else
#ifdef __linux__
#define DLLALSYS
#else
#define DLLALSYS __declspec(dllimport)
#endif
#endif

#endif
