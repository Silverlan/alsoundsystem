// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.soundsystem:buffer_base;

export import :types;

#pragma warning(push)
#pragma warning(disable : 4251)
export {
	namespace ipl {
		struct AudioDataBuffer;
	};
	namespace al {
		class SoundSystem;
		namespace impl {
			struct BufferLoadData {
				enum class Flags : uint32_t { None = 0, ConvertToMono = 1u, SingleSourceDecoder = ConvertToMono << 1u };
				BufferLoadData(SoundSystem &sys);
				SoundSystem &soundSystem;
				Flags flags = Flags::None;
				std::shared_ptr<void> userData = nullptr;
				std::weak_ptr<ipl::AudioDataBuffer> buffer = {};
			};
			using namespace umath::scoped_enum::bitwise;

			class DLLALSYS BufferBase {
			  public:
				virtual ~BufferBase() = default;
				virtual double GetDuration() const = 0;
				virtual double GetLoopPoint() const = 0;
				virtual void SetLoopPoint(double t) = 0;

				void SetTargetChannelConfig(ChannelConfig config);
				ChannelConfig GetTargetChannelConfig() const;

				std::string GetFilePath() const;
				bool IsMono() const;
				virtual bool IsStereo() const = 0;

				void SetUserData(const std::shared_ptr<void> &userData);
				std::shared_ptr<void> GetUserData() const;
			  protected:
				BufferBase(const std::string &path = "");
				std::string m_filePath;
				std::shared_ptr<void> m_userData = nullptr;
				// If the sound is loaded asynchronously and is selected to be converted
				// to mono, the buffer's channel config will be stereo until the sound
				// has been fully loaded. Target channel config is the channel config the
				// buffer will have ultimately.
				std::unique_ptr<ChannelConfig> m_targetChannelConfig = nullptr;
			};
		};
	};

	REGISTER_ENUM_FLAGS(al::impl::BufferLoadData::Flags)
}
#pragma warning(pop)
