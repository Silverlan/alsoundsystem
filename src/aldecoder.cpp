/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_decoder.hpp"

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>

al::Decoder::Decoder(const std::shared_ptr<alure::Decoder> &decoder,const std::string &path)
	: impl::BufferBase(path),m_decoder(decoder)
{}

al::Decoder::~Decoder()
{
	m_decoder.reset();
}

uint32_t al::Decoder::GetFrequency() const {return m_decoder->getFrequency();}
al::ChannelConfig al::Decoder::GetChannelConfig() const {return static_cast<ChannelConfig>(m_decoder->getChannelConfig());}
al::SampleType al::Decoder::GetSampleType() const {return static_cast<SampleType>(m_decoder->getSampleType());}
uint64_t al::Decoder::GetLength() const {return m_decoder->getLength();}
uint64_t al::Decoder::GetPosition() const {return m_decoder->getPosition();}
std::pair<uint64_t,uint64_t> al::Decoder::GetLoopFramePoints() const {return m_decoder->getLoopPoints();}
bool al::Decoder::Seek(uint64_t frame) {return m_decoder->seek(frame);}

const std::shared_ptr<alure::Decoder> &al::Decoder::GetALDecoder() const {return const_cast<Decoder*>(this)->GetALDecoder();}
std::shared_ptr<alure::Decoder> &al::Decoder::GetALDecoder() {return m_decoder;}

#endif
