/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ALHReverbAudioProcessor::ALHReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "PARAMETERS", createParameterLayout()) // apvts construction
#endif
{
    apvts.addParameterListener("decay", this);
    apvts.addParameterListener("size", this);
    apvts.addParameterListener("predelay", this);
    apvts.addParameterListener("width", this);
    apvts.addParameterListener("gain", this);
    apvts.addParameterListener("drywet", this);
}

ALHReverbAudioProcessor::~ALHReverbAudioProcessor()
{
    apvts.removeParameterListener("decay", this);
    apvts.removeParameterListener("size", this);
    apvts.removeParameterListener("predelay", this);
    apvts.removeParameterListener("width", this);
    apvts.removeParameterListener("gain", this);
    apvts.removeParameterListener("drywet", this);
}

//==============================================================================
const juce::String ALHReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ALHReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ALHReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ALHReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ALHReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ALHReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ALHReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ALHReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ALHReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ALHReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ALHReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ALHReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ALHReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ALHReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    // Audio block object
    juce::dsp::AudioBlock<float> block (buffer);
    
    // DSP block
    for (int channel = 0 ; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);
        
        for (int sample = 0; sample < block.getNumSamples(); ++sample)
        {
            channelData[sample] *= rawGain; // gain rotary5
        }
    }
}

//==============================================================================
bool ALHReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ALHReverbAudioProcessor::createEditor()
{
    return new ALHReverbAudioProcessorEditor (*this);
}

//==============================================================================
void ALHReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save parameters
    juce::MemoryOutputStream stream(destData, false);
    apvts.state.writeToStream(stream);
}

void ALHReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Recall parameters
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    apvts.state = tree;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ALHReverbAudioProcessor();
}


//==== PARAMETERS =========================
juce::AudioProcessorValueTreeState::ParameterLayout ALHReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::NormalisableRange<float> decayRange = juce::NormalisableRange<float>(1.f, 200.f, 1.f);
    decayRange.setSkewForCentre(100.f);

    juce::NormalisableRange<float> sizeRange = juce::NormalisableRange<float>(50.f, 200.f, 0.1f);
    sizeRange.setSkewForCentre(100.0f);

    juce::NormalisableRange<float> predelayRange = juce::NormalisableRange<float>(0.00, 200.0, 0.05f);
    predelayRange.setSkewForCentre(50.0f);
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "decay", 1 }, "Decay", decayRange, 100));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "size", 1 }, "Size", sizeRange, 100.f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "predelay", 1 }, "Predelay", predelayRange, 0.00f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "width", 1 }, "Width", 0.0, 100.0, 100));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "gain", 1 }, "Gain", -24.0, 24.0, 0.0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "drywet", 1 }, "Dry/Wet", 0.0, 100, 50.0));
    

    return { params.begin(), params.end() };
}

void ALHReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "gain")
    {
        rawGain = juce::Decibels::decibelsToGain(newValue);
    }
}
