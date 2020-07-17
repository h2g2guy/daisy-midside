#include "daisysp.h"
#include "daisy_patch.h"
#include <string>

#include "oscilloscope.hpp"

using namespace daisy;
using namespace daisysp;

constexpr int bufferSize = 4096;

DaisyPatch patch;

int samplesPerCallback = 0;

float midBuffer[bufferSize] = { 0 };
float returnBuffer[bufferSize] = { 0 };
float sideBuffer[bufferSize] = { 0 };
size_t bufferIndex = 0;
int bufferOffset = 0;

Parameter bufferOffsetParam;

Oscilloscope oscilloscopes[2];
size_t scopeIndex;

uint32_t lastScreenUpdate = 0;

void UpdateControls()
{
    patch.UpdateAnalogControls();
    patch.DebounceControls();

//    //read ctrls and gates, then update sampleholds
//    sampHolds[0].Process(patch.gate_input[0].State(), patch.controls[0].Process());
//    sampHolds[1].Process(patch.gate_input[1].State(), patch.controls[1].Process());
//

    //encoder
    if (patch.encoder.Pressed())
    {
        int scopeChange = patch.encoder.Increment();
        scopeIndex = (scopeIndex + scopeChange + 3) % 3;
    }
    else if (scopeIndex != 2)
    {
        // scale current oscilloscope
        int scale = patch.encoder.Increment();
        if (scale < 0)
        {
            oscilloscopes[scopeIndex].SetScale(oscilloscopes[scopeIndex].GetScale() + 1);
        }
        if (scale > 0 && oscilloscopes[scopeIndex].GetScale() > 1)
        {
            oscilloscopes[scopeIndex].SetScale(oscilloscopes[scopeIndex].GetScale() - 1);
        }
    }

    bufferOffset = bufferOffsetParam.Process();

//    menuPos = (menuPos % 2 + 2) % 2;
//
//    //switch modes
//    if(patch.encoder.RisingEdge())
//    {
//        sampHolds[menuPos].mode = (SampleHold::Mode)((sampHolds[menuPos].mode + 1) % 2);
//    }
}

void AudioCallback(float** in, float** out, size_t size)
{
    UpdateControls();

    for (size_t i = 0; i < size; i++)
    {
        // Send the mid input via the first output, and store it in the buffer
        float mid = midBuffer[bufferIndex] = out[0][i] = in[0][i];

        // Retrieve the return signal
        float returned = returnBuffer[bufferIndex] = in[1][i];

        // Decide where to read from the buffer
        int bufferReadIndex = (bufferIndex - bufferOffset + bufferSize) % bufferSize;

        // Calculate the side signal by subtracting the return from the mid; allow monitoring from the second output
        float side = sideBuffer[bufferIndex] = out[1][i] = midBuffer[bufferReadIndex] - returned;

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


void UpdateOutputs()
{
//    dsy_dac_write(DSY_DAC_CHN1, sampHolds[0].output * 4095);
//    dsy_dac_write(DSY_DAC_CHN2, sampHolds[1].output * 4095);
}

void UpdateOled()
{
    if (scopeIndex < 2)
    {
        patch.display.Fill(false);

        size_t indices[] = { (bufferIndex - bufferOffset + bufferSize) % bufferSize, bufferIndex };
        oscilloscopes[scopeIndex].Draw(indices);

        patch.display.SetCursor(0, SSD1309_HEIGHT - 8);
        std::string str = std::to_string(oscilloscopes[scopeIndex].GetScale());
        char* s = &str[0];
        patch.display.WriteString(s, Font_6x8, false);
    }
    else
    {
        patch.display.Fill(true);
        std::string str = "woof";
        char* s = &str[0];
        patch.display.WriteString(s, Font_11x18, false);
    }

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

    Window scope1windows[2];
    scope1windows[0].buffer = midBuffer;
    scope1windows[0].bufferLength = bufferSize;
    scope1windows[0].x = 0;
    scope1windows[0].y = 0;
    scope1windows[0].width = SSD1309_WIDTH;
    scope1windows[0].height = SSD1309_HEIGHT / 2;

    scope1windows[1] = scope1windows[0];
    scope1windows[1].buffer = returnBuffer;
    scope1windows[1].y = SSD1309_HEIGHT / 2;

    OscilloscopeParams params;
    params.display = &patch.display;
    params.windows = scope1windows;
    params.windowCount = 2;
    params.syncChannel = 0;

    oscilloscopes[0].Init(params);

    Window scope2window;
    scope2window.buffer = sideBuffer;
    scope2window.bufferLength = bufferSize;
    scope2window.x = 0;
    scope2window.y = 0;
    scope2window.width = SSD1309_WIDTH;
    scope2window.height = SSD1309_HEIGHT;

    OscilloscopeParams params2;
    params2.display = &patch.display;
    params2.windows = &scope2window;
    params2.windowCount = 1;
    params2.syncChannel = 0;

    oscilloscopes[1].Init(params2);

    while(1)
    {
        // UpdateOutputs();
        if (dsy_system_getnow() - lastScreenUpdate > 17)
        {
            UpdateOled();
            lastScreenUpdate = dsy_system_getnow();
        }
    }
}
