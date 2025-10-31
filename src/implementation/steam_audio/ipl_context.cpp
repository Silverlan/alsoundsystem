// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.alsoundsystem;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include "steam_audio/fmod/audio_engine_settings.h"

static ipl::Context *s_context = nullptr;
void ipl::check_result(IPLerror err)
{
	if(err == IPL_STATUS_SUCCESS)
		return;
	if(s_context != nullptr && s_context->m_errorHandler != nullptr)
		s_context->m_errorHandler(err);
}

std::shared_ptr<ipl::Context> ipl::Context::Create(uint32_t frameSize)
{
	IPLhandle context;
	check_result(iplCreateContext(
	  [](char *message) {
		  if(s_context != nullptr && s_context->m_logHandler != nullptr)
			  s_context->m_logHandler(message);
	  },
	  nullptr, nullptr, &context));
	std::shared_ptr<void> ptrContext(context, [](IPLhandle context) {}); // Note: ipl context will be destroyed by GlobalState::destroy()
	return std::shared_ptr<Context>(new Context(ptrContext, frameSize));
}

ipl::Context::Context(const std::shared_ptr<void> &context, uint32_t frameSize) : std::enable_shared_from_this<Context>(), m_iplContext(context)
{
	IPLRenderingSettings renderSettings = {};
	renderSettings.samplingRate = ALSYS_INTERNAL_AUDIO_SAMPLE_RATE;
	renderSettings.frameSize = frameSize;
	renderSettings.convolutionType = IPL_CONVOLUTIONTYPE_PHONON;

	IPLAudioFormat outputFormat = {};
	outputFormat.channelLayoutType = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
	outputFormat.channelLayout = IPL_CHANNELLAYOUT_MONO;
	outputFormat.channelOrder = IPL_CHANNELORDER_INTERLEAVED;

	GlobalState::create(context.get(), renderSettings, outputFormat);
	s_context = this;
}
ipl::Context::~Context()
{
	GlobalState::destroy();
	s_context = nullptr;
}
IPLhandle ipl::Context::GetIplContext() const { return m_iplContext.get(); }
std::shared_ptr<ipl::Scene> ipl::Context::CreateScene() { return Scene::Create(*this); }
void ipl::Context::InvokeErrorHandler(IPLerror err)
{
	if(m_errorHandler == nullptr)
		return;
	m_errorHandler(err);
}

void ipl::Context::SetErrorHandler(const std::function<void(IPLerror)> &handler) { m_errorHandler = handler; }
void ipl::Context::SetLogHandler(const std::function<void(std::string)> &handler) { m_logHandler = handler; }

#endif
