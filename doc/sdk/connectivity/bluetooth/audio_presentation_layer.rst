##################
Presentation Layer
##################

To distribute and/or consume audio using BLE a presentation layer is
required between the user application and the ISO data path. The main
roles of the presentation layer are:

* To encode/decode raw audio data using the LC3 codec.
* To synchronize the audio playback/recording with the BLE clock.
* To synchronize the audio playback/recording among all the audio sinks/sources.

Typical LE audio applications
=============================

To better understand the responsibilities of the presentation layer,
this section describes some examples of typical LE audio applications,
from the simplest to the most intricate.

Unidirectional, single source, single sink
------------------------------------------

BLE can be used to transmit stereo audio from a source device to a
headset. The source device and headset are connected using a single
unidirectional CIS, containing both the left and right audio channels,
and the audio is retrieved by the source device from a filesystem or
network. In this scenario, no audio recording is therefore performed and
no synchronization is needed among multiple audio sinks or sources.

On the source device, the audio is given to the presentation layer after
having been converted to PCM audio. Here, the only role of the
presentation layer is to encode the PCM audio using LC3 codec and to
provide the encoded audio to the ISO data path. No audio recording is
performed so there is no need to synchronize any audio clock on the
audio source with the BLE clock.

On the headset, the encoded audio is received and is given to the
presentation layer. The latter decodes the audio and provides the
obtained PCM audio to the user application that will typically transmit
it to a hardware audio codec using e.g. I²S. The rate at which audio is
transmitted over I²S and therefore played is defined by the audio clock.
Ideally, this audio clock should be perfectly synchronized with the BLE
clock but in practice, without corrective action, some drift will
inevitably be observed, which means audio will be played a bit faster or
slower than it is received, eventually causing underrun or overrun in
the audio data path. Hence, the presentation layer should make sure the
audio playback is synchronized with the BLE clock, e.g. by adjusting the
audio clock frequency if that is possible or by applying some drift
compensation algorithm on the audio data.

Bidirectional, single source, single sink
-----------------------------------------

This scenario is identical to the previous one except that the source
device is now sending audio that is recorded in real time using a
microphone. The presentation layer on the headset is exactly the same as
above. However, for the source device, it is now required to synchronize
the audio capture with the BLE clock to avoid drift to lead to overrun
or underrun within the audio data path. Similarly to what has been
described for the previous scenario, the presentation layer on the
source device must compensate somehow drift between the audio and BLE
clocks.

Bidirectional, multiple sources, multiple sinks
-----------------------------------------------

The most intricate scenario is when more than two devices are involved
and all devices are producing and consuming audio data. For example,
let's consider the case where a phone is streaming stereo audio to
earbuds and each earbud is recording audio using microphones and sending
it to the phone. As in the previous cases, the presentation layer is
responsible for encoding and decoding the audio packets, in addition to
compensate for any drift between the audio and BLE clocks. However,
since multiple devices are playing and recording audio it is also
necessary here to synchronize the playback and recording across all
devices: the left and right earbuds have to play and capture their
respective audio data at the exact same time. To do so, the Basic Audio
Profile (BAP) specifies a *presentation delay*, identical to all earbuds
and defining the time at which received audio has to be played and
recorded audio has to be sent. This delay is the maximum amount of time
each earbud can take to encode, decode and process audio before it is
played or sent to the phone. The presentation layer on the earbuds has
to buffer the audio in order to play the received audio and send the
recorded audio on time, according to the specified presentation delay.

Design
======

.. image:: /images/alif_ble_audio_presentation_layer.drawio.png

Timestamp synchronization
-------------------------

The timestamp synchronization module is responsible for generating
timestamps in the BLE clock domain. This module is part of the ISO data
path and is needed by the presentation layer to perform presentation
compensation and to measure drift between the audio clock and the BLE
clock. In practice, the timestamp synchronization module uses a
free-running timer in Application core's clock domain and synchronizes regularly
with the controller to determine the current offset between this timer
and the BLE clock. This offset can then be used at any time to convert
the current value of the free-running timer to the corresponding value
in BLE clock domain.

All timestamps are provided in the domain of the local BLE controller's
ISO clock. It is also possible at any time to retrieve the current time
in this clock domain, for example to measure relative time between now
and an SDU timestamp.

Audio sink
----------

For an audio sink, SDUs containing encoded audio data are regularly
received and provided to the presentation layer by the ISO data path.
Every SDU is composed of a single *audio frame*. Each time a new SDU is
received, the presentation layer decodes the audio frame, typically
using the LC3 codec and then determines if some adjustment is needed to
render the audio at the right time, according to the presentation delay.

Decoding & Processing
~~~~~~~~~~~~~~~~~~~~~

The user application is responsible for decoding the incoming audio
data. For most applications, the |**Alif_LC3**| codec will be used but an
application-specific codec might also be used. After decoding, some
audio processing might also be performed by the user application. Doing
audio processing as the first step in the presentation layer makes it
possible to accurately take the extra delay it requires into account for
presentation compensation.


