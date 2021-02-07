//
// Copyright 2017 Valve Corporation. All rights reserved. Subject to the following license:
// https://valvesoftware.github.io/steam-audio/license.html
//

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "alsound_definitions.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include "steam_audio/fmod/audio_engine_settings.h"
#include "steam_audio/fmod/auto_load_library.h"
#include "steam_audio/alsound_steam_audio.hpp"

std::mutex                      GlobalState::sMutex{};
std::shared_ptr<GlobalState>    GlobalState::sAudioEngineSettings{ nullptr };

GlobalState::GlobalState(const IPLhandle &context,const IPLRenderingSettings& renderingSettings, 
                         const IPLAudioFormat& outputFormat)
{
    mRenderingSettings = renderingSettings;
    mOutputFormat = outputFormat;

	mContext = context;
    //if (gApi.iplCreateContext(nullptr, nullptr, nullptr, &mContext) != IPL_STATUS_SUCCESS)
    //    throw std::exception();

    IPLHrtfParams hrtfParams{ IPL_HRTFDATABASETYPE_DEFAULT, nullptr, "" };

	ipl::check_result(gApi.iplCreateBinauralRenderer(mContext, renderingSettings, hrtfParams, &mBinauralRenderer));
}

GlobalState::~GlobalState()
{
    if (mBinauralRenderer)
        gApi.iplDestroyBinauralRenderer(&mBinauralRenderer);

    if (mContext)
        gApi.iplDestroyContext(&mContext);
}

IPLRenderingSettings GlobalState::renderingSettings() const
{
    return mRenderingSettings;
}

IPLAudioFormat GlobalState::outputFormat() const
{
    return mOutputFormat;
}

IPLhandle GlobalState::context() const
{
    return mContext;
}

IPLhandle GlobalState::binauralRenderer() const
{
    return mBinauralRenderer;
}

std::shared_ptr<GlobalState> GlobalState::get()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sAudioEngineSettings;
}

void GlobalState::create(const IPLhandle &context,const IPLRenderingSettings& renderingSettings, 
                         const IPLAudioFormat& outputFormat)
{
    std::lock_guard<std::mutex> lock(sMutex);
    sAudioEngineSettings = std::make_shared<GlobalState>(context,renderingSettings, outputFormat);
}

void GlobalState::destroy()
{
    std::lock_guard<std::mutex> lock(sMutex);
    sAudioEngineSettings = nullptr;
}

void F_CALL iplFmodResetAudioEngine()
{
    GlobalState::destroy();
}
#endif
#endif
