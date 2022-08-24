/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUIComponent.h"


//==============================================================================
/**
 */
class ALHReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ALHReverbAudioProcessorEditor (ALHReverbAudioProcessor&);
    ~ALHReverbAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ALHReverbAudioProcessor& audioProcessor;
    
    
    GUIComponent GUI;
    juce::Image background;
    juce::Image tby;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALHReverbAudioProcessorEditor)
};
