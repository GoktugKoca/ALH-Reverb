/*
  ==============================================================================

    GUIComponent.h
    Created: 25 Aug 2022 1:05:08am
    Author:  Goktug Koca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
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
           setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(23.f, 115.f, 57.f, 0.f));
           setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffAE9D7A));
           setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentWhite);
           
           return layout;
       }
   };

class GUIComponent  : public juce::Component
{
public:
    GUIComponent();
    ~GUIComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GUIComponent)
    
    MyLookAndFeel lfM;
};
