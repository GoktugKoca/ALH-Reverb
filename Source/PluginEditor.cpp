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
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentWhite);
    
    rotary1.setLookAndFeel(&lfM);
    rotary2.setLookAndFeel(&lfM);
    rotary3.setLookAndFeel(&lfM);
    rotary4.setLookAndFeel(&lfM);
    rotary5.setLookAndFeel(&lfM);
    rotary6.setLookAndFeel(&lfM);
    bypassButton.setLookAndFeel(&lfM);

    bypassButton.setToggleState(false, false);

    bypassButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);
    
    
    rotary1.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag); // vertical? horizontal? ya da her ikisi birden mi?
    rotary2.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag); // e.g., ableton vertical kullanıyor
    rotary3.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    rotary4.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    rotary5.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    rotary6.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    rotary1.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 10);
    rotary2.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 10);
    rotary3.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 10);
    rotary4.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 10);
    rotary5.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 10);
    rotary6.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 10);
    
    rotary1.setRange(0.0, 200.0, 1);
    rotary1.setValue(100);
    rotary1.setTextValueSuffix(" %");
    //    rotary1.setPopupDisplayEnabled(true, true, &rotary1Label);
    rotary1.setPopupMenuEnabled(true);
    rotary1Label.setText("Decay", juce::dontSendNotification);
    rotary1Label.setFont(labelFont);
    rotary1Label.setJustificationType (juce::Justification::centred);
    rotary1Label.setColour(juce::Label::textColourId, juce::Colour(0xffAE9D7A));
    decaySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "decay", rotary1);
    
    
    rotary2.setRange(0.0, 200.0, 1);
    rotary2.setValue(100);
    rotary2.setSkewFactorFromMidPoint(100);
    rotary2.setTextValueSuffix(" %");
    rotary2.setPopupMenuEnabled(true);
    rotary2Label.setText("Size", juce::dontSendNotification);
    rotary2Label.setFont(labelFont);
    rotary2Label.setJustificationType(juce::Justification::centred);
    rotary2Label.setColour(juce::Label::textColourId, juce::Colour(0xffAE9D7A));
    sizeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "size", rotary2);
    
    
    rotary3.setRange(0.00f, 1000.f, 0.01f);
    rotary3.setValue(0.00f);
    rotary3.setTextValueSuffix(" ms");
    rotary3Label.setText("Predelay", juce::dontSendNotification);
    rotary3Label.setFont(labelFont);
    rotary3Label.setJustificationType(juce::Justification::centred);
    rotary3Label.setColour(juce::Label::textColourId, juce::Colour(0xffAE9D7A));
    predelaySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "predelay", rotary3);

    rotary4.setRange(0, 100, 1);
    rotary4.setValue(100);
    rotary4.setTextValueSuffix(" %");
    rotary4Label.setText("Width", juce::dontSendNotification);
    rotary4Label.setFont(labelFont);
    rotary4Label.setJustificationType(juce::Justification::centred);
    rotary4Label.setColour(juce::Label::textColourId, juce::Colour(0xffAE9D7A));
    widthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "width", rotary4);
    
    rotary5.setValue(0.0f);
    rotary5.setTextValueSuffix(" dB");
    rotary5Label.setText("Gain", juce::dontSendNotification);
    rotary5Label.setFont(labelFont);
    rotary5Label.setJustificationType(juce::Justification::centred);
    rotary5Label.setColour(juce::Label::textColourId, juce::Colour(0xffAE9D7A));
    // connection between the rotary and the apvst parameter
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "gain", rotary5);
    
    rotary6.setRange(0.0f, 100.0f, 0.1f);
    rotary6.setValue(50.0f);
    rotary6.setTextValueSuffix(" %");
    rotary6Label.setText("Dry/Wet", juce::dontSendNotification);
    rotary6Label.setFont(labelFont);
    rotary6Label.setJustificationType(juce::Justification::centred);
    rotary6Label.setColour(juce::Label::textColourId, juce::Colour(0xffAE9D7A));
    drywetSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drywet", rotary6);

    
    addAndMakeVisible(rotary1);
    addAndMakeVisible(rotary2);
    addAndMakeVisible(rotary3);
    addAndMakeVisible(rotary4);
    addAndMakeVisible(rotary5);
    addAndMakeVisible(rotary6);

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(reverseButton);

    
    addAndMakeVisible(menuLabel);
    menuLabel.setLookAndFeel(&lfM);
    menuLabel.setFont(textFont);
    addAndMakeVisible (styleMenu);
    styleMenu.addItem ("ALH 1",  1);
    styleMenu.addItem ("ALH 2",   2);
    styleMenu.addItem ("Tasoda", 3);
    styleMenu.setSelectedId (1);

    styleMenu.setLookAndFeel(&lfM);
    
    tooltipWindow.setLookAndFeel(&lfM);
    menuLabel.setTooltip("Impulse Response");
    menuLabel.attachToComponent(&styleMenu, true);
    
    addAndMakeVisible(rotary1Label);
    addAndMakeVisible(rotary2Label);
    addAndMakeVisible(rotary3Label);
    addAndMakeVisible(rotary4Label);
    addAndMakeVisible(rotary5Label);
    addAndMakeVisible(rotary6Label);

    addAndMakeVisible(openIRFileButton);
    openIRFileButton.setButtonText("Open IR File...");
    openIRFileButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(irFileLabel);
    irFileLabel.setText("", juce::dontSendNotification);
    irFileLabel.setJustificationType(juce::Justification::centredLeft);
    juce::TextButton openIRFileButton;
    juce::Label irFileLabel;
    
    setSize (600, 300);
}

