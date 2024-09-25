import math
import numpy as np
import cmath
import ImNum as imnn



class FourierSolver:
    def DiscreteFourierTransform(x):
        N = len(x)
        X = [imnn.ImaginaryNum(0, 0)] * N
        
        for k in range(N):
            for n in range(N):
                exp = -2 * math.pi * k * n / N
                X[k] += imnn.ImaginaryNum.exp(exp) * x[n]
        return X

    def inverseFourierTransform(x):
        N = len(x)
        X = [imnn.ImaginaryNum(0, 0)] * N
        
        for k in range(N):
            for n in range(N):
                exp = 2 * math.pi * k * n / N
                X[k] += imnn.ImaginaryNum.exp(exp) * x[n] / N
        return X



    def Semifast_Fourier(f):
        N = len(f)
        p1 = 1
        p2 = N
        for i in np.arange(1, math.sqrt(N) + 1, 1): # ищем факторы количества сигналов N, пока не найдем пару с минимальным различием p1 p2
            if (N % i == 0) and ((N / i - i) < (N - 1)):
                p1 = int(i) 
                p2 = int(N / p1)


        A1 = [imnn.ImaginaryNum(0,0)] * N
        i = imnn.ImaginaryNum(0, 1)
        for k1 in range(p1):
            for j2 in range(p2):
                for j1 in range(p1):
                    A1[k1 + p1 * j2] += imnn.ImaginaryNum.exp(i * (-2) * math.pi * j1 * k1 / p1) * f[j2 + p2 * j1]
                A1[k1 + p1 * j2] /= p1
        A2 = [imnn.ImaginaryNum(0,0)] * N
        for k1 in range(p1):
            for k2 in range(p2):
                for j2 in range(p2):
                    A2[k1 + p1 * k2] += A1[k1 + p1 * j2] * imnn.ImaginaryNum.exp(i * (-2) * math.pi * (  j2 / (p1 * p2) * (k1 + p1 * k2)))
                A2[k1 + p1 * k2] /= p2 / N

        return A2

    def Inverse_Semifast_Fourier(f):
        N = len(f)
        p1 = 1
        p2 = N
        for i in np.arange(1, math.sqrt(N) + 1, 1): # ищем факторы количества сигналов N, пока не найдем пару с минимальным различием p1 p2
            if (N % i == 0) and ((N / i - i) < (N - 1)):
                p1 = int(i) 
                p2 = int(N / p1)

        A1 = [imnn.ImaginaryNum(0,0)] * N
        i = imnn.ImaginaryNum(0, 1)
        for k1 in range(p1):
            for j2 in range(p2):
                for j1 in range(p1):
                    A1[k1 + p1 * j2] += imnn.ImaginaryNum.exp(i * 2 * math.pi * j1 * k1 / p1) * f[j2 + p2 * j1]
        A2 = [imnn.ImaginaryNum(0,0)] * N
        for k1 in range(p1):
            for k2 in range(p2):
                for j2 in range(p2):
                    A2[k1 + p1 * k2] += A1[k1 + p1 * j2] * imnn.ImaginaryNum.exp(i * 2 * math.pi * (  j2 / (p1 * p2) * (k1 + p1 * k2)))
                A2[k1 + p1 * k2] /= N
        return A2
    
    def fft(A):
        n = len(A)
        # добиваем до степени двоийки
        r = int(math.ceil(math.log2(n)))
        N = 2 ** r
        A += [0] * (N - n)
        n = N

        if n == 1:
            return A

        h = A[::2]  
        g = A[1::2]  

        fft_h = FourierSolver.fft(h)
        fft_g = FourierSolver.fft(g)

        result = [0] * n
        i = imnn.ImaginaryNum(0, 1)
        for k in range(n // 2):
            exp_factor = imnn.ImaginaryNum.exp(i * -2 * math.pi * k / n)
            result[k] = exp_factor * fft_g[k] + fft_h[k] 
            result[k + n // 2] = exp_factor * fft_g[k] * -1 + fft_h[k]

        return result
    def inverse_fft(A, flag = 0):
        n = len(A)
        # добиваем до степени двойки
        r = int(math.ceil(math.log2(n)))
        N = 2 ** r
        A += [0] * (N - n)
        n = N

        if n == 1:
            return A

        h = A[::2]  
        g = A[1::2]  

        fft_h = FourierSolver.inverse_fft(h, 1)
        fft_g = FourierSolver.inverse_fft(g, 1)

        result = [0] * n
        i = imnn.ImaginaryNum(0, 1)
        for k in range(n // 2):
            exp_factor = imnn.ImaginaryNum.exp(i * 2 * math.pi * k / n)
            result[k] = exp_factor * fft_g[k] + fft_h[k]
            result[k + n // 2] = exp_factor * fft_g[k] * -1 + fft_h[k]
        if flag == 0:
            for k in range(n):
                result[k] /= n
        return result
    


if __name__ == "__main__":
    input_signal = [3,0,2,2,2,3,4,2,4,5]
    dft_result = FourierSolver.DiscreteFourierTransform(input_signal[:])
    ppf_result = FourierSolver.Semifast_Fourier(input_signal[:])
    fft_result = FourierSolver.fft(input_signal[:])

    print("\tdft_results")
    for i, val in enumerate(dft_result):
        print(f"X[{i}] = {val}")
    print("\tppf_results")
    for i, val in enumerate(ppf_result):
        print(f"X[{i}] = {val}")
    print("\tfft_results")
    for i, val in enumerate(fft_result):
        print(f"X[{i}] = {val}")

    print("inverse_results")
    print("\tdft_results\n")
    inverse_result = FourierSolver.inverseFourierTransform(dft_result)
    for i, val in enumerate(inverse_result):
        print(f"X[{i}] = {val}")
    print("\n\tppf_results\n")
    inverse_result = FourierSolver.Inverse_Semifast_Fourier(ppf_result)
    for i, val in enumerate(inverse_result):
        print(f"X[{i}] = {val}")
    print("\n\tfft_results")
    inverse_result = FourierSolver.inverse_fft(fft_result)
    for i, val in enumerate(inverse_result):
        print(f"X[{i}] = {val}")

