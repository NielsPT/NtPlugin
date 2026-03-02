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

#include <bitset>
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

struct ToggleSetBase : public juce::Component, public juce::ChangeBroadcaster {
  UiSpec& uiSpec;
  std::vector<std::unique_ptr<Toggle>> toggles;

  float uiScale = 1;
  ToggleSetBase(UiSpec& uiSpec) : uiSpec(uiSpec) { }
  std::unique_ptr<Toggle> makeToggle(std::string option) {
    auto p_toggle = std::make_unique<Toggle>(option);
    this->addAndMakeVisible(p_toggle.get());
    p_toggle->setButtonText(option);
    p_toggle->setClickingTogglesState(true);
    p_toggle->setToggleable(true);
    return std::move(p_toggle);
  }

  virtual void updateToggleStates(int i) = 0;

  void resized() override { this->updateUi(); }

  void updateUi() {
    auto area = this->getLocalBounds();
    auto w    = area.getWidth();
    auto h    = area.getHeight();
    auto n    = this->toggles.size();
    if (!w || !h || !n) { return; }
    float pad = 3 * this->uiScale;
    for (size_t i = 0; i < n; i++) {
      auto p_toggle      = this->toggles[i].get();
      p_toggle->colour   = uiSpec.foregroundColour;
      p_toggle->fontSize = this->uiSpec.defaultFontSize * this->uiScale;
      auto toggleArea =
          area.removeFromTop(this->uiSpec.radioButtonHeight * this->uiScale);
      toggleArea.reduce(pad, pad);
      p_toggle->setBounds(toggleArea);
      this->updateToggleStates(i);
    }
    this->repaint();
  }
};

struct ToggleGroup : public ToggleSetBase {
  ToggleGroupSpec spec;
  // std::bitset<64> vals;
  ToggleGroup(ToggleGroupSpec spec, UiSpec& uiSpec)
      : spec(spec), ToggleSetBase(uiSpec)
  // TODO: What about default val?
  {
    for (size_t i = 0; i < spec.toggles.size(); i++) {
      auto p_toggle     = this->makeToggle(spec.toggles[i].name);
      p_toggle->onClick = [this]() { this->sendChangeMessage(); };
      toggles.push_back(std::move(p_toggle));
    }
  }

  virtual void updateToggleStates(int i) override {
    for (size_t i = 0; i < this->spec.toggles.size(); i++) {
      // TODO: ??
    }
  }
};

struct RadioButtonSet : public ToggleSetBase {
  RadioButtonSetSpec& spec;
  // TODO: Hmmm. What if we just used the bitset here as well? That would make
  // the classes more like the same.
  // TODO: Can we find a way to not use a static?
  static int id;
  int val;
  RadioButtonSet(RadioButtonSetSpec& spec, UiSpec& uiSpec)
      : spec(spec), ToggleSetBase(uiSpec), val(spec._defaultVal) {
    this->id++;
    for (size_t i = 0; i < spec.options.size(); i++) {
      auto option   = spec.options[i];
      auto p_toggle = this->makeToggle(option);
      p_toggle->setRadioGroupId(this->id);
      p_toggle->onClick = [this, i]() {
        this->val = i;
        this->sendChangeMessage();
      };
      toggles.push_back(std::move(p_toggle));
    }
  }

  virtual void updateToggleStates(int i) override {
    if (this->val == i) {
      this->toggles[i]->setToggleState(
          true, juce::NotificationType::dontSendNotification);
    }
  }
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RadioButtonSet)
};
}