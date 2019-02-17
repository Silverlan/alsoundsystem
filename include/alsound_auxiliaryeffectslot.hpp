/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_AUXILIARYEFFECTSLOT_HPP__
#define __ALSOUND_AUXILIARYEFFECTSLOT_HPP__

#include "alsound_definitions.hpp"
#include <memory>

#pragma warning(push)
#pragma warning(disable:4251)
namespace alure {class AuxiliaryEffectSlot;};
namespace al
{
	class SoundSystem;
	class Effect;
	class DLLALSYS AuxiliaryEffectSlot
	{
	public:
		~AuxiliaryEffectSlot();

		void SetGain(float gain);
		float GetGain() const;

		void SetSendAuto(bool bAuto);
		bool GetSendAuto() const;

		void ApplyEffect(const Effect &effect);

		bool IsInUse() const;

		const alure::AuxiliaryEffectSlot &GetALSlot() const;
		alure::AuxiliaryEffectSlot &GetALSlot();
	private:
		AuxiliaryEffectSlot(alure::AuxiliaryEffectSlot *slot);

		alure::AuxiliaryEffectSlot *m_slot = nullptr;
		float m_gain = 1.f;
		bool m_bSendAuto = true;

		friend SoundSystem;
	};
	using PAuxiliaryEffectSlot = std::shared_ptr<AuxiliaryEffectSlot>;
	using WPAuxiliaryEffectSlot = std::weak_ptr<AuxiliaryEffectSlot>;
};
#pragma warning(pop)

#endif
