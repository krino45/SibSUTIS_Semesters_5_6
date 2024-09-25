def quick_mul(x: int, y: int) -> int:
    if x < 10 or y < 10:
        return x * y
    
    n = max(len(str(x)), len(str(y)))
    at = n // 2 # делим числа на половики (например, x = 123456, y = 4567, n = 6, at = 3)
    a = x // 10**at # a = 123
    b = x % 10**at # b = 456
    c = y // 10**at # c = 456
    d = y % 10**at # d = 7

    ac = quick_mul(a, c)
    bd = quick_mul(b, d)
    ab__cd = quick_mul(a + b, c + d)


    return (ac * 10 ** (2 * at)) + ((ab__cd - ac - bd) * 10 ** at) + bd # веселуха


def stolbik_help(): 
    # 2.244560100021772 for stolbik
    # 0.0059362000320106745 for normal eval
    # 2.407247200026177 for quick eval
    n = 231513242351345325432532453245612346324532452345345432
    m = 2315132423513616543324532453245432543253245
    for i in range(n, m):
            j = m + n - i
            quick_mul(i, j)

def stolbik_help2():
    # 2.1132480000378564 for stolbik
    # 0.03409939992707223 for normal eval
    # 3.1679512999253348 for quick eval
    n = 123460
    m = 165430
    for i in range(n, m):
            j = m + n - i
            quick_mul(i, j)

import timeit
print (min(timeit.Timer(stolbik_help).timeit(number=5, repeat=5)))        
print (quick_mul(15364384285324853222,321546234632462344213))
print (15364384285324853222 * 321546234632462344213)