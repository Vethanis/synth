import math
import matplotlib.pyplot as plt
tau = 6.2831853
pi = 3.141592
samples = 10
dphase = tau / samples
def lerp(a, b, alpha):
    return (1.0 - alpha) * a + alpha * b
def saw_wave(phase):
    return lerp(-1.0, 1.0, phase / tau)
def sine_wave(phase):
    return math.sin(phase)
def square_wave(phase):
    if phase < pi:
        return 1.0
    return -1.0
def triangle_wave(phase):
    if phase < pi:
        return lerp(-1.0, 1.0, phase / pi)
    phase -= pi
    return lerp(1.0, -1.0, phase / pi)

phase = 0.0
nums = []
nms = 5
invnms = 1.0 / nms
while phase < tau:
    val = 0.0
    for i in range(0, nms):
        jphase = phase + invnms * i * dphase
        val += sine_wave(jphase)
    val *= invnms
    nums.append(val)
    phase += dphase

plt.plot(nums)
plt.ylabel("Amplitude")
plt.xlabel("Sample")
plt.show()