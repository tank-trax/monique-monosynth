/*
** Monique is Free and Open Source Software
**
** Monique is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html; The authors of the code
** reserve the right to re-license their contributions under the MIT license in the
** future at the discretion of the project maintainers.
**
** Copyright 2016-2022 by various individuals as described by the git transaction log
**
** All source at: https://github.com/surge-synthesizer/monique-monosynth.git
**
** Monique was a commercial product from 2016-2021, with copyright and ownership
** in that period held by Thomas Arndt at Monoplugs. Thomas made Monique
** open source in December 2021.
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "App.h"
#include "mono_AudioDeviceManager.h"

class MIDIControlHandler;
struct MoniqueSynthData;
class Monique_Ui_SegmentedMeter;
class Monique_Ui_Refresher;
class MoniqueSynthesiserVoice;
class ClockSmoothBuffer;
class MoniqueSynthesizer;
class Monique_Ui_AmpPainter;
class Monique_Ui_Mainwindow;

class MoniqueAudioProcessor : public juce::AudioProcessor,
                              public juce::MidiKeyboardState,
                              public mono_AudioDeviceManager,
                              public ParameterListener
{
    struct standalone_features;
    std::unique_ptr<standalone_features> standalone_features_pimpl;

    int stored_note;
    float stored_velocity;

  public:
    int instance_id;

    // ==============================================================================
    // DATA & SYNTH PROCESSOR
  public:
    MoniqueSynthData *synth_data;
    MoniqueSynthData *get_synth_data() noexcept override { return synth_data; }
    MoniqueSynthesiserVoice *voice;
    MoniqueSynthesizer *synth;

    // ==============================================================================
    // UI
  public:
    Monique_Ui_SegmentedMeter *peak_meter;
    juce::CriticalSection peak_meter_lock;

  public:
    void set_peak_meter(Monique_Ui_SegmentedMeter *peak_meter_) noexcept;
    void clear_peak_meter() noexcept;

  private:
    // ==============================================================================
    // PROCESS
  public:
    juce::AudioPlayHead::CurrentPositionInfo current_pos_info;

  private:
    bool force_sample_rate_update;
    void processBlock(juce::AudioSampleBuffer &buffer_, juce::MidiBuffer &midi_messages_) override;
    void processBlockBypassed(juce::AudioSampleBuffer &buffer_,
                              juce::MidiBuffer &midi_messages_) override;
    void process(juce::AudioSampleBuffer &buffer_, juce::MidiBuffer &midi_messages_,
                 bool bypassed_);
    COLD void sample_rate_or_block_changed() noexcept override;
    COLD void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    COLD void releaseResources() override;
    COLD void reset() override;

  public:
    COLD void reset_pending_notes();

    inline const juce::AudioPlayHead::CurrentPositionInfo &get_current_pos_info() const noexcept
    {
        return current_pos_info;
    }

  private:
    //==========================================================================
    // MIDI KEYBOARD
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader *sampleReader;
    int samplePosition;
    juce::AudioSampleBuffer sampleBuffer;
    std::int64_t lastBlockTime;
    int blockTimeCheckCounter;

  private:
    // ==============================================================================
    /// AUTOMATION PARAMETERS
    juce::Array<Parameter *> automateable_parameters;
    void init_automatable_parameters() noexcept;

    int getNumParameters() override;
    bool isParameterAutomatable(int parameterIndex) const override;
    float getParameter(int index_) override;
    void setParameter(int index_, float value_) override;
    const juce::String getParameterName(int index_) override;
    const juce::String getParameterText(int index_) override;
    juce::String getParameterLabel(int index) const override;
    int getParameterNumSteps(int index_) override;
    float getParameterDefaultValue(int index_) override;
    bool isMetaParameter(int parameterIndex) const override;

    void parameter_value_changed(Parameter *) noexcept override;
    void parameter_value_changed_always_notification(Parameter *) noexcept override;
    void parameter_value_on_load_changed(Parameter *) noexcept override;
    void parameter_modulation_value_changed(Parameter *) noexcept override;

    //==========================================================================
    // LOAD SAVE
    std::int64_t restore_time;
    void getStateInformation(juce::MemoryBlock &dest_data_) override;
    void setStateInformation(const void *data_, int size_in_bytes_) override;

    //==========================================================================
    // CONFIG
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool isInputChannelStereoPair(int index_) const override;
    bool isOutputChannelStereoPair(int index_) const override;
    bool isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout &layouts) const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //==========================================================================
    // PROGRAM HANDLING (PLUGIN ONLY)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index_) override;
    const juce::String getProgramName(int index_) override;
    void changeProgramName(int index_, const juce::String &name_) override;

    //==========================================================================
    // BOOT UI
    COLD juce::AudioProcessorEditor *createEditor() override;

    // GET UI
    Monique_Ui_Mainwindow *get_editor() noexcept override
    {
        return reinterpret_cast<Monique_Ui_Mainwindow *>(getActiveEditor());
    }

    //==========================================================================
    // GLOBAL CLASSES
  public:
    std::unique_ptr<UiLookAndFeel> ui_look_and_feel;
    std::unique_ptr<MIDIControlHandler> midi_control_handler;
    std::unique_ptr<Monique_Ui_Refresher> ui_refresher;
    std::unique_ptr<DataBuffer> data_buffer;
    std::unique_ptr<RuntimeInfo> info;
    Monique_Ui_AmpPainter *amp_painter;

    // keeps the shared data singleton alive
    std::shared_ptr<Status> scoped_shared_global_settings;
    std::shared_ptr<ENVData> scoped_shared_ENV_clipboard;
    std::shared_ptr<LFOData> scoped_shared_LFO_clipboard;
    std::shared_ptr<ENV_clipboard_has_data> scoped_shared_ENV_clipboard_has_data;
    std::shared_ptr<LFO_clipboard_has_data> scoped_shared_LFO_clipboard_has_data;

  protected:
    //==========================================================================
    friend juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter();
    friend struct juce::ContainerDeletePolicy<MoniqueAudioProcessor>;
    COLD MoniqueAudioProcessor() noexcept;
    COLD ~MoniqueAudioProcessor() noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoniqueAudioProcessor)
};

#endif // PLUGINPROCESSOR_H_INCLUDED
