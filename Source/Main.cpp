#include <fdeep/model.hpp> // Start this before anything else!
#include <Windows.h>
#include <iostream>
#include <vector>
#include <portaudio.h>
#include <rnnoise.h>
#include <set>
#include <thread>
#include <psapi.h>

#include "AudioRecording.hpp"

void loop(std::set<short>* instruction)
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = 0;

	char name[200];

	while (true)
	{
		HWND hwnd = GetForegroundWindow();
		GetWindowTextA(hwnd, name, 200);
		
		if (strcmp(name, "GTA: San Andreas") == 0) {
			for (short key: *instruction)
			{
				std::cout << "Instruction " << key << "\n";
				ip.ki.wVk = key;
				SendInput(1, &ip, sizeof(INPUT));
			}

			Sleep(200);
		}
	}
}

int main()
{
	std::vector<std::string> labels = {
		"kanan",
		"kiri",
		"maju",
		"mundur",
		"naik",
		"stop",
		"tembak",
		"turun"
	};
	auto model = fdeep::load_model("C:\\model.json");

	std::set<short> instruction;
	std::thread t(loop, &instruction);

	PaError err = paNoError;
	err = Pa_Initialize();
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}

	DenoiseState* st = rnnoise_create(nullptr);

	while (true)
	{
		std::cout << "Next\n";
		std::cin.get();

		std::cout << "Record" << std::endl;

		Recording record(1);
		SAMPLE_TYPE* samples = record.record();
		size_t total_samples = record.get_total_samples();

		rnnoise_process_frame(st, samples, samples);

		record.play_previous_record();

		fdeep::tensor tensor_output(fdeep::tensor_shape(total_samples, 1), std::vector<float>(samples, samples + total_samples));
		auto prediction_result = model.predict({ tensor_output });

		std::cout << fdeep::show_tensors(prediction_result) << '\n';

		size_t i_max = 0;
		float val_max = 0.0f;
		std::vector<float> prediction_result_vec = prediction_result[0].to_vector();
		for (size_t i = 0; i < labels.size(); ++i)
		{
			float temp = prediction_result_vec[i];
			if (temp > val_max)
			{
				val_max = temp;
				i_max = i;
			}
		}

		std::cout << i_max << " (" << labels[i_max] << ") with val " << val_max << '\n';

		if (i_max == 4)
		{
			instruction.insert(0x57);
			std::cout << "Added instruction, naik\n";
		}
		else if (i_max == 5)
		{
			instruction.clear();
			std::cout << "Clearing instruction\n";
		}
		else if (i_max == 7)
		{
			instruction.insert(0x53);
			std::cout << "Added instruction, turun\n";
		}
	}

	rnnoise_destroy(st);

	Pa_Terminate();

	return 0;
}
