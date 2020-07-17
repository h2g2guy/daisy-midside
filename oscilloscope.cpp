#include "oscilloscope.hpp"

Oscilloscope::Oscilloscope(OscilloscopeParams& params)
{
    Init(params);
}

void Oscilloscope::Init(OscilloscopeParams& params)
{
    display = params.display;
    windows = params.windows;
    windowCount = params.windowCount;
    syncChannel = params.syncChannel;
    scale = params.scale;
    overallGain = params.overallGain;

    initialized = true;
}

unsigned int Oscilloscope::GetScale() { return scale; }
void Oscilloscope::SetScale(unsigned int xScale) { scale = xScale; }

float Oscilloscope::GetGain() { return overallGain; }
void Oscilloscope::SetGain(float g) { overallGain = g; }

constexpr int wrap(int i, int mod)
{
    return (i + mod) % mod;
}

int Oscilloscope::getZeroCrossingOffset(size_t* currentIndices)
{
    // Get the largest window we'll need to fill, and the smallest buffer; this info will be important regardless
    // of what we do.
    unsigned int maxWindow = 0;
    size_t minBuffer = SIZE_MAX;

    for (unsigned int i = 0; i < windowCount; i++)
    {
        if (maxWindow < windows[i].width)
        {
            maxWindow = windows[i].width;
        }

        if (minBuffer > windows[i].bufferLength)
        {
            minBuffer = windows[i].bufferLength;
        }
    }

    if (maxWindow > minBuffer)
    {
        return -1;
    }

    if (syncChannel == NoSync)
    {
        return maxWindow;
    }

    // Either find an upward zero crossing, or look back as far as the start of the buffer
    float* buffer = windows[syncChannel].buffer;
    int bufferLength = windows[syncChannel].bufferLength;
    int now = currentIndices[syncChannel];
    int offset = windows[syncChannel].width * scale;

    int curr, next;
    while (offset < bufferLength - 1)
    {
        offset++;

        curr = wrap(now - offset, bufferLength);
        next = wrap(now - offset + 1, bufferLength);

        if (buffer[curr] <= 0.f && buffer[next] >= 0.f)
        {
            break;
        }
    }

    return offset;
}

void Oscilloscope::drawWindow(Window& w, size_t startingIndex)
{
    int midHeight = w.y + (w.height / 2);
    float defaultGain = w.height / 2.f;

    for (unsigned int i = 0; i < w.width; i++)
    {
        int index = wrap(startingIndex + (i*scale), w.bufferLength);

        int height = static_cast<int>(w.buffer[index] * defaultGain * overallGain) + midHeight;

        display->DrawPixel(w.x + i, height, true);
    }
}


void Oscilloscope::Draw(size_t* currentIndices)
{
    if (!initialized)
    {
        std::string str1 = "Not";
        std::string str2 = "initialized!";
        char* s1 = &str1[0];
        char* s2 = &str2[0];

        display->Fill(true);
        display->SetCursor(0, 0);
        display->WriteString(s1, Font_11x18, false);
        display->SetCursor(0, 18);
        display->WriteString(s2, Font_11x18, false);

        return;
    }

    int offset = getZeroCrossingOffset(currentIndices);

    for (size_t i = 0; i < windowCount; i++)
    {
        drawWindow(windows[i], wrap(currentIndices[i] - offset, windows[i].bufferLength));
    }
}
