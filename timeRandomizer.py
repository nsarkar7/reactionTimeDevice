import random

prevTime = 0
times = []

for i in range(10):
    randomNext = random.randint(30,45) + prevTime
    times.append(randomNext)
    prevTime = randomNext

print(times)