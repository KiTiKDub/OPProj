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
    attackAT(p.apvts, "cutoffLFO", cutoffLFO), releaseAT(p.apvts, "modLFO", modLFO)
{
    cutoffFreq.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    afGain.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modAmount.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffLFO.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modLFO.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffDepth.setSliderStyle(juce::Slider::LinearVertical);
    modDepth.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

    cutoffFreq.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    afGain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    modAmount.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    cutoffLFO.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    modLFO.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    cutoffDepth.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    modDepth.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);

    addAndMakeVisible(cutoffFreq);
    addAndMakeVisible(afGain);
    addAndMakeVisible(modAmount);
    addAndMakeVisible(cutoffLFO);
    addAndMakeVisible(modLFO);
    addAndMakeVisible(cutoffDepth);
    addAndMakeVisible(modDepth,0);

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

    auto freqArea = topArea.removeFromLeft(topArea.getWidth() * .33);
    auto gainArea = topArea.removeFromLeft(topArea.getWidth() * .5);
    auto modArea = topArea;

    auto depthAttackArea = botArea.removeFromLeft(botArea.getWidth() * .5);
    auto attackArea = depthAttackArea.reduced(botArea.getWidth() * .05, botArea.getWidth() * .05);
    depthAttackArea.removeFromRight(depthAttackArea.getWidth() * .9);
    auto depthRelArea = botArea;
    auto relArea = depthRelArea.reduced(botArea.getWidth() * .05, botArea.getWidth() * .05);

    //g.drawRect(depthAttackArea), g.drawRect(attackArea);

    g.drawFittedText("Cutoff", freqArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Gain", gainArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Mod Amount", modArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Cutoff LFO", attackArea.toNearestInt(), juce::Justification::centredTop, 1);
    g.drawFittedText("Mod LFO", relArea.toNearestInt(), juce::Justification::centredTop, 1);
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

    auto depthAttackArea = botArea.removeFromLeft(botArea.getWidth() * .5);
    auto attackArea = depthAttackArea.reduced(botArea.getWidth() * .05, botArea.getWidth() * .05);
    depthAttackArea.removeFromRight(depthAttackArea.getWidth() * .9);
    auto depthRelArea = botArea;
    auto relArea = depthRelArea.reduced(botArea.getWidth() * .05, botArea.getWidth() * .05);

    cutoffFreq.setBounds(freqArea);
    afGain.setBounds(gainArea);
    modAmount.setBounds(modArea);
    cutoffLFO.setBounds(attackArea);
    modLFO.setBounds(relArea);
    //cutoffDepth.setBounds(depthAttackArea);
    //modDepth.setBounds(depthRelArea);

    displayCutoff.setBounds(freqArea);
}

void OddProphProjAudioProcessorEditor::timerCallback()
{
    cutoffValue = audioProcessor.nextCutoff;
    displayCutoff.setName((juce::String)cutoffValue);
    displayCutoff.repaint();
}
