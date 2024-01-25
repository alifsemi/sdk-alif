## LE Audio Broadcast Source Sample

This sample acts as an LE audio broadcast source. Since the relevant drivers to receive audio data over I2S are not yet in place, the app simply broadcasts a constant 400 Hz tone on a single channel. This is enough to prove out the ISO data path and LC3 codec on Zephyr. It will be extended later to add in the "real" audio received over I2S.
