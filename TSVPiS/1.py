def bubble_sort(a):
    n = len(a)
    for i in range(n):
        for j in range(0, n - i - 1):
            if a[j] > a[j + 1]:
                a[j], a[j + 1] = a[j + 1], a[j]
    return a

def select_sort(a):
    n = len(a)
    for i in range(n):
        min = i
        for j in range(i + 1, n):
            if a[j] < a[min]:
                min = j
        a[i], a[min] = a[min], a[i]
    return a

def merge(l, r):
    res = []
    i = j = 0
    while i < len(l) and j < len(r):
        if l[i] < r[j]:
            res.append(l[i])
            i += 1
        else:
            res.append(r[j])
            j += 1
    res.extend(l[i:])
    res.extend(r[j:])
    return res

def merge_sort(a):
    n = len(a)
    w = 1

    while w < n:
        for i in range(0, n, w * 2):
            l = a[i:i + w]
            r = a[i + w:i + w * 2]
            print("l: ", l)
            print("r: ", r)
            res = merge(l, r)
            print("res: ", res)
            a[i:i + len(res)] = res
            print(a)
        w *= 2

    return a


array1 = [10, 9, 8, 7, 15, 5, 4, 3, 2, 6, 1, 0]
array2 = [10, 9, 8, 7, 15, 5, 4, 3, 6, 2, 1, 0]
array3 = [10, 9, 8, 7, 15, 5, 4, 6, 3, 2, 1, 0]
print(bubble_sort(array1))
print(select_sort(array2))
merge_sort(array3)