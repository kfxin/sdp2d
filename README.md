# sdp2d
A 2D seismic data processing software developed using Qt

## Compile & Install
1. Install the latest version Qt and FFTW and update the following two lines in the sdp2d.pro file with the new FFTW path:
INCLUDEPATH += /Users/kxin/opt/fftw-3.3.8/include
LIBS += -L/Users/kxin/opt/fftw-3.3.8/lib -lfftw3f -lm

2. Copy home_sdp2d.tar to the user home directory and decompress it. It job parameter XML files are stored in this .sdp2d directory.

3. Open the sdp2d.pro file with Qt Creator.

## Features
+ Using C++
+ Based on Qt and SeisUnix
+ Works on Linux and Mac. (Should also work on Windows, but has not tested it yet)
+ Four main components:
    - User interface & interactive functions
       - Bad trace picking
       - Top & bottom mute picking
       - Frequency analysis
       - Linear velocity measure
       - Amplitude & mute time QC
       - Stack velocity picking is on going
   - Data control and analysis
       - Only pre-stack data implemented
       - Check trace header & trace data values,
       - Offset/elevation, CDP fold curves
   - Parameters setup and input: 
       - One processing function for each job for the time being
   - Processing function modules: 
       - gain, filter, elevation statics...

![image](https://user-images.githubusercontent.com/100523053/155886630-24cd1965-fa5b-4eb3-a2d3-a89482ddce42.png)

![image](https://user-images.githubusercontent.com/100523053/155886633-f6e82b96-3a98-407d-9224-f922cf1666ed.png)


![image](https://user-images.githubusercontent.com/100523053/155886635-7c600253-c509-4740-8a78-68e62b083a49.png)
