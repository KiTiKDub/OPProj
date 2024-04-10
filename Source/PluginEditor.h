/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class OddProphProjAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    OddProphProjAudioProcessorEditor (OddProphProjAudioProcessor&);
    ~OddProphProjAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    OddProphProjAudioProcessor& audioProcessor;

    juce::Slider cutoffFreq, afGain, modAmount, attack, release;

    juce::AudioProcessorValueTreeState::SliderAttachment cutoffFreqAT, afGainAT, modAmountAT, attackAT, releaseAT;

    juce::Component displayCutoff;

    float cutoffValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OddProphProjAudioProcessorEditor)
};
