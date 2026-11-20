/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "LiveMicInput.h"
#include "mic_source.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <cerrno>
#include <cstring>

/* 80 dB. Gain is the multiplier applied to a mic sample. */
static constexpr float kMaxGain = 10000.0f;
/* +1 dB once per half-window, which is +2 dB per second. */
static constexpr float kMaxGainIncPerStride = 1.12201845f;
static constexpr int32_t kInt16Max = 32767;
static constexpr int32_t kInt16Min = -32768;

/* PDM frames delivered to the callback are interleaved stereo. */
static constexpr size_t kMicChannels = 2;

/* Mic frames arrive at the LE Audio rate. The model window is 16 kHz, so each
 * block of kDecimation mic samples becomes one model sample.
 */
static constexpr size_t kCaptureRate = CONFIG_ALIF_BLE_AUDIO_FS_HZ;
static constexpr size_t kModelRate = static_cast<size_t>(arm::app::kws::g_AudioRate);
static_assert(kCaptureRate >= kModelRate, "Mic rate is below the KWS model rate");
static_assert(kCaptureRate % kModelRate == 0, "Mic rate is not an integer multiple of the KWS rate");
static constexpr size_t kDecimation = kCaptureRate / kModelRate;

LOG_MODULE_REGISTER(LiveMicInput, CONFIG_MAIN_LOG_LEVEL);

extern "C" {
static int mic_data_callback(const void *data, size_t size, void *user_data)
{
	if (!data || size == 0 || !user_data) {
		return -EINVAL;
	}

	LiveMicInput *this_class = static_cast<LiveMicInput *>(user_data);
	this_class->data_received(data, size);
	return 0;
}
}

void LiveMicInput::push_model_sample(int16_t sample)
{
	if (m_filled < m_buffer_size) {
		m_audio_input_buffer[m_filled++] = sample;
		return;
	}

	if (m_pending < kPendingCapacity) {
		m_pending_samples[m_pending++] = sample;
		return;
	}

	static int64_t last_log;
	const int64_t now = k_uptime_get();

	if (now - last_log > 1000) {
		LOG_WRN("Mic buffer overrun, dropped 1 sample");
		last_log = now;
	}
}

void LiveMicInput::drain_pending(void)
{
	size_t copied = 0;

	while (copied < m_pending && m_filled < m_buffer_size) {
		m_audio_input_buffer[m_filled++] = m_pending_samples[copied++];
	}

	m_pending -= copied;
	if (m_pending > 0) {
		memmove(m_pending_samples, m_pending_samples + copied, m_pending * sizeof(int16_t));
	}
}

LiveMicInput::LiveMicInput(size_t const buffer_size)
	: m_buffer_size(buffer_size), m_filled(0), m_pending(0), m_decim_count(0),
	  m_samples_since_gain_rise(0), m_decim_sum(0), m_primed(false), m_stopped(false),
	  m_current_gain(kMaxGain)
{
	k_sem_init(&m_data_ready, 0, 1);
	k_mutex_init(&m_lock);

	/* The window is stored in place. Only half of it is captured per call. */
	if (buffer_size < 2 || buffer_size > kAudioCapacity) {
		LOG_ERR("Audio window %zu samples does not fit in %zu", buffer_size, kAudioCapacity);
		m_buffer_size = 0;
		return;
	}

	memset(m_audio_input_buffer, 0, m_buffer_size * sizeof(int16_t));
	m_filled = m_buffer_size / 2;
}

bool LiveMicInput::Start(void)
{
	k_mutex_lock(&m_lock, K_FOREVER);
	m_stopped = false;
	m_primed = false;
	m_pending = 0;
	m_decim_count = 0;
	m_decim_sum = 0;
	m_samples_since_gain_rise = 0;
	m_current_gain = kMaxGain;
	if (m_buffer_size >= 2) {
		/* Older half is silence until the second capture slides real audio in. */
		memset(m_audio_input_buffer, 0, m_buffer_size * sizeof(int16_t));
		m_filled = m_buffer_size / 2;
	} else {
		m_filled = 0;
	}
	k_mutex_unlock(&m_lock);
	k_sem_reset(&m_data_ready);

	/* Microphone frames are delivered here as they are captured. */
	return mic_register_callback(mic_data_callback, (void *)this) == 0;
}

bool LiveMicInput::Stop(void)
{
	k_mutex_lock(&m_lock, K_FOREVER);
	m_stopped = true;
	k_mutex_unlock(&m_lock);
	k_sem_give(&m_data_ready);

	return mic_register_callback((mic_data_callback_t)0, NULL) == 0;
}

