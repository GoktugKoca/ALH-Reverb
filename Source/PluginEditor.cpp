/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ALHReverbAudioProcessorEditor::ALHReverbAudioProcessorEditor (ALHReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 300);
    
    addAndMakeVisible(GUI);
}

ALHReverbAudioProcessorEditor::~ALHReverbAudioProcessorEditor()
{
}

//==============================================================================
void ALHReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);
    
    
    if (GUI.rotary1.getValue() == 31) {
        tby = juce::ImageCache::getFromMemory(BinaryData::tby_png, BinaryData::tby_pngSize);
        g.drawImageWithin(tby, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
        
    }
    else {
        background = juce::ImageCache::getFromMemory(BinaryData::bg_png, BinaryData::bg_pngSize);
        g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::doNotResize);
    }
    
}

void ALHReverbAudioProcessorEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    
    GUI.setBounds(area);
}
