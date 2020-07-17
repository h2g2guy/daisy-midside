#include "daisy_patch.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

struct Window
{
    // Data to draw
    float* buffer;
    size_t bufferLength;

    // Size of the window on the display
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
};

struct OscilloscopeParams
{
    // Display to draw to
    OledDisplay* display;

    // Array of windows for this oscilloscope
    Window* windows;
    size_t windowCount = 0;

    // Which window we should sync zero crossings with (or NoSync)
    int syncChannel = -1;

    unsigned int scale = 1;
    float overallGain = 1.f;
};

class Oscilloscope
{
public:
    static constexpr int NoSync = -1;

    Oscilloscope() = default;
    Oscilloscope(OscilloscopeParams& params);

    void Init(OscilloscopeParams& params);

    unsigned int GetGain();
    void SetGain(unsigned int gain);

    unsigned int GetScale();
    void SetScale(unsigned int xScale);

    void Draw(size_t* currentIndices);

private:
    OledDisplay* display;

    Window* windows;
    unsigned int windowCount;

    int syncChannel;

    unsigned int scale;
    float overallGain;

    int getZeroCrossingOffset(size_t* currentIndices);
    void drawWindow(Window& w, size_t startingIndex);

    bool initialized = false;
};
