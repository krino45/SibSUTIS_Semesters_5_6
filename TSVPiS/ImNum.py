import math

class ImaginaryNum:
    def __init__(self, a, b):
        self.a = a
        self.b = b

    def __add__(self, other):
        if isinstance(other, ImaginaryNum):
            return ImaginaryNum(self.a + other.a, self.b + other.b)
        elif isinstance(other, (float, int)):
            return ImaginaryNum(self.a + other, self.b)
        else:
            raise TypeError("Unsupported operand type")

    def __sub__(self, other):
        if isinstance(other, ImaginaryNum):
            return ImaginaryNum(self.a - other.a, self.b - other.b)
        elif isinstance(other, (float, int)):
            return ImaginaryNum(self.a - other, self.b)
        else:
            raise TypeError("Unsupported operand type")

    def __mul__(self, other):
        if isinstance(other, ImaginaryNum):
            return ImaginaryNum(self.a * other.a - self.b * other.b, self.a * other.b + self.b * other.a)
        elif isinstance(other, (float, int)):
            return ImaginaryNum(self.a * other, self.b * other)
        else:
            raise TypeError("Unsupported operand type")


    def __truediv__(self, other):
        if isinstance(other, ImaginaryNum):
            return ImaginaryNum((self.a * other.a + self.b * other.b) / (other.a * other.a + other.b * other.b), (self.b * other.a - self.a * other.b) / (other.a * other.a + other.b * other.b))
        elif isinstance(other, (float, int)):
            return ImaginaryNum(self.a / other, self.b / other)
        else:
            raise TypeError("Unsupported operand type")
        
    def __str__(self):
        if self.b < 0.0000001 and self.b > -0.0000001:
            return f"{self.a:.4f}"
        elif self.a < 0.0000001 and self.a > -0.0000001:
            return f"{self.b:.4f}i"
        else:
            return f"{self.a:.4f} + {self.b:.4f}i"
    
    def mod(self):
        return (self.a ** 2 + self.b ** 2) ** 0.5
    def arg(self):
        return math.atan2(self.b, self.a)
    
    @staticmethod
    def exp(i) -> 'ImaginaryNum': # e^ix = cos(x) + i*sin(x)
        if isinstance(i, ImaginaryNum):
            a = i.a
            b = i.b
            return ImaginaryNum(math.exp(a) * math.cos(b), math.exp(a) * math.sin(b))
        else:
            return ImaginaryNum(math.cos(i), math.sin(i))