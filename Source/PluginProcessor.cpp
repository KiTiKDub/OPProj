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
    attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("attack"));
    release = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("release"));

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

    /*
    //create lfo value in polar domain
    auto currentLfoValue = lfoRate->get() * buffer.getNumSamples() / getSampleRate() * juce::MathConstants<float>::twoPi;

    //create sine wave values
    _lfoValue += currentLfoValue;
    if (_lfoValue > juce::MathConstants<float>::pi) { _lfoValue -= juce::MathConstants<float>::twoPi; }

    //convert to be between -1 * 1
    auto sineValue = std::sin(_lfoValue);

    //update based on much the user want to ossiclate
    auto cutoffMod = lfoDepth->get() * sineValue;
    */

    //auto leftBlock = block.getSingleChannelBlock(0);
    //auto rightBlock = block.getSingleChannelBlock(1);
    //auto leftContext = juce::dsp::ProcessContextReplacing<float>(leftBlock);
    //auto rightContext = juce::dsp::ProcessContextReplacing<float>(rightBlock);

    ////get rmsValue
    //rmsValue = 0;

    //for (int i = 0; i < totalNumInputChannels; i++)
    //{
    //    auto check = buffer.getRMSLevel(i, 0, buffer.getNumSamples());
    //    rmsValue += juce::Decibels::gainToDecibels(buffer.getRMSLevel(i, 0, buffer.getNumSamples()));
    //}

    //if (rmsValue < -120) { rmsValue = -120; }

    ////Modulation total
    //auto afTotal = rmsValue/totalNumInputChannels + afGain->get();

    auto block = juce::dsp::AudioBlock<float>(buffer);

    //process attack and release times
    balistic.setAttackTime(attack->get());
    balistic.setReleaseTime(release->get());

    auto grabCutoff = cutoffFreq->get();
    auto grabGain = afGain->get();

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

    for (int ch = 0; ch < totalNumInputChannels; ch++)
    {
        auto* data = block.getChannelPointer(ch);
        auto rmsGain = buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        auto rms = juce::Decibels::gainToDecibels(rmsGain, -72.f);
        auto freqAdjusted = juce::jmap(grabGain + rms, -72.f, 24.f, -100.f, 100.f);

        for (int s = 0; s < buffer.getNumSamples(); s++)
        {
            float sample = data[s];

            nextCutoff = grabCutoff + (std::floorf(freqAdjusted) * modAmount->get());

            for (int filter = 0; filter < allpasses.size()/*JUCE_LIVE_CONSTANT(2)*/; filter++)
            {
                if(s % 100 == 0)
                    allpasses[filter].setCutoffFrequency(nextCutoff);
                sample = allpasses[filter].processSample(ch, sample);
            }

            /*auto sampleDb = juce::Decibels::gainToDecibels(sample) + grabGain;

            sample = juce::Decibels::decibelsToGain(sampleDb);

            auto newLimit = juce::Decibels::decibelsToGain(-24);
            auto inverse = 1 / newLimit;
            auto resizeSamples = sample * inverse;
            resizeSamples > 1 ? resizeSamples = 1 : resizeSamples = resizeSamples;
            resizeSamples < -1 ? resizeSamples = -1 : resizeSamples = resizeSamples;
            auto cubic = (resizeSamples - pow(resizeSamples, 3) / 3);
            sample = cubic * newLimit;*/

            data[s] = sample;

        }
    }
    //Boost the actual signal by the imput amount
    //somehow keep the distortion

    ////DBG("cutoff Mod " << nextValue);

    ////lets say maximum this can go up is 200 hz swing
    //auto freqAdjusted = juce::jmap(nextValue, -24.f, 30.f, 0.f, 200.f);

    ////context for all passes, need to seperate left and right channels
    //

    ////DBG("Freq Adjusted " << freqAdjusted);

    ////get new cutoff after modulation, make sure this does not go below zero
    //auto nextCutoff = cutoffFreq->get() + freqAdjusted;
    //if (nextCutoff < 1) { nextCutoff = 1; }

    ////update all passes
    //auto filterCoe = juce::dsp::IIR::Coefficients<float>::makeAllPass(getSampleRate(), nextCutoff);

    ////process all passes
    //for(int filter = 0; filter < allpasses.size(); filter += 2)
    //{
    //    allpasses[filter].coefficients = filterCoe;
    //    allpasses[filter + 1].coefficients = filterCoe;
    //    allpasses[filter].process(leftContext);
    //    allpasses[filter + 1].process(rightContext);
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

    auto cuttoff = NormalisableRange<float>(20, 2000, 1, 1); //update skew
    auto gain = NormalisableRange<float>(0, 24, .01, 1);
    auto adsr = NormalisableRange<float>(0, 5000, 1, 1); //update skew
    auto zeroToOne = NormalisableRange<float>(0, 1, .01);

    layout.add(std::make_unique<AudioParameterFloat>("cutoffFreq", "Cuttoff Freq", cuttoff, 200));
    layout.add(std::make_unique<AudioParameterFloat>("afGain", "Gain", gain, 0));
    layout.add(std::make_unique<AudioParameterFloat>("modAmount", "Mod Amount", zeroToOne, 1));
    layout.add(std::make_unique<AudioParameterFloat>("attack", "Attack", adsr, 10));
    layout.add(std::make_unique<AudioParameterFloat>("release", "release", adsr, 100));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OddProphProjAudioProcessor();
}
