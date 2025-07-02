// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ALSOUND_AUXILIARYEFFECTSLOT_HPP__
#define __ALSOUND_AUXILIARYEFFECTSLOT_HPP__

#include "alsound_definitions.hpp"
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4251)
namespace al {
	class ISoundSystem;
	class IEffect;
	class DLLALSYS IAuxiliaryEffectSlot {
	  public:
		virtual ~IAuxiliaryEffectSlot() {}

		virtual void SetGain(float gain) = 0;
		virtual float GetGain() const = 0;

		virtual void SetSendAuto(bool bAuto) = 0;
		virtual bool GetSendAuto() const = 0;

		virtual void ApplyEffect(const IEffect &effect) = 0;

		virtual bool IsInUse() const = 0;
	  private:
		IAuxiliaryEffectSlot();

		friend ISoundSystem;
	};
	using PAuxiliaryEffectSlot = std::shared_ptr<IAuxiliaryEffectSlot>;
	using WPAuxiliaryEffectSlot = std::weak_ptr<IAuxiliaryEffectSlot>;
};
#pragma warning(pop)

#endif
