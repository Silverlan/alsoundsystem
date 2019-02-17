/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "impl_alsound_buffer_base.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#endif

al::impl::BufferLoadData::BufferLoadData(SoundSystem &sys)
	: soundSystem(sys)
{}
al::impl::BufferBase::BufferBase::BufferBase(const std::string &path)
	: m_filePath(path)
{}

std::string al::impl::BufferBase::GetChannelConfigName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return alure::GetChannelConfigName(static_cast<alure::ChannelConfig>(GetChannelConfig()));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}
std::string al::impl::BufferBase::GetSampleTypeName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return alure::GetSampleTypeName(static_cast<alure::SampleType>(GetSampleType()));
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return "";
#endif
}
std::string al::impl::BufferBase::GetFilePath() const {return m_filePath;}

void al::impl::BufferBase::SetTargetChannelConfig(ChannelConfig config) {m_targetChannelConfig = std::make_unique<ChannelConfig>(config);}
al::ChannelConfig al::impl::BufferBase::GetTargetChannelConfig() const {return (m_targetChannelConfig != nullptr) ? *m_targetChannelConfig : GetChannelConfig();}

float al::impl::BufferBase::GetInverseFrequency() const {return 1.f /GetFrequency();}
float al::impl::BufferBase::GetDuration() const {return GetLength() *GetInverseFrequency();}

void al::impl::BufferBase::SetUserData(const std::shared_ptr<void> &userData) {m_userData = userData;}
std::shared_ptr<void> al::impl::BufferBase::GetUserData() const {return m_userData;}

std::pair<float,float> al::impl::BufferBase::GetLoopTimePoints() const
{
	auto loopPoints = GetLoopFramePoints();
	auto l = static_cast<float>(GetLength());
	auto start = 0.f;
	auto end = 0.f;
	if(l > 0)
	{
		auto dur = GetDuration();
		start = (loopPoints.first /l) *dur;
		end = (loopPoints.second /l) *dur;
	}
	return {start,end};
}

bool al::impl::BufferBase::IsMono() const {return (GetChannelConfig() == al::ChannelConfig::Mono) ? true : false;}
bool al::impl::BufferBase::IsStereo() const {return (GetChannelConfig() == al::ChannelConfig::Stereo) ? true : false;}
