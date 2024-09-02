import matplotlib.pyplot as plt
import numpy as np

# Assume a is already calculated as shown above
# Convert a's values to forces in Newtons using the provided formula
a = [0.87, 2, 3.515, 4.59, 5.545, 7.495]
a = [c * 9.81 for c in a]  # this assumes 'a' is defined earlier with force data in kg

# b is given directly as force in Newtons
b = [8, 20, 35, 45, 54, 73]


# Assuming 'a' and 'b' are defined as numpy arrays
# If they are lists, convert them first:
a = np.array(a)  # Convert a's values to forces in Newtons using the provided formula
b = np.array([8, 20, 35, 45, 54, 73])

relative_errors = np.abs(a - b) / np.abs(a)


# Calculate and print mean relative error to summarize overall error across the dataset
mean_relative_error = np.mean(relative_errors)
print("Mean Relative Error:", mean_relative_error)



# Plotting
plt.figure(figsize=(10, 6))
plt.plot(a, label='Standard Force', marker='o')
plt.plot(b, label='Detected Force by Force Sensor', marker='x')
plt.xlabel('Index')
plt.ylabel('Force in Newtons')
plt.title('Comparison of Standard vs. Detected Force')
plt.legend()
plt.grid(True)
plt.show()


