import random
import csv

data = []

# Normal (0)
for _ in range(500):
    temp = random.randint(25, 32)
    hum = random.randint(55, 74)
    data.append([temp, hum, 0])

# Hot (1)
for _ in range(500):
    temp = random.randint(34, 40)
    hum = random.randint(55, 74)
    data.append([temp, hum, 1])

# Humid (2)
for _ in range(500):
    temp = random.randint(25, 32)
    hum = random.randint(76, 95)
    data.append([temp, hum, 2])

with open("dataset.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["Temperature", "Humidity", "Label"])
    writer.writerows(data)

print("Dataset created!")
