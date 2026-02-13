/*
 * Copyright (C) 2026 Niels Thøgersen, NTlyd
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_processors_headless/juce_audio_processors_headless.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>
#include <string>
#include <vector>

#include "Toggle.h"
#include "lib/UiSpec.h"

namespace NtFx {
struct RadioButton : public juce::Component, public juce::ChangeBroadcaster {
  std::vector<std::unique_ptr<Toggle>> toggles;
  RadioButtonSpec& spec;
  UiSpec& uiSpec;
  // TODO: static id.
  int id        = 1;
  float uiScale = 1;
  int val;
  RadioButton(RadioButtonSpec& spec, UiSpec& uiSpec)
      : spec(spec), uiSpec(uiSpec) {
    this->id++;
    for (size_t i = 0; i < spec.options.size(); i++) {
      auto option   = spec.options[i];
      auto p_toggle = std::make_unique<Toggle>(option);
      this->addAndMakeVisible(p_toggle.get());
      p_toggle->setButtonText(option);
      p_toggle->setClickingTogglesState(true);
      p_toggle->setToggleable(true);
      p_toggle->setRadioGroupId(this->id);
      p_toggle->onClick = [this, i]() { this->toggleStateChanged(i); };
      toggles.push_back(std::move(p_toggle));
    }
  }
  void resized() override { this->updateUi(); }

  void toggleStateChanged(int idx) {
    this->val = idx + 1;
    this->sendChangeMessage();
  }

  void updateUi() {
    auto area = this->getLocalBounds();
    auto w    = area.getWidth();
    auto h    = area.getHeight();
    auto n    = this->toggles.size();
    float pad = 3 * this->uiScale;
    if (!w || !h || !n) { return; }
    for (size_t i = 0; i < n; i++) {
      auto p_toggle      = this->toggles[i].get();
      p_toggle->colour   = uiSpec.foregroundColour;
      p_toggle->fontSize = this->uiSpec.defaultFontSize * this->uiScale;
      auto toggleArea =
          area.removeFromTop(this->uiSpec.radioButtonHeight * this->uiScale);
      toggleArea.reduce(pad, pad);
      p_toggle->setBounds(toggleArea);
      if (this->val - 1 == i) {
        p_toggle->setToggleState(
            // TODO: Do we need a notification here?
            true,
            juce::NotificationType::sendNotification);
      }
    }
    this->repaint();
  }
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RadioButton)
};
}