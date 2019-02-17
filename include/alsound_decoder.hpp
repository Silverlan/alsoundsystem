/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_DECODER_HPP__
#define __ALSOUND_DECODER_HPP__

#include "alsound_definitions.hpp"
#include "impl_alsound_buffer_base.hpp"
#include <memory>

#pragma warning(push)
#pragma warning(disable:4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure {class Decoder;};
#endif
namespace al
{
	class SoundSystem;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	class DLLALSYS Decoder
		: public impl::BufferBase,public std::enable_shared_from_this<Decoder>
	{
	public:
		~Decoder();

		virtual uint32_t GetFrequency() const override;
		virtual ChannelConfig GetChannelConfig() const override;
		virtual SampleType GetSampleType() const override;
		virtual uint64_t GetLength() const override;
		virtual std::pair<uint64_t,uint64_t> GetLoopFramePoints() const override;

		bool Seek(uint64_t frame);
		uint64_t GetPosition() const;
	private:
		friend SoundSystem;

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	public:
		const std::shared_ptr<alure::Decoder> &GetALDecoder() const;
		std::shared_ptr<alure::Decoder> &GetALDecoder();
	private:
		Decoder(const std::shared_ptr<alure::Decoder> &decoder,const std::string &path="");
		std::shared_ptr<alure::Decoder> m_decoder = nullptr;
#endif
	};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	class Decoder;
#endif
	using PDecoder = std::shared_ptr<Decoder>;
};
#pragma warning(pop)

#endif
