#include "daisysp.h"
#include "daisy_patch.h"
#include <string>

#include "oscilloscope.hpp"

using namespace daisy;
using namespace daisysp;

constexpr int bufferSize = 4096;

DaisyPatch patch;

int samplesPerCallback = 0;

float audioBuffer[bufferSize] = { 0 };
size_t bufferIndex = 0;
int bufferOffset = 0;

Parameter bufferOffsetParam;

Oscilloscope oscilloscope;

uint32_t lastScreenUpdate = 0;

void AudioCallback(float** in, float** out, size_t size)
{
    patch.UpdateAnalogControls();
    bufferOffset = bufferOffsetParam.Process();

    for (size_t i = 0; i < size; i++)
    {
        // Send the mid input via the first output, and store it in the buffer
        float mid = audioBuffer[bufferIndex] = out[0][i] = in[0][i];

        // Retrieve the return signal
        float returned = in[1][i];

        // Decide where to read from the buffer
        int bufferReadIndex = (bufferIndex - bufferOffset + bufferSize) % bufferSize;

        // Calculate the side signal by subtracting the return from the mid; allow monitoring from the second output
        float side = out[1][i] = audioBuffer[bufferReadIndex] - returned;

        // Get mid and side output levels based on controls
        mid *= patch.controls[0].Value();
        side *= patch.controls[1].Value();

        // Output the left and right channels
        out[2][i] = mid + side;
        out[3][i] = mid - side;

        bufferIndex = (bufferIndex + 1) % bufferSize;
    }

    samplesPerCallback = size;
}

void UpdateControls()
{
    patch.UpdateAnalogControls();
    patch.DebounceControls();
//
//    //read ctrls and gates, then update sampleholds
//    sampHolds[0].Process(patch.gate_input[0].State(), patch.controls[0].Process());
//    sampHolds[1].Process(patch.gate_input[1].State(), patch.controls[1].Process());
//
    //encoder
    int scale = patch.encoder.Increment();
    if (scale < 0)
    {
        oscilloscope.SetScale(oscilloscope.GetScale() + 1);
    }
    if (scale > 0 && oscilloscope.GetScale() > 1)
    {
        oscilloscope.SetScale(oscilloscope.GetScale() - 1);
    }

//    menuPos = (menuPos % 2 + 2) % 2;
//
//    //switch modes
//    if(patch.encoder.RisingEdge())
//    {
//        sampHolds[menuPos].mode = (SampleHold::Mode)((sampHolds[menuPos].mode + 1) % 2);
//    }
}

void UpdateOutputs()
{
//    dsy_dac_write(DSY_DAC_CHN1, sampHolds[0].output * 4095);
//    dsy_dac_write(DSY_DAC_CHN2, sampHolds[1].output * 4095);
}

void UpdateOled()
{
    patch.display.Fill(false);

    size_t indices[] = { bufferIndex, bufferIndex };
    oscilloscope.Draw(indices);

    patch.display.SetCursor(0, SSD1309_HEIGHT - 8);
    std::string str = std::to_string(oscilloscope.GetScale());
    char* s = &str[0];
    patch.display.WriteString(s, Font_6x8, false);

    patch.display.Update();
}

int main()
{
    //float sampleRate;
    patch.Init();
    //sampleRate = patch.AudioSampleRate();

    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    bufferOffsetParam.Init(patch.controls[2], 0, 1024, Parameter::LINEAR);

    dsy_system_delay(5000);

    Window windows[2];
    windows[0].buffer = audioBuffer;
    windows[0].bufferLength = bufferSize;
    windows[0].x = 0;
    windows[0].y = 0;
    windows[0].width = SSD1309_WIDTH;
    windows[0].height = SSD1309_HEIGHT / 2;

    windows[1] = windows[0];
    windows[1].y = SSD1309_HEIGHT / 2;

    OscilloscopeParams params;
    params.display = &patch.display;
    params.windows = windows;
    params.windowCount = 2;
    params.syncChannel = 0;

    oscilloscope.Init(params);

    while(1)
    {
        UpdateControls();
        UpdateOutputs();
        if (dsy_system_getnow() - lastScreenUpdate > 17)
        {
            UpdateOled();
            lastScreenUpdate = dsy_system_getnow();
        }
    }
}