Audio FIFO
~~~~~~~~~~

The audio FIFO is a single-producer single-consumer lock free FIFO,
which can be used to store and retrieve audio data. The audio FIFO
should be sized depending on the presentation delay and the expected
time before an SDU reference anchor point that an SDU may arrive.

When adding data to the FIFO, the user application can request a fixed
size buffer to be reserved in the FIFO. The buffer is the length of a
single audio frame, plus some metadata. The buffer pointer can be
provided as the output buffer to the LC3 codec (or any other codec or
audio processing function), such that it decodes the audio directly into
the FIFO, removing the need to copy data after decoding.

Once decoding and processing is complete, the user application should provide a *DesiredRenderTime* for the audio block (see :ref:`Presentation Compensation<Presentation Compensation - Sink>` section below).
The application should then *commit* the block into the audio FIFO, which makes it available for a consumer to use.

The consumer may retrieve data from the FIFO either in the same
fixed-size blocks that were placed into the FIFO, or in audio
*fragments* of a different length. An audio fragment may not overlap
between two audio blocks, but subject to this condition audio fragments
of any length can be retrieved from the buffer. This may be useful in
the event that the destination for the audio data cannot accept a buffer
size as large as a whole audio block, for example if copying data
directly to the FIFO of an I²S peripheral without using DMA.

.. _Presentation Compensation - Sink:

Presentation Compensation
~~~~~~~~~~~~~~~~~~~~~~~~~

This module is only useful and enabled when audio is rendered in real
time.

Any incoming SDU is timestamped by the controller with the
synchronization reference point *SDUSyncRef* of the SDU. This point can
be used to compute the time *DesiredRenderTime*, in the BLE clock
domain, at which the audio frame must be rendered:

.. math::

   DesiredRenderTime = SDUSyncRef + PresentationDelay

With *PresentationDelay* the presentation delay of the audio stream:

.. image:: /images/alif_ble_audio_presentation_delay.drawio.png

Each time a new audio fragment must be provided to the audio output, the
user application can get this fragment from the FIFO via the
presentation compensation module.

The presentation compensation module retrieves the next audio fragment
from the audio FIFO and then calculates the *PresentationError*, defined
as:

.. math::

   PresentationError = DesiredRenderTime - TimeNow

Where the *DesiredRenderTime* was stored in the FIFO along with the
audio block when it was decoded, and the *TimeNow* is taken from the Time
Synchronisation module in the clock domain of the local BLE controller.
In the case of rendering an audio fragment that is not aligned with a
boundary between audio blocks, the *DesiredRenderTime* has an offset
applied depending on the offset of the audio fragment from the start of
the audio block it is contained within.

This calculation currently assumes that the fragment retrieved from the
audio FIFO will be rendered now. If we assume that the next fragment is
retrieved in the DMA transfer complete ISR of the previous fragment,
then actually the I²S FIFO still contains data, so the next fragment
provided will not start to be rendered until all of these samples have
left the FIFO. The time delay between the DMA transfer complete ISR and
the start of rendering of the next fragment might vary depending on
latency to service the interrupt, and variation in how full the FIFO is
at the point where the DMA transfer completes. This makes estimating the
presentation error challenging.

A PresentationThreshold is defined, which determines the maximum
*PresentationError* for which an audio fragment will be rendered. If the
*PresentationError* would be outside the PresentationThreshold, then
either silence is inserted (by providing an audio fragment consisting of
all zeros to the user application) or samples are dropped to
re-synchronise the playback.

If the *PresentationError* is within the PresentationThreshold but is
still non-zero, then the audio fragment is provided to the output, and
the presentation compensation module attempts to correct the
presentation error by calculating an adjustment to the audio clock. A proportional-integral(PI)
controller is used to calculate the required
audio clock frequency. Over time this PI controller will correct for
both the presentation delay, and any drift between the local bluetooth
clock and the rate at which SDUs are provided by the peer device.

The user application is responsible for actually adjusting the audio
clock frequency depending on the demand from the presentation
compensation module, since the way in which the clock is adjusted will
be application specific.

This solution assumes the size of the audio fragment is variable. This
is useful since this means the presentation compensation block can add
blocks of silence of any size, and remove part of an audio block to
adjust finely the rendering time. However a user could still use fixed
size blocks if desired (e.g. if this is simpler to implement for the
specific destination the audio is sent to).

Audio source
------------

For an audio source, audio data is regularly provided to the
presentation layer by the user application. Audio data is divided into
fixed-size frames, which will generally be 10 ms in length for most LE
audio applications.

Since the size of audio blocks is fixed, if the audio is not recorded in
real time, e.g. if read from a file, the user application might have to
pad the last audio block with silence.

Timing info queue
~~~~~~~~~~~~~~~~~

In the sink direction, the *PresentationError* of an audio packet can be
determined at the point of rendering, by comparing the
*DesiredRenderTime* of the packet with the *TimeNow* at the point of
rendering. Samples may be dropped or added to compensate for any
*PresentationError*.

