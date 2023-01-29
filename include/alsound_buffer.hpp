/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_BUFFER_HPP__
#define __ALSOUND_BUFFER_HPP__

#include "alsound_definitions.hpp"
#include "impl_alsound_buffer_base.hpp"
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure {
	class Context;
	class Buffer;
};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
namespace FMOD {
	class System;
	class Sound;
};
#endif
namespace al {
	class SoundSystem;
	class DLLALSYS ISoundBuffer : public impl::BufferBase, public std::enable_shared_from_this<ISoundBuffer> {
	  public:
		virtual ~ISoundBuffer() override {}

		virtual bool IsReady() const = 0;

		virtual uint32_t GetSize() const = 0;
		virtual void SetLoopFramePoints(uint32_t start, uint32_t end) = 0;
		virtual void SetLoopTimePoints(float tStart, float tEnd) = 0;
		// GetSources() ?

		virtual std::string GetName() const = 0;
		virtual bool IsInUse() const = 0;
	  private:
		friend SoundSystem;
	};
	class ISoundBuffer;
	using PSoundBuffer = std::shared_ptr<ISoundBuffer>;

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	class DLLALSYS Decoder : public ISoundBuffer {
	  public:
		using ISoundBuffer::ISoundBuffer;
	};
#endif
};
#pragma warning(pop)

#endif
