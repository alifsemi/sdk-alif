## LE Audio Broadcast Source Sample

This sample demonstrates the LE audio broadcast source use-case.

The broadcast name used can be configured using `CONFIG_BROADCAST_NAME`. This might need to be changed for compatibility with a 3rd party broadcast sink device, as the broadcast name is sometimes used to choose which broadcast source to receive from.

Currently the sample only supports mono (single channel audio). Stereo support is planned to be added later.

### Alif B1 FPGA board

On this board, audio support relies on the following additional hardware:
- SI570 clock generator, to generate the MCLK signal. This is connected via a TCA9548 I2C mux.
- WM8904 audio codec, to record audio data and send to the SOC via I2S.
