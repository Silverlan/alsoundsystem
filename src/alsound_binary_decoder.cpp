/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_binary_decoder.hpp"
#include "steam_audio/alsound_steam_audio.hpp"
#include <mathutil/umath.h>

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE && ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
al::BinaryDecoder::BinaryDecoder(std::shared_ptr<ipl::AudioDataBuffer> &audioDataBuffer) : m_audioDataBuffer(audioDataBuffer) {}
al::BinaryDecoder::~BinaryDecoder() {}
ALuint al::BinaryDecoder::getFrequency() const { return ALSYS_DECODER_FREQUENCY; }
alure::ChannelConfig al::BinaryDecoder::getChannelConfig() const { return alure::ChannelConfig::Mono; }
alure::SampleType al::BinaryDecoder::getSampleType() const { return alure::SampleType::Float32; }
uint64_t al::BinaryDecoder::getLength() const { return m_audioDataBuffer->audioResampler->GetOutputData().size(); }
uint64_t al::BinaryDecoder::getPosition() const { return m_offset; }
bool al::BinaryDecoder::seek(uint64_t pos)
{
	if(pos >= getLength())
		return false;
	m_offset = pos;
	return true;
}
std::pair<uint64_t, uint64_t> al::BinaryDecoder::getLoopPoints() const { return {0ull, 0ull}; }
ALuint al::BinaryDecoder::read(ALvoid *ptr, ALuint count)
{
	if(m_offset >= getLength())
		return 0u;
	auto readCount = umath::min<uint64_t>(getLength() - m_offset, static_cast<uint64_t>(count));
	memcpy(ptr, m_audioDataBuffer->audioResampler->GetOutputData().data() + m_offset, readCount * sizeof(float));
	m_offset += readCount;
	return readCount;
}
#endif
