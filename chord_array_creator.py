import math

# Generate one cycle of a note with harmonics (sparkly sound)
WAVETABLE_SIZE = 256
wavetable = []

for i in range(WAVETABLE_SIZE):
    phase = 2 * math.pi * i / WAVETABLE_SIZE
    
    # Add harmonics for sparkle
    sample = (math.sin(phase) + 
              0.3 * math.sin(2 * phase) + 
              0.15 * math.sin(3 * phase) + 
              0.1 * math.sin(4 * phase))
    
    # Normalize to prevent clipping
    sample = sample / 1.55
    
    # Convert to int16_t range
    sample_int = int(sample * 32767)
    wavetable.append(sample_int)

# Print as C array
print("const int16_t WAVETABLE[256] = {")
for i in range(0, len(wavetable), 8):
    print("   ", ", ".join(str(x) for x in wavetable[i:i+8]), ",")
print("};")