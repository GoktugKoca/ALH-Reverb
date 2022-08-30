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
    apvts.addParameterListener("gain", this);
}

ALHReverbAudioProcessor::~ALHReverbAudioProcessor()
{
    apvts.removeParameterListener("gain", this);
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
    // pre-initialization process
    juce::dsp::ProcessSpec spec = juce::dsp::ProcessSpec();
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    spec.maximumBlockSize = samplesPerBlock;

    soundtouch.setSampleRate(sampleRate);
    soundtouch.setChannels(1);

    outputGainer.prepare(spec);
    outputGainer.reset();
    dryWetMixer.prepare(spec);
    dryWetMixer.reset();
    delay.prepare(spec);
    delay.setMaximumDelayInSamples(sampleRate);
    delay.reset();
    convolver.prepare(spec);
    convolver.reset();
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

    
    auto outputGainValue = apvts.getRawParameterValue("gain");
    auto dryWetMixValue = apvts.getRawParameterValue("drywet");
    auto preDelayTimeValue = apvts.getRawParameterValue("predelay");
    auto stereoWidthValue = apvts.getRawParameterValue("width");
    auto isBypassed = apvts.getRawParameterValue("bypass");

    if (isBypassed->load() == true) {
        return;
    }

    outputGainer.setGainDecibels(outputGainValue->load());
    dryWetMixer.setWetMixProportion(dryWetMixValue->load() / 100.0f);
    delay.setDelay(preDelayTimeValue->load() / 1000.0 * this->getSampleRate());

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    dryWetMixer.pushDrySamples(block);
    convolver.process(context);
    delay.process(context);

    // set stereo width using mid/side technique
    if (context.getInputBlock().getNumChannels() == 2) {
        const float width = stereoWidthValue->load() / 100.0;
        for (int sample = 0; sample < context.getInputBlock().getNumSamples();
            ++sample) {
            float left = context.getInputBlock().getSample(0, sample);
            float right = context.getInputBlock().getSample(1, sample);
            context.getOutputBlock().setSample(
                0, sample, left * (1 + width) / 2 + right * (1 - width) / 2);
            context.getOutputBlock().setSample(
                1, sample, left * (1 - width) / 2 + right * (1 + width) / 2);
        }
    }

    dryWetMixer.mixWetSamples(block);
    outputGainer.process(context);

    
    // Audio block object
    // juce::dsp::AudioBlock<float> block (buffer);
    
    // DSP block
   
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

void ALHReverbAudioProcessor::setIRBufferSize(int newNumChannels,
    int newNumSamples,
    bool keepExistingContent,
    bool clearExtraSpace,
    bool avoidReallocating) {
    originalIRBuffer.setSize(newNumChannels, newNumSamples, keepExistingContent,
        clearExtraSpace, avoidReallocating);
}

juce::AudioBuffer<float>& ALHReverbAudioProcessor::getOriginalIR() {
    return originalIRBuffer;
}

juce::AudioBuffer<float>& ALHReverbAudioProcessor::getModifiedIR() {
    return modifiedIRBuffer;
}

void ALHReverbAudioProcessor::loadImpulseResponse() {
    // normalized IR signal
    float globalMaxMagnitude =
        originalIRBuffer.getMagnitude(0, originalIRBuffer.getNumSamples());
    originalIRBuffer.applyGain(1.0f / (globalMaxMagnitude + 0.01));

    // trim IR signal
    int numSamples = originalIRBuffer.getNumSamples();
    int blockSize = static_cast<int>(std::floor(this->getSampleRate()) / 100);
    int startBlockNum = 0;
    int endBlockNum = numSamples / blockSize;
    float localMaxMagnitude = 0.0f;
    while ((startBlockNum + 1) * blockSize < numSamples) {
        localMaxMagnitude =
            originalIRBuffer.getMagnitude(startBlockNum * blockSize, blockSize);
        // find the start position of IR
        if (localMaxMagnitude > 0.001) {
            break;
        }
        ++startBlockNum;
    }
    localMaxMagnitude = 0.0f;
    while ((endBlockNum - 1) * blockSize > 0) {
        --endBlockNum;
        localMaxMagnitude =
            originalIRBuffer.getMagnitude(endBlockNum * blockSize, blockSize);
        // find the time to decay by 60 dB (T60)
        if (localMaxMagnitude > 0.001) {
            break;
        }
    }

    int trimmedNumSamples;
    if (endBlockNum * blockSize < numSamples) {
        trimmedNumSamples = (endBlockNum - startBlockNum) * blockSize - 1;
    }
    else {
        trimmedNumSamples = numSamples - startBlockNum * blockSize;
    }
    modifiedIRBuffer.setSize(originalIRBuffer.getNumChannels(), trimmedNumSamples,
        false, true, false);
    for (int channel = 0; channel < originalIRBuffer.getNumChannels();
        ++channel) {
        for (int sample = 0; sample < trimmedNumSamples; ++sample) {
            modifiedIRBuffer.setSample(
                channel, sample,
                originalIRBuffer.getSample(channel,
                    sample + startBlockNum * blockSize));
        }
    }

    originalIRBuffer.makeCopyOf(modifiedIRBuffer);

    auto decayTimeParam = apvts.getParameter("decay");
    double decayTime =
        static_cast<double>(trimmedNumSamples) / this->getSampleRate();
    decayTimeParam->beginChangeGesture();
    decayTimeParam->setValueNotifyingHost(
        decayTimeParam->convertTo0to1(decayTime));
    decayTimeParam->endChangeGesture();

    updateImpulseResponse(modifiedIRBuffer);
}

void ALHReverbAudioProcessor::updateImpulseResponse(
    juce::AudioBuffer<float> irBuffer) {
    convolver.loadImpulseResponse(std::move(irBuffer), this->getSampleRate(),
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::no,
        juce::dsp::Convolution::Normalise::yes);
}

void ALHReverbAudioProcessor::updateIRParameters() {
    if (originalIRBuffer.getNumSamples() < 1) {
        return;
    }

    // stretch IR according to decay time
    auto decayTimeValue = apvts.getRawParameterValue("decay");
    int decaySample = static_cast<int>(
        std::round(decayTimeValue->load() * this->getSampleRate()));
    double stretchRatio =
        originalIRBuffer.getNumSamples() / static_cast<double>(decaySample);

    int numChannels = originalIRBuffer.getNumChannels();
    soundtouch.setTempo(stretchRatio);
    modifiedIRBuffer.setSize(numChannels, decaySample, false, true, false);
    for (int channel = 0; channel < numChannels; ++channel) {
        soundtouch.putSamples(originalIRBuffer.getReadPointer(channel),
            originalIRBuffer.getNumSamples());
        soundtouch.receiveSamples(modifiedIRBuffer.getWritePointer(channel),
            decaySample);
        soundtouch.clear();
    }

    // reverse of the IR
    auto isReversed = apvts.getRawParameterValue("reverse");
    if (isReversed->load() == true) {
        modifiedIRBuffer.reverse(0, modifiedIRBuffer.getNumSamples());
    }

    updateImpulseResponse(modifiedIRBuffer);
}




//==== PARAMETERS =========================
juce::AudioProcessorValueTreeState::ParameterLayout ALHReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::NormalisableRange<float> decayRange = juce::NormalisableRange<float>(0.10f, 8.f, 0.01f);
    decayRange.setSkewForCentre(3.f);

    juce::NormalisableRange<float> sizeRange = juce::NormalisableRange<float>(50.f, 200.f, 0.1f);
    sizeRange.setSkewForCentre(100.0f);

    juce::NormalisableRange<float> predelayRange = juce::NormalisableRange<float>(0.0f, 1000.0f, 1.0f, 0.5f);

    juce::NormalisableRange<float> widthRange = juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f);

    juce::NormalisableRange<float> gainRange = juce::NormalisableRange<float>(-72.0f, 36.0f, 0.1f);
    gainRange.setSkewForCentre(0.0f);
    
    juce::NormalisableRange<float> drywetRange = juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f);
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "decay", 1 }, "Decay", decayRange, 3.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "size", 1 }, "Size", sizeRange, 100.f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "predelay", 1 }, "Predelay", predelayRange, 0.00f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "width", 1 }, "Width", widthRange, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "gain", 1 }, "Gain", gainRange, 0.0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "drywet", 1 }, "Dry/Wet", drywetRange, 100.0));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "reverse", 1 }, "Reverse", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "bypass", 1 }, "Bypass", false));
    

    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ALHReverbAudioProcessor();
}


void ALHReverbAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
  

}