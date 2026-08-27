# BB Tube Compressor — GUI / DSP status

The supplied GUI reference image is stored at `Resources/BB_Tube_Compressor_GUI_Reference.jpeg` and is used as a visual design reference. The plugin GUI itself is rendered by JUCE code; it does not load the reference screenshot at runtime.

Implemented in the current source:
- Integrated single-panel hardware-style GUI
- Input/output/threshold/ratio/attack/release/knee
- Tube drive/bias/harmonics controls
- Peak/RMS detector switch
- Stereo Link
- Stereo/Mono/Mid-Side/Dual-Mono routing
- Sidechain HPF control
- Limiter threshold/ceiling/release
- Lookahead control
- True Peak control parameter
- Input/output peak metering
- Gain-reduction metering/history
- Spectrum display fed from audio processing
- Waveform display fed from processed blocks
- Compression transfer curve
- Stereo correlation and balance
- Loudness readouts
- Factory preset menu

Engineering note:
- The current source is functional and intended for compilation/testing.
- The loudness readout is RMS-derived and is not a complete ITU-R BS.1770 gated LUFS meter.
- The current true-peak display is sample-peak based rather than a validated inter-sample true-peak implementation.
- The lookahead and oversampling controls are present; production mastering release should be validated with automated reference vectors and dedicated oversampled detector/output stages.
