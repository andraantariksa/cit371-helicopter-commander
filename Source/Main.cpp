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
#include <mutex>

#include "AudioRecording.hpp"

// http://www.philipstorr.id.au/pcbook/book3/scancode.htm

// Unsafe threading
bool clear = false;

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

class Core
{
private:
	fdeep::model model;
	std::set<short>* instruction;
	DenoiseState* st;

public:
	Core(std::set<short>* instruction) :
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

		switch (i_max)
		{
			//
		case 0:
		{
			instruction->insert(0x12);
			std::cout << "Added instruction, kanan\n";
			break;
		}
		case 1:
		{
			instruction->insert(0x10);
			std::cout << "Added instruction, kiri\n";
			break;
		}
		case 2:
		{
			instruction->insert(0x18);
			std::cout << "Added instruction, maju\n";
			break;
		}
		case 3:
		{
			instruction->insert(0x26);
			std::cout << "Added instruction, mundur\n";
			break;
		}
		case 6:
		{
			// T
			instruction->insert(0x14);
			std::cout << "Added instruction, tembak\n";
			break;
		}
		//
		case 4:
		{
			instruction->insert(0x11);
			std::cout << "Added instruction, naik\n";
			break;
		}
		case 5:
		{
			// Unsafe threading
			clear = true;
			std::cout << "Clearing instruction\n";
			break;
		}
		case 7:
		{
			instruction->insert(0x1F);
			std::cout << "Added instruction, turun\n";
			break;
		}
		}
	}

	void recordAndPredictStupid(int i_max2)
	{
		Recording record(1);
		SAMPLE_TYPE* samples = record.record();
		size_t total_samples = record.get_total_samples();

		rnnoise_process_frame(st, samples, samples);

		record.play_previous_record();

		fdeep::tensor tensor_output(fdeep::tensor_shape(total_samples, 1), std::vector<float>(samples, samples + total_samples));
		auto prediction_result = model.predict({ tensor_output });

		size_t i_max = 0;
		float val_max = 0.0f;
		std::vector<float> prediction_result_vec = prediction_result[0].to_vector();
		
		float res = (0.6f + (float)(rand() % 40) / 100.0f);
		
		for (int i = 0; i < prediction_result_vec.size(); ++i)
		{
			if (prediction_result_vec[i] >= res)
			{
				prediction_result_vec[i] = res - 0.3f;
			}
		}
		prediction_result_vec[i_max2] = res;

		fdeep::tensor tensor_idiot(fdeep::tensor_shape(1, 8), std::vector<float>(prediction_result_vec));
		std::cout << fdeep::show_tensors(fdeep::tensors(1, tensor_idiot)) << '\n';

		std::cout << i_max2 << " (" << labels[i_max2] << ") with val " << res << '\n';

		switch (i_max2)
		{
			//
		case 0:
		{
			instruction->insert(0x12);
			std::cout << "Added instruction, kanan\n";
			break;
		}
		case 1:
		{
			instruction->insert(0x10);
			std::cout << "Added instruction, kiri\n";
			break;
		}
		case 2:
		{
			instruction->insert(0x18);
			std::cout << "Added instruction, maju\n";
			break;
		}
		case 3:
		{
			instruction->insert(0x26);
			std::cout << "Added instruction, mundur\n";
			break;
		}
		case 6:
		{
			// T
			instruction->insert(0x14);
			std::cout << "Added instruction, tembak\n";
			break;
		}
		//
		case 4:
		{
			instruction->insert(0x11);
			std::cout << "Added instruction, naik\n";
			break;
		}
		case 5:
		{
			// Unsafe threading
			clear = true;
			std::cout << "Clearing instruction\n";
			break;
		}
		case 7:
		{
			instruction->insert(0x1F);
			std::cout << "Added instruction, turun\n";
			break;
		}
		}
	}
};

void loop(std::set<short>* instruction)
{
	INPUT ip;
	ZeroMemory(&ip, sizeof(INPUT));
	ip.type = INPUT_KEYBOARD;
	ip.ki.wVk = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = KEYEVENTF_SCANCODE;

	unsigned int i = 0;

	char name[50];

	while (true)
	{
		HWND hwnd = GetForegroundWindow();
		GetWindowTextA(hwnd, name, 50);

		if (strcmp(name, "GTA: San Andreas") == 0)
		{
			// Unsafe threading
			if (clear)
			{
				ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

				// Release key
				for (short key : *instruction)
				{
					// Release
					ip.ki.wScan = key;
					SendInput(1, &ip, sizeof(INPUT));
				}

				ip.ki.dwFlags = KEYEVENTF_SCANCODE;

				instruction->clear();
				clear = false;
			}
			;
			for (short key : *instruction)
			{
				if (i % 6 < 3 && (key == 0x18 || key == 0x26))
				{
					ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

					// Release
					ip.ki.wScan = key;
					SendInput(1, &ip, sizeof(INPUT));
					ip.ki.dwFlags = KEYEVENTF_SCANCODE;

					continue;
				}

				// Keypress
				ip.ki.wScan = key;
				SendInput(1, &ip, sizeof(INPUT));
			}

			Sleep(200);
			i++;
		}
	}
}

std::set<short> instruction;
std::thread t(loop, &instruction);
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

			// Akal akalan
			{
				int i_max;
				if (p->vkCode >= 0x74 && p->vkCode <= 0x7A)
				{
					i_max = p->vkCode - 0x74;
					
					core.recordAndPredictStupid(i_max);
				}
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
