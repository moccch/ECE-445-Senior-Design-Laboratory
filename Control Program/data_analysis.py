import sqlite3
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from collections import defaultdict

def plot_overview():
        # Connect to the database and fetch all data
        conn = sqlite3.connect('result.db')
        cursor = conn.cursor()
        cursor.execute('SELECT x, y, adc_value, angle FROM adc_values')
        data = cursor.fetchall()
        conn.close()

        # if not data:
        #     messagebox.showinfo("No Data", "No data available to plot.")
        #     return

        # Group data by (x, y)
        grouped = defaultdict(list)
        for x, y, adc, angle in data:
            grouped[(x, y)].append((adc, angle))

        # Prepare data for plotting
        xs, ys, zs, colors = [], [], [], []
        batch_size = 10  # Define the batch size for averaging

        # Process each group
        for (x, y), points in grouped.items():
            # Process each batch in the group
            for i in range(0, len(points), batch_size):
                batch = points[i:i+batch_size]
                avg_adc = sum(p[0] for p in batch) / len(batch)
                avg_angle = sum(p[1] for p in batch) / len(batch)
                depth = (avg_angle / 360) * 0.5  # Calculate depth based on average angle

                xs.append(x)
                ys.append(y)
                zs.append(depth)
                colors.append(avg_adc * 6 * 9.81)  # Use averaged ADC value as color scale

        # Create a 3D scatter plot
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')
        scatter = ax.scatter(xs, ys, zs, c=colors, cmap='viridis', marker='o', edgecolor='k', s=50, depthshade=True)

        fig.colorbar(scatter, ax=ax, label='Force Value (N)')
        ax.set_xlabel('X Position')
        ax.set_ylabel('Y Position')
        ax.set_zlabel('Depth (cm)')
        plt.title('Overview of Force Values (N) Across Every 10 Positions and Depths')
        ax.invert_xaxis()
        plt.xticks(range(7))
        plt.yticks(range(7))

        plt.show()

plot_overview()
