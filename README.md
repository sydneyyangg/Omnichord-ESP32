# Omnichord

## Description

The project is an omnichord, which is an electronic harp. Users can touch the softpot sensor and trigger a ringing harp tone in the corresponding note, depending on the location(s) touched on the softpot sensor. 

## Technical explanation

### Softpot sensor
In the softpot sensor task, when the softpot sensor is touched, it sends an analog value to the analog pin of the MCU (ESP32). This translates to a digital value from 0-4096. 
Then, the MCU takes that value and assigns it a note value in the softpot task. For instance, from 0-8, note 1 of a chord is triggered. 
This is put into an RTOS queue and sent to the audio synthesis task. 

### Audio synthesis
In the audio synthesis task, a there is 256 precalculated samples of one wave cycle, which are produced with the following formula:

phase = 2pi * i / 256
    
sample = (math.sin(phase) + 
            0.3 * math.sin(2 * phase) + 
            0.15 * math.sin(3 * phase) + 
            0.1 * math.sin(4 * phase))

here, we take the angular frequency, 2pi in this case, and multiply by i, which increments every loop. we divide by 256 as we would effectively then iterate through 1/256 of a cycle each loop. 

the math.sin(phase), where angular frequency of = 1, dominates. however, there are harmonics (of lesser volume/amp) where phase is multipled by 2, 3, or 4, which adds extra higher frequencies to create a brighter layered sound.

to create varied notes, you need to have varied frequencies. This is done by iterating through the wave at different rates, where a faster speed means higher frequency, and thus a higher note.

Fade was done by linearly decreasing the amplitude of the note based on the time since tap.

What is finally output is a waveform of int16_t, where it adds all of the active notes' values (since audio obeys superposition), which encapsulates volume, and frequency.

This waveform is output via I2S into the DAC + amp module, which then processes the digital waveform into an audio signal to be amplified and output into a speaker. This creates the notes we hear!

## Features and Hardware

Buttons are used to change the chords. Upon click, it selects the first index of a 2D frequency array. The first index is the chord, and the second index is the note of the chord. 

The components were perfboarded, and powered with an esp32. that esp32 was powered with a portable charger.

The enclosure was made with laser cut acrylic on the top and bottom panels, which a hole to access the pushbottons and the softpot sensor. 
The two panels were separated and held together with 3D printed pillars.

The videos are available [here:](https://drive.google.com/drive/folders/1StZ0ep-ZoS5HR6rfUK1RdGIkPp2XnS4G?usp=sharing)
