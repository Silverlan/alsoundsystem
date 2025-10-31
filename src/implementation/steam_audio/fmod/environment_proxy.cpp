//
// Copyright 2017 Valve Corporation. All rights reserved. Subject to the following license:
// https://valvesoftware.github.io/steam-audio/license.html
//

module;

module pragma.alsoundsystem;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "alsound_definitions.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include "steam_audio/fmod/environment_proxy.h"
#include "steam_audio/fmod/auto_load_library.h"
#include "steam_audio/alsound_steam_audio.hpp"

std::mutex                    SceneState::sMutex{};
std::shared_ptr<SceneState>   SceneState::sSceneState{ nullptr };
bool                          SceneState::sEnvironmentHasReset{ false };

SceneState::SceneState(const IPLSimulationSettings& simulationSettings, 
                       const IPLhandle environment,
                       const IPLConvolutionType convolutionType) :
    mSimulationSettings(simulationSettings),
    mEnvironment(environment),
    mConvolutionType(convolutionType),
    mEnvironmentalRenderer(nullptr),
    mUsingAcceleratedMixing(false)
{}

SceneState::~SceneState()
{
    if (mEnvironmentalRenderer)
        gApi.iplDestroyEnvironmentalRenderer(&mEnvironmentalRenderer);
}

IPLSimulationSettings SceneState::simulationSettings() const
{
    return mSimulationSettings;
}

IPLhandle SceneState::environment() const
{
    return mEnvironment;
}

IPLhandle SceneState::environmentalRenderer() const
{
    if (!mEnvironment)
        return nullptr;

    auto globalState = GlobalState::get();
    if (!globalState)
        return nullptr;

    if (!mEnvironmentalRenderer)
    {
        auto ambisonicsOrder = mSimulationSettings.ambisonicsOrder;
        auto numChannels = (ambisonicsOrder + 1) * (ambisonicsOrder + 1);

        IPLAudioFormat outputFormat{ IPL_CHANNELLAYOUTTYPE_AMBISONICS, IPL_CHANNELLAYOUT_CUSTOM, numChannels, nullptr,
            ambisonicsOrder, IPL_AMBISONICSORDERING_ACN, IPL_AMBISONICSNORMALIZATION_N3D,
            IPL_CHANNELORDER_DEINTERLEAVED };

        auto renderingSettings = globalState->renderingSettings();
        renderingSettings.convolutionType = mConvolutionType;

		ipl::check_result(gApi.iplCreateEnvironmentalRenderer(globalState->context(), mEnvironment, renderingSettings, outputFormat, nullptr,nullptr, const_cast<IPLhandle*>(&mEnvironmentalRenderer)));
		if(!mEnvironmentalRenderer)
			return nullptr;
    }

    return mEnvironmentalRenderer;
}

IPLConvolutionType SceneState::convolutionType() const
{
    return mConvolutionType;
}

bool SceneState::isUsingAcceleratedMixing() const
{
    return mUsingAcceleratedMixing;
}

void SceneState::setUsingAcceleratedMixing(const bool usingAcceleratedMixing)
{
    mUsingAcceleratedMixing = usingAcceleratedMixing;
}

void SceneState::setEnvironment(const IPLSimulationSettings& simulationSettings, 
                                const IPLhandle environment,
                                const IPLConvolutionType convolutionType)
{
    std::lock_guard<std::mutex> lock(sMutex);
    sSceneState = std::make_shared<SceneState>(simulationSettings, environment, convolutionType);
}

void SceneState::resetEnvironment()
{
    std::lock_guard<std::mutex> lock(sMutex);
    sSceneState = nullptr;
    sEnvironmentHasReset = true;
}

bool SceneState::hasEnvironmentReset()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sEnvironmentHasReset;
}

void SceneState::acknowledgeEnvironmentReset()
{
    std::lock_guard<std::mutex> lock(sMutex);
    sEnvironmentHasReset = false;
}

std::shared_ptr<SceneState> SceneState::get()
{
    std::lock_guard<std::mutex> lock(sMutex);
    return sSceneState;
}

void F_CALL iplFmodSetEnvironment(IPLSimulationSettings simulationSettings, 
                                  IPLhandle environment,
                                  IPLConvolutionType convolutionType)
{
    SceneState::setEnvironment(simulationSettings, environment, convolutionType);
}

void F_CALL iplFmodResetEnvironment()
{
    SceneState::resetEnvironment();
}
#endif
#endif
