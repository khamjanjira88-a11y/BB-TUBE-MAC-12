# Implementation status

## Active in this revision
- Integrated hardware-style GUI matching the supplied BB Tube reference layout more closely.
- Functional Input/Output, Threshold, Ratio, Attack, Release, Knee.
- Functional Peak/RMS detector selection.
- Functional Stereo Link and routing modes.
- Functional tube drive, bias and harmonics.
- Functional sidechain HPF/LPF control path.
- Functional lookahead delay path.
- Functional limiter with ceiling clamp and release smoothing.
- Live sample-peak, RMS, gain-reduction, correlation and balance values.
- Live spectrum from the processor FFT FIFO.
- Live waveform from processed audio.
- Live compression curve from threshold/ratio.
- Gain reduction history.
- Factory preset menu with 21 presets.
- A/B snapshot controls.
- macOS Universal 2 and Windows x64 build scripts.

## Important measurement note
The `True Peak` control is exposed, but the current displayed value is sample-peak dBFS/dBTP-style information. A full inter-sample true-peak implementation should use dedicated oversampled peak detection and be validated with reference test vectors before mastering-grade commercial claims.

The LUFS display is a calibrated RMS-derived indication, not a full BS.1770/EBU R128 implementation with K-weighting and gating.