On the contrary, for the source direction it is not possible to
compensate for any *PresentationError* at the point of sending an SDU to
the data path, since by this point the SDU is encoded and must be
treated as a complete unit which may not be split or delayed (one or
more SDUs must be sent at every ISO event).

So compensation for any *PresentationError* must be applied at the point
of capturing the audio frame. However at the time of capture the
*PresentationError* of the current audio frame is unknown, we can only
know the *PresentationError* of SDUs that have already been sent over the
air by the link layer.

The reference anchor point of the last SDU that was sent over the air
can be retrieved from the ISO data path, along with the associated SDU
sequence number. This information is maintained for each individual data
path instance.

To be able to calculate the *PresentationError*, we must therefore store
the *CaptureTime* of the last few SDUs captured along with the associated
sequence number. This information is stored in a FIFO queue after each
SDU is encoded. Then when the reference anchor point of the last SDU to
be sent over the air is retrieved from the data path, we can search
through the queue to find a matching sequence number and use the
*CaptureTime* along with the *RefAnchor* to calculate the
*PresentationError*. This queue must store the timing information and
associated sequence number of the last few SDUs to be captured rather
than just one, as it is typical for multiple SDUs to be encoded, queued
and ready to send at the point in time where a previous SDU is sent over
the air.

.. _Presentation Compensation - Source:

Presentation Compensation
~~~~~~~~~~~~~~~~~~~~~~~~~

In the audio source direction, the presentation compensation module is
used at the point of audio capture to achieve the desired presentation
delay, and to compensate for drift between clocks.

When it is time to start capture of the next frame of audio data, the
presentation compensation module is first used to determine if any
compensation is required. The presentation compensation module takes as
its inputs the *RefAnchor* of the last SDU to be sent over the air, and
the queue of SDU *CaptureTime*\ s with associated sequence numbers. It
finds the *CaptureTime* of the last SDU to be sent and calculates the
*PresentationError*. Then, depending on the presentation error it can
take one of the following actions:

-  **No action:** if either the clocks are perfectly synchronised
   already, or if there is not enough information to determine what
   action to take (e.g. we are capturing one of the first few frames and
   nothing has been sent over the air yet, so we have no feedback on
   whether the timing is correct.

-  **Add silence:** if the *PresentationError* is a large negative value,
   which means that audio data is being captured later than desired. In
   practice the silence is inserted by filling part of an audio frame
   with zeros, and then filling the remaining frame from the I²S. Since
   the I²S will be capturing fewer samples than a full frame, this
   results in more frames being generated in a given time, allowing the
   generation of frames to “catch up” with the rate at which they are
   sent out over the air.

-  **Drop samples**: if the *PresentationError* is a large positive
   value, which means that audio data is being captured earlier than
   desired. In practice the samples are dropped by receiving a number of
   samples over I²S into a buffer, but then not sending the buffer to
   the next stage of the presentation layer and instead overwriting the
   samples with the next I²S receive operation. This could also be
   achieved by simply scheduling a timer callback for some time in the
   future to start filling the frame, but this would require a timer
   channel in addition to the I²S peripheral.

-  **Adjust audio PLL**: if the *PresentationError* is within some
   margin. No samples are added or dropped, and any *PresentationError*
   is compensated for by adjusting the audio PLL to speed up or slow
   down audio capture. The desired frequency of the audio clock is
   determined using a PI controller which aims to minimise
   *PresentationError* in the same way as for the sink use-case.

.. _Audio FIFO - Source:

Audio FIFO
~~~~~~~~~~

Each frame is captured directly into a buffer which is part of the audio
FIFO. The *CaptureTime* is also written into the audio FIFO. Once the
frame capture is complete, the buffer can be committed to the FIFO. A
semaphore is used to indicate to the thread running the LC3 encoder that
a new audio frame is ready to be encoded.

Processing & Encoding
~~~~~~~~~~~~~~~~~~~~~

Whenever a new audio frame has been captured, this is encoded using the
LC3 codec. Any additional audio processing could be performed before the
encoder is run.

SDU FIFO
~~~~~~~~

After encoding, SDUs are added to a FIFO. When the previous SDU has been
transferred to the data path, the next SDU can be pulled out of the FIFO
and sent. If there is no SDU available to be sent at the time when the
previous SDU has completed transfer to the link layer, then the next SDU
can be sent later (it can be sent directly as soon as encoding is
complete, rather than adding to the FIFO). However it must be ensured
that the SDU has been transferred to the link layer before the next ISO
event, otherwise the link layer will be forced to send an empty SDU at
this event.

It would be possible to operate the presentation layer without an audio
FIFO in the event that the chosen presentation delay and encoding
latency mean that it is not required. For example if the presentation
delay is 20 ms, frames are 10 ms long, and it takes 5 ms to encode a
frame then it is possible to send each encoded SDU to the link layer 15
ms after capture, to be sent 20 ms after capture, and the link layer's
SDU buffer will be free again before the next SDU is ready at 25 ms
after the original SDU's capture time.

However adding an SDU FIFO allows more flexibility in choosing the
presentation delay.
