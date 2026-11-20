/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef LIVEMICINPUT_H
#define LIVEMICINPUT_H

#include "mfcc/MicroNetKwsModel.hpp"

#include <cstddef>
#include <cstdint>
#include <zephyr/kernel.h>

class LiveMicInput
{
public:
	/* One second of mono audio at the model sample rate. */
	static constexpr size_t kAudioCapacity = static_cast<size_t>(arm::app::kws::g_AudioRate);
	/* Room for audio that arrives while a full window is still being read. */
	static constexpr size_t kPendingCapacity = kAudioCapacity / 10;

	/* buffer_size is the inference window in mono PCM samples. */
	LiveMicInput(size_t buffer_size);

	bool Start(void);
	bool Stop(void);

	/* Block until half a window of new audio is available.
	 * The first call leaves the older half as silence.
	 * size is set to the full window length in bytes. Returns nullptr if stopped.
	 */
	void *GetInputData(size_t &size);

	void data_received(const void *data, const size_t size);

private:
	void push_model_sample(int16_t sample);
	void drain_pending();
	float adjust_dynamic_gain(const int16_t *samples, size_t frames);

	size_t m_buffer_size;
	size_t m_filled;
	size_t m_pending;
	size_t m_decim_count;
	size_t m_samples_since_gain_rise;
	int32_t m_decim_sum;
	bool m_primed;
	bool m_stopped;
	struct k_sem m_data_ready;
	struct k_mutex m_lock;
	int16_t m_audio_input_buffer[kAudioCapacity];
	int16_t m_pending_samples[kPendingCapacity];
	float m_current_gain;
};

#endif /* LIVEMICINPUT_H */
