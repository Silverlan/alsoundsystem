/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