float LiveMicInput::adjust_dynamic_gain(const int16_t *samples, size_t count)
{
	/* Peak magnitude of the left channel. int32 so -32768 does not overflow. */
	int32_t peak = 0;

	for (size_t i = 0; i < count; i++) {
		const int32_t value = samples[i * kMicChannels];
		const int32_t magnitude = value < 0 ? -value : value;

		if (magnitude > peak) {
			peak = magnitude;
		}
	}

	float target = kMaxGain;

	if (peak > 0) {
		const float full_scale_gain =
			static_cast<float>(kInt16Max) / static_cast<float>(peak);

		if (full_scale_gain < target) {
			target = full_scale_gain;
		}
	}
	const float previous = m_current_gain;

	if (target < m_current_gain) {
		m_current_gain = target;
		m_samples_since_gain_rise = 0;
	} else {
		const size_t samples_per_stride = (kModelRate / 2) * kDecimation;

		m_samples_since_gain_rise += count;
		if (m_samples_since_gain_rise >= samples_per_stride) {
			m_samples_since_gain_rise -= samples_per_stride;
			const float raised = m_current_gain * kMaxGainIncPerStride;

			m_current_gain = raised < target ? raised : target;
		}
	}

	if (m_current_gain != previous) {
		LOG_DBG("Gain: %f", static_cast<double>(m_current_gain));
	}

	return m_current_gain;
}

void LiveMicInput::data_received(const void *data, const size_t size)
{
	const size_t frame_bytes = kMicChannels * sizeof(int16_t);

	if (data == nullptr || size < frame_bytes || m_buffer_size == 0) {
		return;
	}

	const auto *samples = static_cast<const int16_t *>(data);
	const size_t frames = size / frame_bytes;
	bool ready = false;

	k_mutex_lock(&m_lock, K_FOREVER);

	if (m_stopped) {
		k_mutex_unlock(&m_lock);
		return;
	}

	const float gain = adjust_dynamic_gain(samples, frames);

	/* Left channel only. Average each decimation group down to the model rate
	 * so a half-window is half a second, not half a second of 48 kHz samples.
	 */
	for (size_t i = 0; i < frames; i++) {
		const float scaled = static_cast<float>(samples[i * kMicChannels]) * gain;
		int32_t sample_value = static_cast<int32_t>(scaled);

		if (sample_value > kInt16Max) {
			sample_value = kInt16Max;
		} else if (sample_value < kInt16Min) {
			sample_value = kInt16Min;
		}

		m_decim_sum += sample_value;
		m_decim_count++;
		if (m_decim_count < kDecimation) {
			continue;
		}

		const int16_t model_sample = static_cast<int16_t>(
			(m_decim_sum + static_cast<int32_t>(kDecimation / 2)) /
			static_cast<int32_t>(kDecimation));
		push_model_sample(model_sample);
		m_decim_sum = 0;
		m_decim_count = 0;
	}

	/* The older half is already present (silence, then the previous capture). */
	ready = m_filled >= m_buffer_size;
	k_mutex_unlock(&m_lock);

	if (ready) {
		k_sem_give(&m_data_ready);
	}
}

void *LiveMicInput::GetInputData(size_t &size)
{
	size = 0;

	if (m_buffer_size < 2) {
		return nullptr;
	}

	k_mutex_lock(&m_lock, K_FOREVER);
	if (m_stopped) {
		k_mutex_unlock(&m_lock);
		return nullptr;
	}

	/* Keep the half captured last time and fill the rest on this call. */
	if (m_primed) {
		const size_t stride = m_buffer_size / 2;

		if (m_filled > stride) {
			memmove(m_audio_input_buffer, m_audio_input_buffer + stride,
				(m_filled - stride) * sizeof(int16_t));
			m_filled -= stride;
		} else {
			m_filled = 0;
		}
	}
	drain_pending();
	k_mutex_unlock(&m_lock);

	for (;;) {
		k_mutex_lock(&m_lock, K_FOREVER);
		if (m_stopped) {
			k_mutex_unlock(&m_lock);
			return nullptr;
		}
		if (m_filled >= m_buffer_size) {
			size = m_buffer_size * sizeof(int16_t);
			m_primed = true;
			k_mutex_unlock(&m_lock);
			return m_audio_input_buffer;
		}
		k_mutex_unlock(&m_lock);

		k_sem_take(&m_data_ready, K_FOREVER);
	}
}
