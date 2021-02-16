# daisy-midside
![build](https://github.com/h2g2guy/daisy-midside/workflows/C/C++%20CI/badge.svg)

A Daisy Patch based module that can turn any FX module into a wild stereo effect.

## What?
I *love* messing around with stereo effects in Eurorack. Problem is, a lot of the effects we use -- wavefolders, distortion, etc -- aren't always in stereo. 

daisy-midside uses the concepts behind mid-side processing in a novel way to apply transform those monoaural effects into a weird stereo soundscape. Some may sound like the
original effect; others might not. You'll just have to experiment and find out ;)

## How does it work?
Send a dry signal into In 1 on the Daisy Patch -- it gets multed to Out 1 for convenience. Send that same signal to a mono effect. Non-time-based effects seem to work best, but
feel free to experiment! Send the fully wet signal into In 2.

You'll see two scope traces on the screen. The top is the dry signal; the bottom is the wet signal. Adjust CTRL 3 to phase-align the two signals... or send in a CV if you're 
feeling particularly wild!

Internally, the module will subtract the dry signal from the wet signal to produce what I call the 'difference' signal. Then, we perform a typical mid-side conversion, where
the dry signal is treated as the mid channel, and the "difference" is treated as the side channel. (If you don't know what mid-side processing is, that's okay -- basically, we're
just adding the "difference" to the left channel, and subtracting it from the right!)

You can use CTRL 1 and 2 to tweak the balance of the mid and side signals, respectively. 

## What else?
The scope is adjustable. Click the encoder until the bottom number is highlighted, then turn right to zoom in, and left to zoom out. Click the encoder until the bottom number is
not highlighted to adjust the scope gain. (The highlighted "Null" at the top of the screen should display the scope gain. This is a known bug.)

Click and hold the encoder and twist to see other pages. Page 2 is a single scope displaying the side channel; the same controls can be used to adjust this scope.
Page 3 is currently unimplemented, but will serve as a status/menu page. (Today, it simply reads "meow".)
