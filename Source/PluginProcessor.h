/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../soundtouch/SoundTouch.h"


//==============================================================================
/**
*/
class ALHReverbAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ALHReverbAudioProcessor();
    ~ALHReverbAudioProcessor() override;

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
    
    void setIRBufferSize(int newNumChannels, int newNumSamples,
        bool keepExistingContent = false,
        bool clearExtraSpace = false,
        bool avoidReallocating = false);
    juce::AudioBuffer<float>& getOriginalIR();
    juce::AudioBuffer<float>& getModifiedIR();

    void loadImpulseResponse();
    void updateImpulseResponse(juce::AudioBuffer<float> irBuffer);

    void updateIRParameters();

    // Value Trees
    juce::AudioProcessorValueTreeState apvts;

    

private:
    //==============================================================================
   
    juce::AudioBuffer<float> originalIRBuffer;
    juce::AudioBuffer<float> modifiedIRBuffer;

    soundtouch::SoundTouch soundtouch;

    juce::dsp::Gain<float> outputGainer;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    juce::dsp::DelayLine<float> delay;
    juce::dsp::Convolution convolver;

    
    
    
    // Parameters
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALHReverbAudioProcessor)
};
