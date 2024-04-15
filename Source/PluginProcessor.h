/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class OddProphProjAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    OddProphProjAudioProcessor();
    ~OddProphProjAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "parameters", createParameterLayout() };

    float nextCutoff;
private:

    //std::array<juce::dsp::IIR::Filter<float>, 64> allpasses;
    std::array<juce::dsp::FirstOrderTPTFilter<float>, 64> allpasses;
    juce::dsp::FirstOrderTPTFilter<float> fixDCOffset;

    juce::dsp::BallisticsFilter<float> balistic;

    juce::AudioParameterFloat* cutoffFreq{ nullptr };
    juce::AudioParameterFloat* afGain{ nullptr };
    juce::AudioParameterFloat* modAmount{ nullptr };
    juce::AudioParameterFloat* cutoffLFO{ nullptr };
    juce::AudioParameterFloat* modLFO{ nullptr };

    float _lfoCutoff{ 0 };
    float _lfoMod{ 0 };

    //Need to add modulation into scheme
    //Test to see if it does dope things

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OddProphProjAudioProcessor)
};
