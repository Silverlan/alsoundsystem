// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.alsoundsystem:buffer;

export import :buffer_base;
export import :types;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace al {
	class SoundSystem;
	class DLLALSYS ISoundBuffer : public impl::BufferBase, public std::enable_shared_from_this<ISoundBuffer> {
	  public:
		virtual ~ISoundBuffer() override {}

		virtual bool IsReady() const = 0;
		virtual double GetDuration() const = 0;
		virtual double GetLoopPoint() const = 0;
		virtual void SetLoopPoint(double t) = 0;

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
