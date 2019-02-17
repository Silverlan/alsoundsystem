/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_buffer.hpp"
#include "alsoundsystem.hpp"
#include "alsound_settings.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#endif

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
al::SoundBuffer::SoundBuffer(alure::Context &context,alure::Buffer *buffer,const std::string &path)
	: impl::BufferBase(path),m_context(context),m_buffer(buffer)
{}
alure::Buffer *al::SoundBuffer::GetALBuffer() {return m_buffer;}
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
al::SoundBuffer::SoundBuffer(FMOD::System &system,const std::shared_ptr<FMOD::Sound> &sound)
	: m_fmSystem(system),m_fmSound(sound)
{}
const FMOD::Sound *al::SoundBuffer::GetFMODSound() const {return const_cast<al::SoundBuffer*>(this)->GetFMODSound();}
FMOD::Sound *al::SoundBuffer::GetFMODSound() {return m_fmSound.get();}
#endif

al::SoundBuffer::~SoundBuffer()
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_context.removeBuffer(m_buffer);
#endif
}

bool al::SoundBuffer::IsReady() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	auto status = m_buffer->getLoadStatus();
	return (status == alure::BufferLoadStatus::Ready) ? true : false;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD_OPENSTATE openState;
	uint32_t percentBuffered;
	bool starving,diskBusy;
	al::fmod::check_result(m_fmSound->getOpenState(&openState,&percentBuffered,&starving,&diskBusy));
	return openState == FMOD_OPENSTATE_READY;
#endif
}

uint64_t al::SoundBuffer::GetLength() const
{
	if(IsReady() == false)
		return 0;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_buffer->getLength();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	auto length = 0u;
	al::fmod::check_result(m_fmSound->getLength(&length,FMOD_TIMEUNIT_MS));
	return length;
#endif
}
uint32_t al::SoundBuffer::GetFrequency() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_buffer->getFrequency();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0u;
#endif
}
al::ChannelConfig al::SoundBuffer::GetChannelConfig() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return static_cast<al::ChannelConfig>(m_buffer->getChannelConfig());
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	int32_t channels;
	al::fmod::check_result(m_fmSound->getFormat(nullptr,nullptr,&channels,nullptr));
	return (channels >= 2) ? al::ChannelConfig::Stereo : al::ChannelConfig::Mono;
#endif
}
al::SampleType al::SoundBuffer::GetSampleType() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return static_cast<al::SampleType>(m_buffer->getSampleType());
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	FMOD_SOUND_FORMAT format;
	al::fmod::check_result(m_fmSound->getFormat(nullptr,&format,nullptr,nullptr));
	switch(format)
	{
		case FMOD_SOUND_FORMAT_PCMFLOAT:
			return al::SampleType::Float32;
		case FMOD_SOUND_FORMAT_PCM8:
			return al::SampleType::UInt8;
		case FMOD_SOUND_FORMAT_PCM16:
			return al::SampleType::Int16;
		default:
			// FMOD TODO
			throw std::runtime_error("Unsupported sample type " +std::to_string(format));
			break;
	}
#endif
}
uint32_t al::SoundBuffer::GetSize() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(IsReady() == false)
		return 0;
	return m_buffer->getSize();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return 0u;
#endif
}
void al::SoundBuffer::SetLoopFramePoints(uint32_t start,uint32_t end)
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(IsReady() == false)
		return; // TODO: Set when ready
	m_buffer->setLoopPoints(start,end);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	al::fmod::check_result(m_fmSound->setLoopPoints(start,FMOD_TIMEUNIT_PCM,end,FMOD_TIMEUNIT_PCM));
#endif
}
void al::SoundBuffer::SetLoopTimePoints(float tStart,float tEnd)
{
	auto dur = GetDuration();
	auto start = 0u;
	auto end = 0u;
	if(dur > 0.f)
	{
		tStart /= dur;
		tEnd /= dur;

		auto l = GetLength();
		start = umath::round(tStart *l);
		end = umath::round(tEnd *l);
	}
	SetLoopFramePoints(start,end);
}
std::pair<uint64_t,uint64_t> al::SoundBuffer::GetLoopFramePoints() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	if(IsReady() == false)
		return {0,0};
	return m_buffer->getLoopPoints();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	uint32_t start,end;
	al::fmod::check_result(m_fmSound->getLoopPoints(&start,FMOD_TIMEUNIT_PCM,&end,FMOD_TIMEUNIT_PCM));
	return {start,end};
#endif
}

const std::string &al::SoundBuffer::GetName() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_buffer->getName();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	static std::string r = "";
	return r;
#endif
}
bool al::SoundBuffer::IsInUse() const
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	return m_buffer->isInUse();
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	// FMOD TODO
	return true;
#endif
}
