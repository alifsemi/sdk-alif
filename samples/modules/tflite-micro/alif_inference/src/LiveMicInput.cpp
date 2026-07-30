#include "LiveMicInput.h"

#include "AudioBackend.hpp"

bool LiveMicInput::Start()
{
	return audio_init(CONFIG_I2S_SAMPLE_RATE) == 0;
}

bool LiveMicInput::Stop()
{
	audio_uninit();
	return true;
}

bool LiveMicInput::GetInputData(void *buffer)
{
	int16_t *output_buffer = static_cast<int16_t *>(buffer);
	int num_samples = OutputSize / sizeof(int16_t);

	if (get_audio_data(output_buffer, num_samples) != 0) {
		return false;
	}

	if (wait_for_audio() != 0) {
		return false;
	}

	audio_preprocessing(output_buffer, num_samples);

	return true;
}
