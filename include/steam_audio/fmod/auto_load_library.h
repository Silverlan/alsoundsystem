//
// Copyright 2017 Valve Corporation. All rights reserved. Subject to the following license:
// https://valvesoftware.github.io/steam-audio/license.html
//

#pragma once

#include "alsound_definitions.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <phonon.h>

/** Function pointer types for all the Steam Audio API functions used by the Unity plugin.
 */
typedef IPLerror (*IPLCreateContext)(IPLLogFunction logCallback,
                                     IPLAllocateFunction allocateCallback,
                                     IPLFreeFunction freeCallback,
                                     IPLhandle* context);

typedef IPLvoid (*IPLDestroyContext)(IPLhandle* context);

typedef IPLVector3 (*IPLCalculateRelativeDirection)(IPLVector3 sourcePosition,
                                                    IPLVector3 listenerPosition,
                                                    IPLVector3 listenerAhead,
                                                    IPLVector3 listenerUp);

typedef IPLerror(*IPLCreateBinauralRenderer)(IPLhandle context, 
                                             IPLRenderingSettings renderingSettings,
                                             IPLHrtfParams params, 
                                             IPLhandle* renderer);

typedef IPLvoid(*IPLDestroyBinauralRenderer)(IPLhandle* renderer);

typedef IPLerror(*IPLCreatePanningEffect)(IPLhandle renderer, 
                                          IPLAudioFormat inputFormat, 
                                          IPLAudioFormat outputFormat,
                                          IPLhandle* effect);

typedef IPLvoid(*IPLDestroyPanningEffect)(IPLhandle* effect);

typedef IPLvoid(*IPLApplyPanningEffect)(IPLhandle effect, 
                                        IPLAudioBuffer inputAudio, 
                                        IPLVector3 direction,
                                        IPLAudioBuffer outputAudio);

typedef IPLerror(*IPLCreateBinauralEffect)(IPLhandle renderer, 
                                           IPLAudioFormat inputFormat,
                                           IPLAudioFormat outputFormat, 
                                           IPLhandle* effect);

typedef IPLvoid(*IPLDestroyBinauralEffect)(IPLhandle* effect);

typedef IPLvoid(*IPLApplyBinauralEffect)(IPLhandle effect, 
                                         IPLAudioBuffer inputAudio, 
                                         IPLVector3 direction,
                                         IPLHrtfInterpolation interpolation, 
                                         IPLAudioBuffer outputAudio);

typedef IPLerror(*IPLCreateAmbisonicsPanningEffect)(IPLhandle renderer, 
                                                    IPLAudioFormat inputFormat,
                                                    IPLAudioFormat outputFormat, 
                                                    IPLhandle* effect);

typedef IPLvoid(*IPLDestroyAmbisonicsPanningEffect)(IPLhandle* effect);

typedef IPLvoid(*IPLApplyAmbisonicsPanningEffect)(IPLhandle effect, 
                                                  IPLAudioBuffer inputAudio,
                                                  IPLAudioBuffer outputAudio);

typedef IPLvoid(*IPLFlushAmbisonicsPanningEffect)(IPLhandle effect);

typedef IPLerror(*IPLCreateAmbisonicsBinauralEffect)(IPLhandle renderer, 
                                                     IPLAudioFormat inputFormat,
                                                     IPLAudioFormat outputFormat, 
                                                     IPLhandle* effect);

typedef IPLvoid(*IPLDestroyAmbisonicsBinauralEffect)(IPLhandle* effect);

typedef IPLvoid(*IPLApplyAmbisonicsBinauralEffect)(IPLhandle effect, 
                                                   IPLAudioBuffer inputAudio,
                                                   IPLAudioBuffer outputAudio);

typedef IPLvoid(*IPLFlushAmbisonicsBinauralEffect)(IPLhandle effect);

typedef IPLerror(*IPLCreateEnvironmentalRenderer)(IPLhandle context, 
                                                  IPLhandle environment,
                                                  IPLRenderingSettings renderingSettings, 
                                                  IPLAudioFormat outputFormat,
                                                  IPLSimulationThreadCreateCallback threadCreateCallback,
                                                  IPLSimulationThreadDestroyCallback threadDestroyCallback, 
                                                  IPLhandle* renderer);

typedef IPLvoid(*IPLDestroyEnvironmentalRenderer)(IPLhandle* renderer);

typedef IPLDirectSoundPath(*IPLGetDirectSoundPath)(IPLhandle environment, IPLVector3 listenerPosition,
        IPLVector3 listenerAhead, IPLVector3 listenerUp, IPLSource source, IPLfloat32 sourceRadius,
        IPLDirectOcclusionMode occlusionMode, IPLDirectOcclusionMethod occlusionMethod);

typedef IPLerror(*IPLCreateDirectSoundEffect)(IPLhandle renderer, 
                                              IPLAudioFormat inputFormat,
                                              IPLAudioFormat outputFormat, 
                                              IPLhandle* effect);

