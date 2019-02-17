/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_BUFFER_HPP__
#define __ALSOUND_BUFFER_HPP__

#include "alsound_definitions.hpp"
#include "impl_alsound_buffer_base.hpp"
#include <memory>

#pragma warning(push)
#pragma warning(disable:4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure {class Context; class Buffer;};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
namespace FMOD
{
	class System;
	class Sound;
};
#endif
namespace al
{
	class SoundSystem;
	class DLLALSYS SoundBuffer
		: public impl::BufferBase,public std::enable_shared_from_this<SoundBuffer>
	{
	public:
		~SoundBuffer();

		bool IsReady() const;

		virtual uint32_t GetFrequency() const override;
		virtual ChannelConfig GetChannelConfig() const override;
		virtual SampleType GetSampleType() const override;
		virtual uint64_t GetLength() const override;
		virtual std::pair<uint64_t,uint64_t> GetLoopFramePoints() const override;

		uint32_t GetSize() const;
		void SetLoopFramePoints(uint32_t start,uint32_t end);
		void SetLoopTimePoints(float tStart,float tEnd);
		// GetSources() ?

		const std::string &GetName() const;
		bool IsInUse() const;
	private:
		friend SoundSystem;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	public:
		alure::Buffer *GetALBuffer();
	private:
		SoundBuffer(alure::Context &context,alure::Buffer *buffer,const std::string &path="");
		alure::Buffer *m_buffer = nullptr;
		alure::Context &m_context;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	public:
		SoundBuffer(FMOD::System &system,const std::shared_ptr<FMOD::Sound> &sound);

		const FMOD::Sound *GetFMODSound() const;
		FMOD::Sound *GetFMODSound();
	private:
		FMOD::System &m_fmSystem;
		std::shared_ptr<FMOD::Sound> m_fmSound = nullptr;
#endif
	};
	class SoundBuffer;
	using PSoundBuffer = std::shared_ptr<SoundBuffer>;

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	class DLLALSYS Decoder
		: public SoundBuffer
	{
	public:
		using SoundBuffer::SoundBuffer;
	};
#endif
};
#pragma warning(pop)

#endif
