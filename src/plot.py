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

def clamp(x, a, b):
    return min(max(x, a), b)

def quadratic_bezier(t, p):
    t0 = (1.0 - t) * (1.0 - t) * p[0]
    t1 = 2.0 * (1.0 - t) * t * p[1]
    t2 = t * t * p[2]
    return t0 + t1 + t2

def envelope(t, durations, beziers, num_states):        
    state = 0
    while state < num_states - 1:
        if t < durations[state]:
            break
        t -= durations[state]
        state += 1
    normalized_time = clamp(t / durations[state], 0.0, 1.0)
    return quadratic_bezier(normalized_time, beziers[state])

t = 0.0
dt = 0.01
durations = [0.333, 0.5, 3.0]
beziers = [
    [0.0, 0.0, 1.0],
    [1.0, 1.0, 1.0],
    [1.0, 0.0, 0.0]
]
num_states = 3

values = []

while t < 4.0:
    values.append(envelope(t, durations, beziers, num_states))
    t += dt

plt.plot(values)
plt.ylabel("Amplitude")
plt.xlabel("Sample")
plt.show()