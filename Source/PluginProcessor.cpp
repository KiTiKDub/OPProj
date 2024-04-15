/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OddProphProjAudioProcessor::OddProphProjAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    cutoffFreq = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("cutoffFreq"));
    afGain = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("afGain"));
    modAmount = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modAmount"));
    cutoffLFO = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("cutoffLFO"));
    modLFO = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("modLFO"));

}

OddProphProjAudioProcessor::~OddProphProjAudioProcessor()
{
}

//==============================================================================
const juce::String OddProphProjAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OddProphProjAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OddProphProjAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OddProphProjAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OddProphProjAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OddProphProjAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OddProphProjAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OddProphProjAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OddProphProjAudioProcessor::getProgramName (int index)
{
    return {};
}

void OddProphProjAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OddProphProjAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;

    for (auto& all : allpasses)
    {
        all.reset();
        all.prepare(spec);
        all.setType(juce::dsp::FirstOrderTPTFilterType::allpass);   
    }

    balistic.prepare(spec);
    balistic.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::RMS);

    fixDCOffset.prepare(spec);
    fixDCOffset.setType(juce::dsp::FirstOrderTPTFilterType::highpass);
    fixDCOffset.setCutoffFrequency(5);
}

void OddProphProjAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OddProphProjAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OddProphProjAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    //create lfo value in polar domain, this goes up to a rate of 50
    auto currentLfoValue = (cutoffLFO->get() * 50) * buffer.getNumSamples() / getSampleRate() * juce::MathConstants<float>::twoPi;
    auto modLFOValue = (modLFO->get() * 50) * buffer.getNumSamples() / getSampleRate() * juce::MathConstants<float>::twoPi;

    //create sine wave values
    _lfoCutoff += currentLfoValue;
    if (_lfoCutoff > juce::MathConstants<float>::pi) { _lfoCutoff -= juce::MathConstants<float>::twoPi; }
    _lfoMod += modLFOValue;
    if (_lfoMod > juce::MathConstants<float>::pi) { _lfoMod -= juce::MathConstants<float>::twoPi; }

    //convert to be between -1 * 1, add depth
    auto sineValueCutoff = std::sin(_lfoCutoff);
    auto cuttoffChange = sineValueCutoff * cutoffLFO->get();
    auto sineValueCutoffMod = std::sin(_lfoMod);
    auto modChange = sineValueCutoffMod * modLFO->get();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    //process attack and release times
    balistic.setAttackTime(10);
    balistic.setReleaseTime(100);

    auto grabCutoff = cutoffFreq->get();
    auto grabGain = afGain->get();
    auto grabMod = modAmount->get();

    float newRms = 0;

    for (int ch = 0; ch < totalNumInputChannels; ch++)
    {
        newRms += buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
    }

    newRms /= 2;
    newRms = juce::Decibels::gainToDecibels(newRms, -72.f);

    auto followerDB = newRms + grabGain;

    auto modulatedCuttoff = std::fmin(std::fmax(grabCutoff + (cuttoffChange * 1000), 20), 2000);
    auto modulatedMod = std::fmin(std::fmax(grabMod + (modChange * .5), 0), 1);

    //DBG("Mod Cutoff: " << modulatedMod);

    for (int ch = 0; ch < totalNumInputChannels; ch++)
    {
        auto* data = block.getChannelPointer(ch);

        for (int s = 0; s < buffer.getNumSamples(); s++)
        {
            float sample = data[s];

            auto nextFollowerValue = balistic.processSample(ch, juce::Decibels::decibelsToGain(followerDB, -72.f));

            auto freqAdjusted = juce::jmap(nextFollowerValue, 0.f, 1.f, 0.f, 1000.f);

            nextCutoff = modulatedCuttoff + (std::floorf(freqAdjusted) * modulatedMod);

            for (int filter = 0; filter < allpasses.size()/*JUCE_LIVE_CONSTANT(2)*/; filter++)
            {
                if(s % 100 == 0)
                    allpasses[filter].setCutoffFrequency(nextCutoff);
                sample = allpasses[filter].processSample(ch, sample);
            }

            auto power = pow(sample, 2) / abs(sample) * 1.25f;
            auto distort = (sample / abs(sample)) * (1 - std::exp(-power));

            data[s] = distort;
        }
    }

    fixDCOffset.process(context);


    //=====================================================ALT VERSION==================================================

    //for (int ch = 0; ch < totalNumInputChannels; ch++)
    //{
    //    auto* data = block.getChannelPointer(ch);
    //    auto rmsGain = buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
    //    auto rms = juce::Decibels::gainToDecibels(rmsGain, -72.f);

    //    for (int s = 0; s < buffer.getNumSamples(); s++)
    //    {
    //        float sample = data[s];

    //        auto freqAdjusted = juce::jmap(afGain->get() + rms, -72.f, 24.f, 0.f, 10000.f);

    //        nextCutoff = grabCutoff + (std::floorf(freqAdjusted) * modAmount->get());

    //        for (int filter = 0; filter < allpasses.size()/*JUCE_LIVE_CONSTANT(2)*/; filter++)
    //        {
    //            allpasses[filter].setCutoffFrequency(nextCutoff);
    //            sample = allpasses[filter].processSample(ch, sample);    
    //        }

    //        auto sampleDb = juce::Decibels::gainToDecibels(sample) + afGain->get();

    //        sample = juce::Decibels::decibelsToGain(sampleDb);

    //        auto newLimit = juce::Decibels::decibelsToGain(-24);
    //        auto inverse = 1 / newLimit;
    //        auto resizeSamples = sample * inverse;
    //        resizeSamples > 1 ? resizeSamples = 1 : resizeSamples = resizeSamples;
    //        resizeSamples < -1 ? resizeSamples = -1 : resizeSamples = resizeSamples;
    //        auto cubic = (resizeSamples - pow(resizeSamples, 3) / 3);
    //        sample = cubic * newLimit;

    //        data[s] = sample;
    //    }
    //}
 
}

//==============================================================================
bool OddProphProjAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OddProphProjAudioProcessor::createEditor()
{
    return new OddProphProjAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void OddProphProjAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OddProphProjAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout OddProphProjAudioProcessor::createParameterLayout()
{
    using namespace juce;
    
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto cuttoff = NormalisableRange<float>(20, 2000, 1, .5); //update skew
    auto gain = NormalisableRange<float>(0, 24, .01, 1);
    //auto adsr = NormalisableRange<float>(0, 5000, 1, 1); //update skew
    auto rate = NormalisableRange<float>(.1, 100, .1, .4);
    auto zeroToOne = NormalisableRange<float>(0, 1, .01);

    layout.add(std::make_unique<AudioParameterFloat>("cutoffFreq", "Cuttoff Freq", cuttoff, 200));
    layout.add(std::make_unique<AudioParameterFloat>("afGain", "Gain", gain, 0));
    layout.add(std::make_unique<AudioParameterFloat>("modAmount", "Mod Amount", zeroToOne, 1));
    layout.add(std::make_unique<AudioParameterFloat>("cutoffLFO", "cutoffLFO", zeroToOne, 0));
    layout.add(std::make_unique<AudioParameterFloat>("modLFO", "modLFO", zeroToOne, 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OddProphProjAudioProcessor();
}
