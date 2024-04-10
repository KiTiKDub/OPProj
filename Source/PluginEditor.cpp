/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OddProphProjAudioProcessorEditor::OddProphProjAudioProcessorEditor (OddProphProjAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), cutoffFreqAT(p.apvts, "cutoffFreq", cutoffFreq),
    afGainAT(p.apvts, "afGain", afGain), modAmountAT(p.apvts, "modAmount", modAmount),
    attackAT(p.apvts, "attack", attack), releaseAT(p.apvts, "release", release)
{
    cutoffFreq.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    afGain.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modAmount.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    attack.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    release.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    cutoffFreq.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    afGain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    modAmount.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    attack.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    release.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);

    addAndMakeVisible(cutoffFreq);
    addAndMakeVisible(afGain);
    addAndMakeVisible(modAmount);
    addAndMakeVisible(attack);
    addAndMakeVisible(release);

   /* addAndMakeVisible(displayCutoff);
    displayCutoff.setSize(50, 50);*/

    setSize (400, 400);
}

OddProphProjAudioProcessorEditor::~OddProphProjAudioProcessorEditor()
{
}

//==============================================================================
void OddProphProjAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    

    auto bounds = getLocalBounds();

    auto dialArea = bounds.reduced(bounds.getWidth() * .05, bounds.getHeight() * .05);
    auto topArea = dialArea.removeFromTop(dialArea.getHeight() * .5);
    auto botArea = dialArea;

    auto freqArea = topArea.removeFromLeft(dialArea.getWidth() * .33);
    auto gainArea = topArea.removeFromLeft(dialArea.getWidth() * .5);
    auto modArea = topArea;

    auto attackArea = botArea.removeFromLeft(botArea.getWidth() * .5);
    auto relArea = botArea;

    g.drawFittedText("Cutoff", freqArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Gain", gainArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Mod Amount", modArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Attack", attackArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Release", relArea.toNearestInt(), juce::Justification::centredTop, 1);
}

void OddProphProjAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto dialArea = bounds.reduced(bounds.getWidth() * .05, bounds.getHeight() * .05);
    auto topArea = dialArea.removeFromTop(dialArea.getHeight() * .5);
    auto botArea = dialArea;

    auto freqArea = topArea.removeFromLeft(topArea.getWidth() * .33);
    auto gainArea = topArea.removeFromLeft(topArea.getWidth() * .5);
    auto modArea = topArea;

    auto attackArea = botArea.removeFromLeft(botArea.getWidth() * .5);
    auto relArea = botArea;

    cutoffFreq.setBounds(freqArea);
    afGain.setBounds(gainArea);
    modAmount.setBounds(modArea);
    attack.setBounds(attackArea);
    release.setBounds(relArea);

    displayCutoff.setBounds(freqArea);
}

void OddProphProjAudioProcessorEditor::timerCallback()
{
    cutoffValue = audioProcessor.nextCutoff;
    displayCutoff.setName((juce::String)cutoffValue);
    displayCutoff.repaint();
}
