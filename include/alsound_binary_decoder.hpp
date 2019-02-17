/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_BINARY_DECODER_HPP__
#define __ALSOUND_BINARY_DECODER_HPP__

#include "alsound_definitions.hpp"

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE && ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
#include <alure2.h>
namespace ipl {struct AudioDataBuffer;};
namespace al
{
	class DLLALSYS BinaryDecoder
		: public alure::Decoder
	{
	public:
		BinaryDecoder(std::shared_ptr<ipl::AudioDataBuffer> &audioDataBuffer);
		virtual ~BinaryDecoder() override;
		virtual ALuint getFrequency() const override;
		virtual alure::ChannelConfig getChannelConfig() const override;
		virtual alure::SampleType getSampleType() const override;
		virtual uint64_t getLength() const override;
		virtual uint64_t getPosition() const override;
		virtual bool seek(uint64_t pos) override;
		virtual std::pair<uint64_t,uint64_t> getLoopPoints() const override;
		virtual ALuint read(ALvoid *ptr, ALuint count) override;
	protected:
		uint64_t m_offset = 0ull;
		std::shared_ptr<ipl::AudioDataBuffer> m_audioDataBuffer = nullptr;
	};
};
#endif

#endif
