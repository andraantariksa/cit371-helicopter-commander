#include <fdeep/model.hpp>
#include <portaudio.h>
#include <rnnoise.h>

#include <Windows.h>
#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <psapi.h>
#include <array>

#include "AudioRecording.hpp"

class Core
{
private:
	const std::array<char[7], 8> labels = {
		"kanan",
		"kiri",
		"maju",
		"mundur",
		"naik",
		"stop",
		"tembak",
		"turun"
	};
	fdeep::model model;
	std::set<short>* instruction;
	DenoiseState* st;

public:
	Core(std::set<short>* instruction):
		model(fdeep::load_model("C:\\model.json")),
		instruction(instruction)
	{
		st = rnnoise_create(nullptr);
	}

	~Core()
	{
		rnnoise_destroy(st);
	}

	void recordAndPredict()
	{
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
			instruction->insert(0x57);
			std::cout << "Added instruction, naik\n";
		}
		else if (i_max == 5)
		{
			instruction->clear();
			std::cout << "Clearing instruction\n";
		}
		else if (i_max == 7)
		{
			instruction->insert(0x53);
			std::cout << "Added instruction, turun\n";
		}
	}
};

void loop(std::set<short>& instruction)
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = 0;

	char name[50];

	while (true)
	{
		HWND hwnd = GetForegroundWindow();
		GetWindowTextA(hwnd, name, 50);

		if (strcmp(name, "GTA: San Andreas") == 0) {
			for (short key : instruction)
			{
				ip.ki.wVk = key;
				SendInput(1, &ip, sizeof(INPUT));
			}

			Sleep(200);
		}
	}
}

std::set<short> instruction;
std::thread t(loop, instruction);
Core core(&instruction);

LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL fEatKeystroke = FALSE;

	if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			if (fEatKeystroke = (p->vkCode == 0x73))
			{
				core.recordAndPredict();
			}
			break;
		}
	}
	return fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
	PaError err = paNoError;
	err = Pa_Initialize();
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}

	// Bind the global Windows keyboard hook
	HHOOK lowLevelKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, lowLevelKeyboardProc, 0, 0);

	MSG msg;
	while (GetMessage(&msg, NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(lowLevelKeyboardHook);

	Pa_Terminate();

	return 0;
}
