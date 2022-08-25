/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GUIComponent.h"


//==============================================================================

 void prepareAll (const juce::dsp::ProcessSpec& spec, Processors&... processors)
 {
     forEach ([&] (auto& proc) { proc.prepare (spec); }, processors...);
 }

class ALHReverbAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ALHReverbAudioProcessor();
    ~ALHReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        const auto channels = jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());

        if (channels == 0)
            return;

        chain.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) channels });

        reset();
    }

    void reset() override
    {
        chain.reset();
        update();
    }
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts == BusesLayout { {juce::AudioChannelSet::stereo() },
            { juce::AudioChannelSet::stereo() } };
    }

   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override
    {
        if (jmax (getTotalNumInputChannels(), getTotalNumOutputChannels()) == 0)
            return;

        juce::ScopedNoDenormals noDenormals;

        if (requiresUpdate.load())
            update();

        irSize = dsp::get<convolutionIndex> (chain).reverb.getCurrentIRSize();

        const auto totalNumInputChannels  = getTotalNumInputChannels();
        const auto totalNumOutputChannels = getTotalNumOutputChannels();

        setLatencySamples (dsp::get<convolutionIndex> (chain).getLatency()
                           + (dsp::isBypassed<distortionIndex> (chain) ? 0 : roundToInt (dsp::get<distortionIndex> (chain).getLatency())));

        const auto numChannels = jmax (totalNumInputChannels, totalNumOutputChannels);

        auto inoutBlock = dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
        chain.process (dsp::ProcessContextReplacing<float> (inoutBlock));
    }
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override {}

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override {return nullptr;}
    bool hasEditor() const override {return false; }

    //==============================================================================
    const juce::String getName() const override {return "ALH Reverb"; }

    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override {return 0; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override
    {
        copyXmlToBinary (*apvts.copyState().createXml(), destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        apvts.replaceState (ValueTree::fromXml (*getXmlFromBinary (data, sizeInBytes)));
    }

    int getCurrentIRSize() const { return irSize; }

    using Parameter = juce::AudioProcessorValueTreeState::Parameter;
    using Attributes = juce::AudioProcessorValueTreeStateParameterAttributes;
    
struct ParameterReferences
    {
        template <typename Param>
        static void add (juce::AudioProcessorParameterGroup& group, std::unique_ptr<Param> param)
        {
            group.addChild (std::move (param));
        }

        template <typename Param>
        static void add (juce::AudioProcessorValueTreeState::ParameterLayout& group, std::unique_ptr<Param> param)
        {
            group.add (std::move (param));
        }

        template <typename Param, typename Group, typename... Ts>
        static Param& addToLayout (Group& layout, Ts&&... ts)
        {
            auto param = new Param (std::forward<Ts> (ts)...);
            auto& ref = *param;
            add (layout, rawToUniquePtr (param));
            return ref;
        }

        static juce::String valueToTextFunction (float x, int) { return String (x, 2); }
        static float textToValueFunction (const juce::String& str) { return str.getFloatValue(); }

        static auto getBasicAttributes()
        {
            return Attributes().withStringFromValueFunction (valueToTextFunction)
                               .withValueFromStringFunction (textToValueFunction);
        }

        static auto getDbAttributes()           { return getBasicAttributes().withLabel ("dB"); }
        static auto getMsAttributes()           { return getBasicAttributes().withLabel ("ms"); }
        static auto getHzAttributes()           { return getBasicAttributes().withLabel ("Hz"); }
        static auto getPercentageAttributes()   { return getBasicAttributes().withLabel ("%"); }
        static auto getRatioAttributes()        { return getBasicAttributes().withLabel (":1"); }

        static juce::String valueToTextPanFunction (float x, int) { return getPanningTextForValue ((x + 100.0f) / 200.0f); }
        static float textToValuePanFunction (const juce::String& str) { return getPanningValueForText (str) * 200.0f - 100.0f; }

        struct MainGroup
        {
            explicit MainGroup (juce::AudioProcessorParameterGroup& layout)
                : inputGain (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::inputGain, 1 },
                                                     "Input",
                                                     NormalisableRange<float> (-40.0f, 40.0f),
                                                     0.0f,
                                                     getDbAttributes())),
                  outputGain (addToLayout<Parameter> (layout,
                                                      ParameterID { ID::outputGain, 1 },
                                                      "Output",
                                                      NormalisableRange<float> (-40.0f, 40.0f),
                                                      0.0f,
                                                      getDbAttributes())),
                  pan (addToLayout<Parameter> (layout,
                                               ParameterID { ID::pan, 1 },
                                               "Panning",
                                               NormalisableRange<float> (-100.0f, 100.0f),
                                               0.0f,
                                               Attributes().withStringFromValueFunction (valueToTextPanFunction)
                                                           .withValueFromStringFunction (textToValuePanFunction))) {}

            Parameter& inputGain;
            Parameter& outputGain;
            Parameter& pan;
        };
        
        struct ConvolutionGroup
        {
            explicit ConvolutionGroup (juce::AudioProcessorParameterGroup& layout)
                : cabEnabled (addToLayout<AudioParameterBool> (layout,
                                                               ParameterID { ID::convolutionCabEnabled, 1 },
                                                               "Cabinet",
                                                               false)),
                  reverbEnabled (addToLayout<AudioParameterBool> (layout,
                                                                  ParameterID { ID::convolutionReverbEnabled, 1 },
                                                                  "Reverb",
                                                                  false)),
                  reverbMix (addToLayout<Parameter> (layout,
                                                     ParameterID { ID::convolutionReverbMix, 1 },
                                                     "Reverb Mix",
                                                     NormalisableRange<float> (0.0f, 100.0f),
                                                     50.0f,
                                                     getPercentageAttributes())) {}

            juce::AudioParameterBool& cabEnabled;
            juce::AudioParameterBool& reverbEnabled;
            Parameter& reverbMix;
        };
        MainGroup main;
        
        ConvolutionGroup convolution;
    };
    
    const ParameterReferences& getParameterValues() const noexcept { return parameters; }

    //==============================================================================
    // We store this here so that the editor retains its state if it is closed and reopened
    int indexTab = 0;
        
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALHReverbAudioProcessor)
    void update()
    {

        {
            ConvolutionProcessor& convolution = dsp::get<convolutionIndex> (chain);
            convolution.cabEnabled    = parameters.convolution.cabEnabled;
            convolution.reverbEnabled = parameters.convolution.reverbEnabled;
            convolution.mixer.setWetMixProportion (parameters.convolution.reverbMix.get() / 100.0f);
        }

        juce::dsp::get<inputGainIndex>  (chain).setGainDecibels (parameters.main.inputGain.get());
        juce::dsp::get<outputGainIndex> (chain).setGainDecibels (parameters.main.outputGain.get());
        juce::dsp::get<pannerIndex> (chain).setPan (parameters.main.pan.get() / 100.0f);

        requiresUpdate.store (false);
    }
    
    static juce::String getPanningTextForValue (float value)
    {
        if (value == 0.5f)
            return "center";

        if (value < 0.5f)
            return String (roundToInt ((0.5f - value) * 200.0f)) + "%L";

        return String (roundToInt ((value - 0.5f) * 200.0f)) + "%R";
    }

    static float getPanningValueForText (juce::String strText)
    {
        if (strText.compareIgnoreCase ("center") == 0 || strText.compareIgnoreCase ("c") == 0)
            return 0.5f;

        strText = strText.trim();

        if (strText.indexOfIgnoreCase ("%L") != -1)
        {
            auto percentage = (float) strText.substring (0, strText.indexOf ("%")).getDoubleValue();
            return (100.0f - percentage) / 100.0f * 0.5f;
        }

        if (strText.indexOfIgnoreCase ("%R") != -1)
        {
            auto percentage = (float) strText.substring (0, strText.indexOf ("%")).getDoubleValue();
            return percentage / 100.0f * 0.5f + 0.5f;
        }

        return 0.5f;
    }
    
    struct ConvolutionProcessor
    {
        ConvolutionProcessor()
        {
            loadImpulseResponse (cabinet, "guitar_amp.wav");
            loadImpulseResponse (reverb,  "reverb_ir.wav");
            mixer.setMixingRule (juce::::DryWetMixingRule::balanced);
        }

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            prepareAll (spec, cabinet, reverb, mixer);
        }

        void reset()
        {
            resetAll (cabinet, reverb, mixer);
        }

        template <typename Context>
        void process (Context& context)
        {
            auto contextConv = context;
            contextConv.isBypassed = (! cabEnabled) || context.isBypassed;
            cabinet.process (contextConv);

            if (cabEnabled)
                context.getOutputBlock().multiplyBy (4.0f);

            if (reverbEnabled)
                mixer.pushDrySamples (context.getInputBlock());

            contextConv.isBypassed = (! reverbEnabled) || context.isBypassed;
            reverb.process (contextConv);

            if (reverbEnabled)
            {
                const auto& outputBlock = context.getOutputBlock();
                outputBlock.multiplyBy (4.0f);
                mixer.mixWetSamples (outputBlock);
            }
        }

        int getLatency() const
        {
            auto latency = 0;

            if (cabEnabled)
                latency += cabinet.getLatency();

            if (reverbEnabled)
                latency += reverb.getLatency();

            return latency;
        }

        juce::dsp::ConvolutionMessageQueue queue;
        juce::dsp::Convolution cabinet { juce::dsp::Convolution::NonUniform { 512 }, queue };
        juce::dsp::Convolution reverb { juce::dsp::Convolution::NonUniform { 512 }, queue };
        juce::dsp::DryWetMixer<float> mixer;
        bool cabEnabled = false, reverbEnabled = false;

    private:
        static void loadImpulseResponse (juce::dsp::Convolution& convolution, const char* filename)
        {
            auto stream = createAssetInputStream (filename);

            if (stream == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioFormatManager manager;
            manager.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> reader { manager.createReaderFor (std::move (stream)) };

            if (reader == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioBuffer<float> buffer (static_cast<int> (reader->numChannels),
                                       static_cast<int> (reader->lengthInSamples));
            reader->read (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, buffer.getNumSamples());

            convolution.loadImpulseResponse (std::move (buffer),
                                             reader->sampleRate,
                                             dsp::Convolution::Stereo::yes,
                                             dsp::Convolution::Trim::yes,
                                             dsp::Convolution::Normalise::yes);
        }
    };
    ParameterReferences parameters;
    juce::AudioProcessorValueTreeState apvts;

    using Chain = juce::dsp::ProcessorChain<juce::dsp::NoiseGate<float>,
    juce::dsp::Gain<float>,
    ConvolutionProcessor;
                                      
    Chain chain;
};
