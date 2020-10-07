#include <fdeep/model.hpp> // Start this before anything else!
#include <Windows.h>
#include <iostream>
#include <vector>
#include <portaudio.h>

#include "AudioRecording.hpp"

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
	auto model = fdeep::load_model("C:\\Users\\andra\\Projects\\Helicopter Commander\\Helicopter Commander\\model.json");
	PaError err = paNoError;
	err = Pa_Initialize();
	if (err != paNoError)
	{
		std::cout << "PortAudio error: " << Pa_GetErrorText(err) << '\n';
		abort();
	}


	//MSG msg;
	//while (true)
	//{
	//	//auto activeWindow = GetForegroundWindow();
	//	if (GetMessage(&msg, NULL, 0, 0))
	//	{
	//		std::cout << "Enter";
	//		if (msg.message == VK_END)
	//		{
	//			std::cout << "OK" << std::endl;
	//		}
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//}

	while (true) {
		std::cout << "Next\n";
		char trash;
		std::cin >> trash;

		std::cout << "Record" << std::endl;

		Recording record(1);
		SAMPLE_TYPE* samples = record.record();
		size_t total_samples = record.get_total_samples();

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

		std::cout << i_max << " (" << labels[i_max] << ") with val " << val_max << std::endl;
	}

	Pa_Terminate();

	return 0;
}
