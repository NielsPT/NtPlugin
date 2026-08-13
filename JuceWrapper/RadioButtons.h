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
  std::vector<std::unique_ptr<Toggle>> toggles;
  UiSpec& _uiSpec;

  float uiScale = 1;
  ToggleSetBase(UiSpec& uiSpec) : _uiSpec(uiSpec) { }
  std::unique_ptr<Toggle> makeToggle(std::string option, size_t i) {
    auto p_toggle = std::make_unique<Toggle>(option, i);
    this->addAndMakeVisible(p_toggle.get());
    p_toggle->setButtonText(option);
    p_toggle->setClickingTogglesState(true);
    p_toggle->setToggleable(true);
    return p_toggle;
  }

  virtual void updateToggleStates(size_t) { }

  void resized() override { this->updateUi(); }

  void updateUi() {
    auto area = this->getLocalBounds();
    auto w    = area.getWidth();
    auto h    = area.getHeight();
    auto n    = this->toggles.size();
    if (!w || !h || !n) { return; }
    int pad = int(3.0f * this->uiScale);
    for (size_t i = 0; i < n; i++) {
      auto p_toggle      = this->toggles[i].get();
      p_toggle->colour   = _uiSpec.foregroundColour;
      p_toggle->fontSize = this->_uiSpec.defaultFontSize * this->uiScale;
      auto toggleArea    = area.removeFromTop(
          int(this->_uiSpec.radioButtonHeight * this->uiScale));
      toggleArea.reduce(pad, pad);
      p_toggle->setBounds(toggleArea);
      this->updateToggleStates(i);
    }
    this->repaint();
  }
};

struct ToggleSet final : public ToggleSetBase {
  ToggleSetSpec _spec;
  ToggleSet(ToggleSetSpec spec, UiSpec& uiSpec)
      : ToggleSetBase(uiSpec), _spec(spec) {
    for (size_t i = 0; i < spec.toggles.size(); i++) {
      auto p_toggle     = this->makeToggle(spec.toggles[i].name, i);
      auto pp_toggle    = p_toggle.get();
      p_toggle->onClick = [this, pp_toggle]() {
        DBG("Onclick toggle '" << pp_toggle->getName()
                               << "': " << int(pp_toggle->getToggleState()));
        this->sendChangeMessage();
      };
      toggles.push_back(std::move(p_toggle));
    }
  }
  ToggleSet(ToggleSet&&)            = delete;
  ~ToggleSet() final                = default;
  ToggleSet& operator=(ToggleSet&&) = delete;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSet)
};

struct RadioButtonSet final : public ToggleSetBase {
  RadioButtonSetSpec& _spec;
  static int _s_id;
  RadioButtonSet(RadioButtonSetSpec& spec, UiSpec& uiSpec)
      : ToggleSetBase(uiSpec), _spec(spec) {
    for (size_t i = 0; i < spec.options.size(); i++) {
      auto option   = spec.options[i];
      auto p_toggle = this->makeToggle(option, i);
      if (!this->_spec._id) {
        this->_spec._id = ++_s_id;
        p_toggle->setRadioGroupId(
            this->_spec._id, juce::NotificationType::dontSendNotification);
      }
      auto pp_toggle    = p_toggle.get();
      p_toggle->onClick = [this, pp_toggle]() {
        if (*this->_spec.p_val == pp_toggle->_id) {
          if (!pp_toggle->getToggleState()) {
            pp_toggle->setToggleState(true, juce::sendNotification);
          }
          return;
        }
        *this->_spec.p_val = pp_toggle->_id;
        DBG("Onclick radiobutton '" << this->_spec.name
                                    << "', group ID: " << this->_spec._id
                                    << " val: " << *this->_spec.p_val);
        this->sendChangeMessage();
      };
      this->toggles.push_back(std::move(p_toggle));
    }
  }
  RadioButtonSet(RadioButtonSet&&)            = delete;
  ~RadioButtonSet() final                     = default;
  RadioButtonSet& operator=(RadioButtonSet&&) = delete;

  void updateToggleStates(size_t i) override {
    if (*this->_spec.p_val != int(i)) {
      this->toggles[i]->setToggleState(
          false, juce::NotificationType::sendNotificationSync);
    } else {
      this->toggles[i]->setToggleState(
          true, juce::NotificationType::sendNotificationSync);
    }
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RadioButtonSet)
};
}
