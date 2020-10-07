#ifndef _H_AUDIO_RECORDING
#define _H_AUDIO_RECORDING

#include <portaudio.h>

#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SILENCE 0.0f
#define SAMPLE_TYPE float
#define FRAMES_PER_BUFFER 512
#define TOTAL_CHANNEL 1

struct InputSoundData
{
	int frame_index;
	int max_frame_index;
	SAMPLE_TYPE* samples;
};

class Recording
{
private:
	InputSoundData sound_data;
	int sample_rate;
	int total_channel;
	int total_seconds;

	int total_frame;
	int total_samples;
	int total_bytes;

	PaStreamParameters input_param;
	PaStreamParameters output_param;
	PaError err;
	PaStream* stream;

public:
	Recording(int total_seconds, int sample_rate = 8000);
	~Recording();

	size_t get_total_samples();

	void play_previous_record();
	SAMPLE_TYPE* record();
};

#endif
