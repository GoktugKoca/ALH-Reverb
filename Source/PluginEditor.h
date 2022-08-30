/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin editor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


// ======= GUI customization ===================================================
class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Slider::SliderLayout getSliderLayout (juce::Slider&) override
    {
        juce::Slider::SliderLayout layout;
        
        
        layout.sliderBounds = juce::Rectangle<int> (0, 0, 70,70 );
        layout.textBoxBounds = juce::Rectangle<int> (layout.sliderBounds.getX(), layout.sliderBounds.getY() + layout.sliderBounds.getHeight() - 13, layout.sliderBounds.getWidth(), 20);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4AABE1)); // Blue Jeans
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffAE9D7A)); // Grullo
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xffb76a5e)); // Copper
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffAE9D7A));
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentWhite);
        setColour(juce::TooltipWindow::backgroundColourId, juce::Colours::transparentWhite);
        setColour(juce::TooltipWindow::outlineColourId, juce::Colours::transparentWhite);
        setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::black);
        setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
        setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentWhite);
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x1a1a1a));
        setColour(juce::TextEditor::textColourId, juce::Colour(0xffAE9D7A));
        setColour(juce::TextEditor::shadowColourId, juce::Colours::transparentWhite);

        return layout;
    }
};

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
    
    MyLookAndFeel lfM;
    
    juce::TooltipWindow tooltipWindow;
     
    juce::Slider rotary1;
    juce::Slider rotary2;
    juce::Slider rotary3;
    juce::Slider rotary4;
    juce::Slider rotary5;
    juce::Slider rotary6;
    
    juce::Label rotary1Label;
    juce::Label rotary2Label;
    juce::Label rotary3Label;
    juce::Rectangle<int> middleRect;
    
    juce::Label menuLabel { {}, "IR:" };
    juce::Font textFont   { 17.0f };
    juce::ComboBox styleMenu;
    juce::Font labelFont {13.0f};
    
    juce::Image background;
    juce::Image tby;
    
    //===============================================
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> predelaySliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> drywetSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decaySliderAttachment;
    
   
    
    //===============================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALHReverbAudioProcessorEditor)
};