ALHReverbAudioProcessorEditor::~ALHReverbAudioProcessorEditor()
{
}

//==============================================================================
void ALHReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);
    
    // Background image
    if (rotary5.getValue() == 3.10f) {
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

    
    const int sliderBorder = 20;
    /*
     juce::Rectangle<int> SlidersArea = getLocalBounds().reduced(sliderBorder);
     juce::Rectangle<int> upperRow = SlidersArea.removeFromTop(SlidersArea.getHeight() / 2);
     
     rotary1.setBounds(upperRow.removeFromLeft(upperRow.getWidth() / 3));
     rotary1Label.setBounds(rotary1.getX(), rotary1.getY() - 14, rotary1.getWidth(), 20 );
     rotary2.setBounds(upperRow.removeFromLeft(upperRow.getWidth() / 2));
     rotary3.setBounds(SlidersArea.removeFromLeft(SlidersArea.getWidth() / 3));
     rotary4.setBounds(SlidersArea.removeFromLeft(SlidersArea.getWidth() / 2));
     rotary5.setBounds(upperRow);
     rotary6.setBounds(SlidersArea);
     */
    
    juce::Rectangle<int> SlidersArea = getLocalBounds().reduced(sliderBorder);
    juce::Rectangle<int> SlidersArea2 = getLocalBounds().reduced(sliderBorder);
    juce::Rectangle<int> leftSlidersArea = SlidersArea.removeFromLeft(SlidersArea.getWidth()/3-120);
    juce::Rectangle<int> rightSlidersArea = SlidersArea2.removeFromRight(SlidersArea2.getWidth()/3-120);
    // juce::Rectangle<int> middleArea = getLocalBounds().reduced(rotary1.getX() + rotary1.getWidth(), sliderBorder);
    
    
    // rotary2Label.setBounds(rotary2.getX(), rotary2.getY() + 20    , rotary2.getWidth(), 10);
    
    rotary1.setBounds(leftSlidersArea.removeFromTop(leftSlidersArea.getHeight()/3));
    rotary2.setBounds(leftSlidersArea.removeFromTop(leftSlidersArea.getHeight()/2));
    rotary3.setBounds(leftSlidersArea);
    rotary4.setBounds(rightSlidersArea.removeFromTop(rightSlidersArea.getHeight()/3));
    rotary5.setBounds(rightSlidersArea.removeFromTop(rightSlidersArea.getHeight()/2));
    rotary6.setBounds(rightSlidersArea);

    openIRFileButton.setBounds(120, 150, 30, 30);
    
    rotary1Label.setBounds(rotary1.getX(), rotary1.getY() - 10, rotary1.getWidth(), 10);
    rotary2Label.setBounds(rotary2.getX(), rotary2.getY() - 10, rotary2.getWidth(), 10);
    rotary3Label.setBounds(rotary3.getX(), rotary3.getY() - 10, rotary3.getWidth(), 10);
    rotary4Label.setBounds(rotary4.getX(), rotary4.getY() - 10, rotary4.getWidth(), 10);
    rotary5Label.setBounds(rotary5.getX(), rotary5.getY() - 10, rotary5.getWidth(), 10);
    rotary6Label.setBounds(rotary6.getX(), rotary6.getY() - 10, rotary6.getWidth(), 10);

    bypassButton.setBounds(120, 205, 60, 20);
    
    styleMenu.setBounds(150,240,320,22);
}




void ALHReverbAudioProcessorEditor::openButtonClicked() {
    fileChooser = std::make_unique<juce::FileChooser>(
        "Choose a support IR File (WAV, AIFF, OGG)...", juce::File(),
        "*.wav;*.aif;*.aiff;*.ogg", true, false);
    formatManager.registerBasicFormats();
    
    auto chooserFlags = juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File()) {
            // update text of IR file label
            irFileLabel.setText(file.getFileName(), juce::dontSendNotification);
            irFileLabel.repaint();

            auto* reader = formatManager.createReaderFor(file);
            if (reader != nullptr) {
                audioProcessor.setIRBufferSize(
                    static_cast<int>(reader->numChannels),
                    static_cast<int>(reader->lengthInSamples));
                reader->read(&audioProcessor.getOriginalIR(), 0,
                    static_cast<int>(reader->lengthInSamples), 0, true, true);
                audioProcessor.loadImpulseResponse();

                shouldPaintWaveform = true;
                enableIRParameters = true;
                reverseButton.setEnabled(enableIRParameters);
                rotary1.setEnabled(enableIRParameters);
                repaint();
            }
        }
        });
}