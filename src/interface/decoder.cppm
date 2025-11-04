// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:decoder;

export import :buffer_base;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace al {
	class SoundSystem;
	class DLLALSYS Decoder : public impl::BufferBase, public std::enable_shared_from_this<Decoder> {
	  public:
		~Decoder() {}

		virtual double GetDuration() const override { return 0.0; }
		virtual double GetLoopPoint() const override { return 0.0; }
		virtual void SetLoopPoint(double t) override {}

		bool Seek(uint64_t frame) {}
	  private:
		friend SoundSystem;

	  /*public:
		const std::shared_ptr<alure::Decoder> &GetALDecoder() const;
		std::shared_ptr<alure::Decoder> &GetALDecoder();
	  private:
		Decoder(const std::shared_ptr<alure::Decoder> &decoder, const std::string &path = "");
		std::shared_ptr<alure::Decoder> m_decoder = nullptr;*/
	};
	using PDecoder = std::shared_ptr<Decoder>;
};
#pragma warning(pop)