typedef IPLvoid(*IPLDestroyDirectSoundEffect)(IPLhandle* effect);

typedef IPLvoid(*IPLApplyDirectSoundEffect)(IPLhandle effect, 
                                            IPLAudioBuffer inputAudio,
                                            IPLDirectSoundPath directSoundPath, 
                                            IPLDirectSoundEffectOptions options, 
                                            IPLAudioBuffer outputAudio);

typedef IPLerror(*IPLCreateConvolutionEffect)(IPLhandle renderer, 
                                              IPLBakedDataIdentifier identifier, 
                                              IPLSimulationType simulationType,
                                              IPLAudioFormat inputFormat, 
                                              IPLAudioFormat outputFormat, 
                                              IPLhandle* effect);

typedef IPLvoid(*IPLDestroyConvolutionEffect)(IPLhandle* effect);

typedef IPLvoid(*IPLSetConvolutionEffectIdentifier)(IPLhandle effect, 
                                                    IPLBakedDataIdentifier identifier);

typedef IPLvoid(*IPLSetDryAudioForConvolutionEffect)(IPLhandle effect, IPLSource source,
        IPLAudioBuffer dryAudio);

typedef IPLvoid(*IPLGetWetAudioForConvolutionEffect)(IPLhandle effect, 
                                                     IPLVector3 listenerPosition,
                                                     IPLVector3 listenerAhead, 
                                                     IPLVector3 listenerUp, 
                                                     IPLAudioBuffer wetAudio);

typedef IPLvoid(*IPLGetMixedEnvironmentalAudio)(IPLhandle renderer, 
                                                IPLVector3 listenerPosition,
                                                IPLVector3 listenerAhead, 
                                                IPLVector3 listenerUp, 
                                                IPLAudioBuffer mixedWetAudio);

typedef IPLvoid(*IPLFlushConvolutionEffect)(IPLhandle effect);

/** An interface object that contains function pointers to the Steam Audio API.
 */
struct SteamAudioApi
{
    IPLCreateContext                        iplCreateContext;
    IPLDestroyContext                       iplDestroyContext;
    IPLCalculateRelativeDirection           iplCalculateRelativeDirection;
    IPLCreateBinauralRenderer               iplCreateBinauralRenderer;
    IPLDestroyBinauralRenderer              iplDestroyBinauralRenderer;
    IPLCreatePanningEffect                  iplCreatePanningEffect;
    IPLDestroyPanningEffect                 iplDestroyPanningEffect;
    IPLApplyPanningEffect                   iplApplyPanningEffect;
    IPLCreateBinauralEffect                 iplCreateBinauralEffect;
    IPLDestroyBinauralEffect                iplDestroyBinauralEffect;
    IPLApplyBinauralEffect                  iplApplyBinauralEffect;
    IPLCreateAmbisonicsPanningEffect        iplCreateAmbisonicsPanningEffect;
    IPLDestroyAmbisonicsPanningEffect       iplDestroyAmbisonicsPanningEffect;
    IPLApplyAmbisonicsPanningEffect         iplApplyAmbisonicsPanningEffect;
    IPLFlushAmbisonicsPanningEffect         iplFlushAmbisonicsPanningEffect;
    IPLCreateAmbisonicsBinauralEffect       iplCreateAmbisonicsBinauralEffect;
    IPLDestroyAmbisonicsBinauralEffect      iplDestroyAmbisonicsBinauralEffect;
    IPLApplyAmbisonicsBinauralEffect        iplApplyAmbisonicsBinauralEffect;
    IPLFlushAmbisonicsBinauralEffect        iplFlushAmbisonicsBinauralEffect;
    IPLCreateEnvironmentalRenderer          iplCreateEnvironmentalRenderer;
    IPLDestroyEnvironmentalRenderer         iplDestroyEnvironmentalRenderer;
    IPLGetDirectSoundPath                   iplGetDirectSoundPath;
    IPLCreateDirectSoundEffect              iplCreateDirectSoundEffect;
    IPLDestroyDirectSoundEffect             iplDestroyDirectSoundEffect;
    IPLApplyDirectSoundEffect               iplApplyDirectSoundEffect;
    IPLCreateConvolutionEffect              iplCreateConvolutionEffect;
    IPLDestroyConvolutionEffect             iplDestroyConvolutionEffect;
    IPLSetConvolutionEffectIdentifier       iplSetConvolutionEffectIdentifier;
    IPLSetDryAudioForConvolutionEffect      iplSetDryAudioForConvolutionEffect;
    IPLGetWetAudioForConvolutionEffect      iplGetWetAudioForConvolutionEffect;
    IPLGetMixedEnvironmentalAudio           iplGetMixedEnvironmentalAudio;
    IPLFlushConvolutionEffect               iplFlushConvolutionEffect;
};

extern SteamAudioApi gApi;
#endif
