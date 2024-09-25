import FourierSolver as fs
import numpy as np
import math
from enum import Enum

class Type(Enum):
    DFT = 1
    SEMI_FAST_FOURIER = 2
    FFT = 1

# 5    
def svertka(a, b):
    c = (len(a) + len(b) - 1) * [0]
    for i in range(len(a)):
        for j in range(len(b)):            
            c[i + j] += (a[i] * b[j])
    return c
#6 7 8
def svertka_fourier(a, b, type : Type):

    
    n = len(a) + len(b) - 1
    N = 2 ** math.ceil(math.log2(n))  # ближайшая степень двойки
    a += [0] * (N - len(a))
    b += [0] * (N - len(b))

    if type == Type.DFT:
        fft_a = fs.FourierSolver.DiscreteFourierTransform(a)
        fft_b = fs.FourierSolver.DiscreteFourierTransform(b)
        fft_c = [fft_a[i] * fft_b[i] for i in range(N)]
        c = fs.FourierSolver.inverseFourierTransform(fft_c)
    elif type == Type.SEMI_FAST_FOURIER:
        fft_a = fs.FourierSolver.Semifast_Fourier(a)
        fft_b = fs.FourierSolver.Semifast_Fourier(b)
        fft_c = [fft_a[i] * fft_b[i] for i in range(N)]
        c = fs.FourierSolver.Inverse_Semifast_Fourier(fft_c)
    else:
        fft_a = fs.FourierSolver.fft(a)
        fft_b = fs.FourierSolver.fft(b)
        fft_c = [fft_a[i] * fft_b[i] for i in range(N)]
        c = fs.FourierSolver.inverse_fft(fft_c)

    return c

a = [1, 2, 3, 4, 5]
b = [1, 2, 3]  

result = svertka(a[:], b[:])
result_dft = svertka_fourier(a[:], b[:], Type.DFT)
result_sfft = svertka_fourier(a[:], b[:], Type.SEMI_FAST_FOURIER)
result_fft = svertka_fourier(a[:], b[:], Type.FFT)

print("numpy convolution result:", np.convolve(a, b))

print("regular convolution result: ", result)

print ("dft convolution result: ")
for i, val in enumerate(result_dft):
    print(f"\tX[{i}] = {val}")
print ("sfft convolution result: ")
for i, val in enumerate(result_sfft):
    print(f"\tX[{i}] = {val}")
print ("fft convolution result: ")
for i, val in enumerate(result_fft):
    print(f"\tX[{i}] = {val}")