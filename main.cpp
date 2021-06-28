#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <cstdio>
#include <thread>
#include <audio>
#include <array>
#include <WinUser.h>

int main()
{
	using namespace std::experimental;

	auto device = get_default_audio_output_device();
	if (!device)
		return 1;

	struct Note
	{
		float delta;
		float phase;
		float gain = 0.0f;
		bool pressed = false;
	};

	std::array<Note, 8> notes;
	{
		const std::array<float, 8> intervals{1.0f / 1.0f, 9.0f / 8.0f, 5.0f / 4.0f, 4.0f / 3.0f, 3.0f / 2.0f, 5.0f / 3.0f, 15.0f / 8.0f, 2.0f / 1.0f};

		float fundamental_frequency_hz = 261.63f;
		for (size_t i = 0; i < 8; i++)
		{
			notes[i] = {2.0f * fundamental_frequency_hz * intervals[i] * float(M_PI / device->get_sample_rate()), 0};
		}
	}

	bool distortionOn = false;

	device->start();

	printf("keys: a s d f g h j k\nDISTORTION: Ctrl");

	while (!(GetAsyncKeyState(VK_ESCAPE) & (0x01 << (8 * sizeof(SHORT)))))
	{
		distortionOn = GetAsyncKeyState(VK_LCONTROL) & 0x01 ? !distortionOn : distortionOn;

		notes[0].pressed = GetAsyncKeyState('A') & (0x01 << (8 * sizeof(SHORT)));
		notes[1].pressed = GetAsyncKeyState('S') & (0x01 << (8 * sizeof(SHORT)));
		notes[2].pressed = GetAsyncKeyState('D') & (0x01 << (8 * sizeof(SHORT)));
		notes[3].pressed = GetAsyncKeyState('F') & (0x01 << (8 * sizeof(SHORT)));
		notes[4].pressed = GetAsyncKeyState('G') & (0x01 << (8 * sizeof(SHORT)));
		notes[5].pressed = GetAsyncKeyState('H') & (0x01 << (8 * sizeof(SHORT)));
		notes[6].pressed = GetAsyncKeyState('J') & (0x01 << (8 * sizeof(SHORT)));
		notes[7].pressed = GetAsyncKeyState('K') & (0x01 << (8 * sizeof(SHORT)));

		device->process([&](audio_device &, audio_device_io<float> &io)
						{
							if (!io.output_buffer.has_value())
								return;

							auto &out = *io.output_buffer;

							for (int frame = 0; frame < out.size_frames(); ++frame)
							{
								float next_sample = 0;

								for (auto &note : notes)
								{
									note.gain += note.pressed * 0.002f - 0.001f;
									note.gain = note.gain >= 1.0f ? 1.0f : note.gain <= 0.0f ? 0.0f
																							 : note.gain;
									note.phase = std::fmod(note.phase + note.delta, 2.0f * static_cast<float>(M_PI));

									next_sample += 0.2f * note.gain * std::sin(note.phase);
								}
								if (distortionOn)
									for (size_t i = 0; i < 30; i++)
										next_sample = tanh(next_sample * 1.05f);

								next_sample = next_sample >= 1.0f ? 1.0f : next_sample <= -1.0f ? -1.0f
																								: next_sample;

								for (int channel = 0; channel < out.size_channels(); ++channel)
									out(frame, channel) = next_sample;
							}
						});
	}
}