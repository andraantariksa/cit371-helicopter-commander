#include "AudioRecording.hpp"

#include <portaudio.h>
#include <cstring>
#include <iostream>

static int play_callback(
	const void* input_buffer,
	void* output_buffer,
	unsigned long frames_per_buffer,
	const PaStreamCallbackTimeInfo* time_info,
	PaStreamCallbackFlags status_flags,
	void* user_data)
{
	InputSoundData* data = (InputSoundData*)user_data;
	SAMPLE_TYPE* rptr = &data->samples[data->frame_index * TOTAL_CHANNEL];
	SAMPLE_TYPE* wptr = (SAMPLE_TYPE*)output_buffer;
	unsigned int i;
	int finished;
	unsigned int frames_left = data->max_frame_index - data->frame_index;

	// Prevent unused variable warnings
	(void)input_buffer;
	(void)time_info;
	(void)status_flags;
	(void)user_data;

	if (frames_left < frames_per_buffer)
	{
		for (i = 0; i < frames_left; i++)
		{
			*wptr++ = *rptr++; // Left
			if (TOTAL_CHANNEL == 2) *wptr++ = *rptr++; // Right
		}
		for (; i < frames_per_buffer; i++)
		{
			*wptr++ = 0; // Left
			if (TOTAL_CHANNEL == 2) *wptr++ = 0; // Right
		}
		data->frame_index += frames_left;
		finished = paComplete;
	}
	else
	{
		for (i = 0; i < frames_per_buffer; i++)
		{
			*wptr++ = *rptr++; // Left
			if (TOTAL_CHANNEL == 2) *wptr++ = *rptr++; // Right
		}
		data->frame_index += frames_per_buffer;
		finished = paContinue;
	}
	return finished;
}

static int record_callback(
	const void* input_buffer,
	void* output_buffer,
	unsigned long frames_per_buffer,
	const PaStreamCallbackTimeInfo* time_info,
	PaStreamCallbackFlags status_flags,
	void* user_data)
{
	InputSoundData* data = (InputSoundData*)user_data;
	const SAMPLE_TYPE* rptr = (SAMPLE_TYPE*)input_buffer;
	SAMPLE_TYPE* wptr = &data->samples[data->frame_index * TOTAL_CHANNEL];
	long frame_to_calculate;
	long i;
	int finished;
	unsigned int frames_left = data->max_frame_index - data->frame_index;

	// Prevent unused variable warnings.
	(void)output_buffer;
	(void)time_info;
	(void)status_flags;
	(void)user_data;

	if (frames_left < frames_per_buffer)
	{
		frame_to_calculate = frames_left;
		finished = paComplete;
	}
	else
	{
		frame_to_calculate = frames_per_buffer;
		finished = paContinue;
	}

	if (input_buffer == NULL)
	{
		for (i = 0; i < frame_to_calculate; i++)
		{
			*wptr++ = SAMPLE_SILENCE; // Left
			if (TOTAL_CHANNEL == 2) *wptr++ = SAMPLE_SILENCE; // Right
		}
	}
	else
	{
		for (i = 0; i < frame_to_calculate; i++)
		{
			*wptr++ = *rptr++; // Left
			if (TOTAL_CHANNEL == 2) *wptr++ = *rptr++; // Right
		}
	}
	data->frame_index += frame_to_calculate;
	return finished;
}

Recording::Recording(int total_seconds, int sample_rate)
{
	this->sample_rate = sample_rate;
	this->total_channel = TOTAL_CHANNEL;
	this->total_seconds = total_seconds;

	total_frame = total_seconds * sample_rate;
	total_samples = total_frame * total_channel;
	total_bytes = total_samples * sizeof(SAMPLE_TYPE);

	sound_data.max_frame_index = total_frame;
	sound_data.samples = new SAMPLE_TYPE[total_samples];
	if (sound_data.samples == nullptr)
	{
		abort();
	}

	input_param.device = Pa_GetDefaultInputDevice();
	if (input_param.device == paNoDevice) {
		std::cout << "PortAudio error: No input device\n";
		abort();
	}
	input_param.channelCount = 1; // Mono
	input_param.sampleFormat = PA_SAMPLE_TYPE;
	input_param.suggestedLatency = Pa_GetDeviceInfo(input_param.device)->defaultLowInputLatency;
	input_param.hostApiSpecificStreamInfo = nullptr;


	output_param.device = Pa_GetDefaultOutputDevice();
	if (output_param.device == paNoDevice) {
		std::cout << "PortAudio error: No output device\n";
		abort();
	}
	output_param.channelCount = 1; // Mono
	output_param.sampleFormat = PA_SAMPLE_TYPE;
	output_param.suggestedLatency = Pa_GetDeviceInfo(output_param.device)->defaultLowOutputLatency;
	output_param.hostApiSpecificStreamInfo = NULL;
}

Recording::~Recording()
{
	delete[] sound_data.samples;
}

size_t Recording::get_total_samples()
{
	return (size_t)total_samples;
}

void Recording::play_previous_record()
{
	err = paNoError;
	sound_data.frame_index = 0;

	err = Pa_OpenStream(
		&stream,
		NULL,
		&output_param,
		sample_rate,
		FRAMES_PER_BUFFER,
		paClipOff,
		play_callback,
		&sound_data);
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}

	std::cout << "Playing audio" << std::endl;
	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}

	while (Pa_IsStreamActive(stream))
	{
		Pa_Sleep(100);
	}

	std::cout << "Stop playing audio" << std::endl;
	err = Pa_CloseStream(stream);
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}
}

SAMPLE_TYPE* Recording::record()
{
	err = paNoError;

	sound_data.frame_index = 0;

	memset(sound_data.samples, 0, total_samples);

	PaStream* stream;
	err = Pa_OpenStream(
		&stream,
		&input_param,
		nullptr,
		sample_rate,
		FRAMES_PER_BUFFER,
		paClipOff, // Clip audio in the end
		record_callback,
		&sound_data);
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}

	std::cout << "Start Recording" << std::endl;
	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}

	while (Pa_IsStreamActive(stream))
	{
		Pa_Sleep(1000);
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}
	std::cout << "End Recording" << std::endl;

	return sound_data.samples;
}
