import asyncio
import tkinter as tk
from tkinter import messagebox, scrolledtext
from bleak import BleakScanner, BleakClient
from threading import Thread
import sqlite3
from datetime import datetime
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from collections import defaultdict

class BleakApp:
    def __init__(self, master, loop):
        self.master = master
        self.loop = loop
        master.title("Bluetooth Communication with Device")
        self.database_setup()
        self.setup_connection_page()
        self.last_position = None
        self.setup_close_event()  # Setup close event binding
    
    def setup_close_event(self):
        self.master.protocol("WM_DELETE_WINDOW", self.on_close)

    def on_close(self):
        # Run the asynchronous return_to_home task
        asyncio.run(self.return_to_home())
        self.master.destroy()
    
    async def return_to_home(self):
        if self.last_position is None:
            dx, dy = 0, 0  # Sending absolute position for the first click
        else:
            dx, dy = 0 - self.last_position[0], 0 - self.last_position[1]

        home_command = f"move {dx} {dy}"
        data_to_send = home_command.encode('utf-8')
        write_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50200406e"  # Replace with your characteristic UUID
        try:
            await self.client.write_gatt_char(write_uuid, data_to_send, response=False)
            print("Device returning to home position (0,0)")
        except Exception as e:
            print(f"Failed to send home command: {str(e)}")


    
    def database_setup(self):
        # Connect to the SQLite database (the database file will be created if it does not exist)
        self.conn = sqlite3.connect('result.db')
        self.cursor = self.conn.cursor()
        # Create a table to store ADC values with a timestamp
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS adc_values (
                timestamp TEXT NOT NULL,
                x INTEGER NOT NULL,
                y INTEGER NOT NULL,
                adc_value REAL NOT NULL,
                angle REAL NOT NULL
            )
        ''')
        self.conn.commit()


    def setup_connection_page(self):
        self.connection_frame = tk.Frame(self.master)
        self.connection_frame.pack(padx=10, pady=10)

        tk.Label(self.connection_frame, text="Select a Bluetooth Device:").pack(pady=(0, 10))

        self.scan_button = tk.Button(self.connection_frame, text="Scan", command=self.initiate_scan)
        self.scan_button.pack()

        self.devices_listbox = tk.Listbox(self.connection_frame, width=50, height=10)
        self.devices_listbox.pack(pady=10)

        self.connect_button = tk.Button(self.connection_frame, text="Connect", command=self.connect_device)
        self.connect_button.pack(pady=(10, 0))

    def initiate_scan(self):
        asyncio.run_coroutine_threadsafe(self.scan_devices(), self.loop)

    async def scan_devices(self):
        self.devices_listbox.delete(0, tk.END)
        devices = await BleakScanner.discover()
        for device in devices:
            self.devices_listbox.insert(tk.END, f"{device.name} | {device.address}")

    def connect_device(self):
        selection = self.devices_listbox.curselection()
        if not selection:
            messagebox.showerror("Error", "Please select a device from the list.")
            return

        selected_device = self.devices_listbox.get(selection[0])
        address = selected_device.split('|')[1].strip()
        asyncio.run_coroutine_threadsafe(self.start_client(address), self.loop)

    async def start_client(self, address):
        client = BleakClient(address)
        try:
            await client.connect()
            if client.is_connected:
                self.client = client
                self.setup_communication_interface()
                self.setup_grid_selection()
                self.setup_results_display()
                self.setup_overview_button()
                await self.manage_device_communication()
            else:
                messagebox.showinfo("Connection Status", "Failed to connect to the device.")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))

    def setup_communication_interface(self):
        self.connection_frame.pack_forget()
        self.communication_frame = tk.Frame(self.master)
        self.communication_frame.pack(padx=10, pady=10)

        self.text_area = scrolledtext.ScrolledText(self.communication_frame, width=60, height=10)
        self.text_area.pack(pady=10)

        self.entry = tk.Entry(self.communication_frame, width=40)
        self.entry.pack(side=tk.LEFT, padx=(10, 0))

        self.send_button = tk.Button(self.communication_frame, text="Send", command=self.send_message)
        self.send_button.pack(side=tk.LEFT, padx=10)

        # self.up_button = tk.Button(self.communication_frame, text="Up", command=lambda: self.send_predefined_message("up"))
        # self.up_button.pack(side=tk.LEFT, padx=10)

        # self.down_button = tk.Button(self.communication_frame, text="Down", command=lambda: self.send_predefined_message("down"))
        # self.down_button.pack(side=tk.LEFT, padx=10)

        self.stop_button = tk.Button(self.communication_frame, text="Return", command=lambda: self.send_predefined_message("return"))
        self.stop_button.pack(side=tk.LEFT, padx=10)

        self.stop_button = tk.Button(self.communication_frame, text="Start", command=lambda: self.send_predefined_message("start"))
        self.stop_button.pack(side=tk.LEFT, padx=10)
    
    def send_predefined_message(self, message):
        self.append_text(f"Sent: {message}")
        data_to_send = message.encode('utf-8')
        write_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50200406e"
        
        # Send the "power" message immediately.
        asyncio.run_coroutine_threadsafe(
            self.client.write_gatt_char(write_uuid, data_to_send, response=False),
            self.loop
        )

        # If the message is "power", schedule to send "stop" after 20 seconds.
        # if message == "power":
        #     self.master.after(32500, self.send_stop_message)

    def send_stop_message(self):
        stop_message = "stop"
        self.append_text(f"Sent: {stop_message}")
        data_to_send = stop_message.encode('utf-8')
        write_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50200406e"
        
        # Send the "stop" message after a delay.
        asyncio.run_coroutine_threadsafe(
            self.client.write_gatt_char(write_uuid, data_to_send, response=False),
            self.loop
        )

    def setup_grid_selection(self):
        self.grid_frame = tk.Frame(self.master)
        self.grid_frame.pack(side=tk.TOP, padx=10, pady=10)

        # Description label with pack
        tk.Label(self.grid_frame, text="Select a cell to send the probe to the corresponding position:", font=('Helvetica', 10)).pack(pady=(0, 10))

        # Sub-frame for grid, using a different frame to use grid layout
        self.button_grid_frame = tk.Frame(self.grid_frame)
        self.button_grid_frame.pack(pady=(10, 10))  # Using pack for the frame that contains the grid

        # Creating a grid of buttons inside the sub-frame
        self.grid_buttons = {}
        self.selected_buttons = set()
        for i in range(7):
            for j in range(7):
                button = tk.Button(self.button_grid_frame, text=f'{i},{j}', width=6, height=2)
                button.grid(row=i, column=j, padx=2, pady=2)
                self.grid_buttons[(i, j)] = button
                button.config(command=lambda i=i, j=j, btn=button: self.handle_grid_click(i, j, btn))

    def handle_grid_click(self, i, j, button):
        # Check if it's the first click
        if self.last_position is None:
            # For the first click, treat the clicked position as relative to (0,0)
            dx, dy = i, j  # Sending absolute position for the first click
        else:
            # For subsequent clicks, calculate relative motion
            dx, dy = i - self.last_position[0], j - self.last_position[1]

        # Update the last position to the current position
        self.last_position = (i, j)

        # Change the button color to indicate it has been selected at least once
        if button not in self.selected_buttons:
            button.config(bg='lightblue')  # Change color to light blue
            self.selected_buttons.add(button)

        # Schedule the database record insertion and motion command sending
        asyncio.run_coroutine_threadsafe(self.send_motion_to_device(dx, dy), self.loop)


    async def send_motion_to_device(self, dx, dy):
        message = f'move {dx} {dy}'
        data_to_send = message.encode('utf-8')
        write_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50200406e"
        await self.client.write_gatt_char(write_uuid, data_to_send, response=False)
        self.append_text(f"Motion sent: {dx}, {dy}")


    def setup_results_display(self):
        self.results_frame = tk.Frame(self.master)
        self.results_frame.pack(side=tk.BOTTOM, padx=10, pady=10, fill=tk.X, expand=True)

        self.results_label = tk.Label(self.results_frame, text="Results (Time - ADC Value for each (x, y))")
        self.results_label.pack()

        self.results_text = scrolledtext.ScrolledText(self.results_frame, height=10)
        self.results_text.pack(fill=tk.BOTH, expand=True)


    async def notification_handler(self, sender, data):
        data_str = data.decode('utf-8').strip()
        # print(f"Received data: {data_str}")
        
        try:
            # Split data based on comma and remove any surrounding whitespace
            parts = [part.strip() for part in data_str.split(',')]
            if len(parts) != 2:
                raise ValueError(f"Expected 2 parts but got {len(parts)}: {parts}")

            adc_value, angle_value = map(float, parts)  # Convert both parts to float
            # print(adc_value, angle_value)

            # Add milliseconds to the timestamp
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

            if (adc_value * 6 * 9.81 > 90):
                print("Force is too Large... Return")
                message = "return"
                data_to_send = message.encode('utf-8')
                write_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50200406e"
                asyncio.run_coroutine_threadsafe(
                    self.client.write_gatt_char(write_uuid, data_to_send, response=False),
                    self.loop
                )
                self.append_text(f"Force is too Large... Return")

            else:
                await self.loop.run_in_executor(None, self.insert_db_record, timestamp, adc_value, angle_value*0.225)


        except ValueError as e:
            print(f"Error processing notification: {e}")
            self.append_text(f"Error processing notification: {e}")

    def insert_db_record(self, timestamp, adc_value, angle_value):
        try:
            # Perform the SQLite operations
            conn = sqlite3.connect('result.db')
            cursor = conn.cursor()
            cursor.execute('INSERT INTO adc_values (timestamp, x, y, adc_value, angle) VALUES (?, ?, ?, ?, ?)', 
                        (timestamp, self.last_position[0], self.last_position[1], adc_value, angle_value))
            conn.commit()
        except sqlite3.Error as e:
            # Log or handle the error
            print(f"Database error: {e}")
        finally:
            conn.close()

    async def manage_device_communication(self):
        notify_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50300406e"
        # Change 'notification_handler' to 'self.notification_handler'
        await self.client.start_notify(notify_uuid, self.notification_handler)
        self.append_text("Subscribed to notifications. Listening for messages from the device...")

    def append_text(self, text):
        # Ensure the UI is updated in a thread-safe way
        self.text_area.after(0, lambda: self.text_area.insert(tk.END, f"{text}\n"))
        self.text_area.after(0, lambda: self.text_area.see(tk.END))
    
    def setup_results_display(self):
        self.results_frame = tk.Frame(self.master)
        self.results_frame.pack(side=tk.BOTTOM, padx=10, pady=10, fill=tk.X, expand=True)

        # Variable to hold the currently selected (x, y) pair
        self.selected_xy = tk.StringVar(self.master)
        self.selected_xy.set('Select Coordinates')  # default value

        # Dropdown menu to select (x, y) pair
        self.xy_menu = tk.OptionMenu(self.results_frame, self.selected_xy, 'Select Coordinates')
        self.xy_menu.pack(pady=10)

        # Attach an event to the dropdown to refresh the list whenever it's clicked
        self.xy_menu.bind('<Button-1>', self.update_dropdown)

        self.plot_button = tk.Button(self.results_frame, text="Plot ADC Values", command=self.plot_adc_values)
        self.plot_button.pack()

    def update_dropdown(self, event=None):
        # Fetch unique (x, y) pairs from the database to populate the dropdown
        connection = sqlite3.connect('result.db')
        cursor = connection.cursor()
        cursor.execute('SELECT DISTINCT x, y FROM adc_values ORDER BY x, y')
        coordinates = cursor.fetchall()
        connection.close()

        # Clear the current options in the OptionMenu
        menu = self.xy_menu["menu"]
        menu.delete(0, "end")
        for coordinate in coordinates:
            menu.add_command(label=coordinate, command=lambda value=coordinate: self.selected_xy.set(value))

        # If no data is available, set to default value
        if not coordinates:
            self.selected_xy.set('Select Coordinates')

    def plot_adc_values(self):
        selected_pair = self.selected_xy.get()
        if selected_pair == 'Select Coordinates':
            messagebox.showerror("Selection Error", "Please select valid coordinates.")
            return

        # Extract x, y from the selected pair string
        x, y = map(int, selected_pair.strip('()').split(','))

        # Fetch data from the database for the selected (x, y) pair
        connection = sqlite3.connect('result.db')
        cursor = connection.cursor()
        # Fetching both adc_value and angle
        cursor.execute('SELECT timestamp, adc_value, angle FROM adc_values WHERE x=? AND y=? ORDER BY timestamp', (x, y))
        records = cursor.fetchall()
        connection.close()

        if not records:
            messagebox.showinfo("No Data", "No data available for this position.")
            return

        # Prepare data for plotting
        timestamps = [datetime.strptime(record[0], '%Y-%m-%d %H:%M:%S.%f') for record in records]
        adc_values = [record[1] * 6 * 9.81 for record in records]
        angles = [record[2] for record in records]

        # Calculate depth from angle
        depths = [(angle / 360) * 0.5 for angle in angles]

        # Plotting the data
        plt.figure(figsize=(10, 5))
        plt.plot(depths, adc_values, marker='o', linestyle='-')
        # plt.title(f'ADC Values Over Depth for Position ({x}, {y})')
        plt.title(f'Force (N) Over Depth for Position ({x}, {y})')
        plt.xlabel('Depth (cm)')
        # plt.ylabel('ADC Value')
        plt.ylabel('Force Value (N)')
        plt.grid(True)
        plt.tight_layout()
        plt.show()
    
    def setup_overview_button(self):
        # Adding a frame for the plot button to keep the layout organized
        self.plot_frame = tk.Frame(self.master)
        self.plot_frame.pack(padx=10, pady=10, fill=tk.X)

        # Button for plotting the overview of all data
        self.overview_plot_button = tk.Button(self.plot_frame, text="Plot Data Overview", command=self.plot_overview)
        self.overview_plot_button.pack()

    # def plot_overview(self):
    #     # Connect to the database and fetch all data
    #     conn = sqlite3.connect('adc_data.db')
    #     cursor = conn.cursor()
    #     cursor.execute('SELECT x, y, adc_value, angle FROM adc_values')
    #     data = cursor.fetchall()
    #     conn.close()

    #     if not data:
    #         messagebox.showinfo("No Data", "No data available to plot.")
    #         return

    #     # Prepare data for plotting
    #     xs, ys, zs, colors = [], [], [], []
    #     for x, y, adc, angle in data:
    #         depth = (angle / 360) * 0.5  # Calculate depth based on angle
    #         xs.append(x)
    #         ys.append(y)
    #         zs.append(depth)
    #         colors.append(adc*120)  # Use ADC value as color scale

    #     # Create a 3D scatter plot
    #     fig = plt.figure(figsize=(10, 8))
    #     ax = fig.add_subplot(111, projection='3d')

    #     scatter = ax.scatter(xs, ys, zs, c=colors, cmap='viridis', marker='o', edgecolor='k', s=50, depthshade=True)
    #     # fig.colorbar(scatter, ax=ax, label='ADC Value')
    #     fig.colorbar(scatter, ax=ax, label='Force Value')

    #     ax.set_xlabel('X Position')
    #     ax.set_ylabel('Y Position')
    #     ax.set_zlabel('Depth (cm)')
    #     # plt.title('Overview of ADC Values Across Positions and Depths')
    #     plt.title('Overview of Force Values Across Positions and Depths')
        
    #     ax.invert_xaxis()
    #     plt.xticks(range(7))
    #     plt.yticks(range(7))

    #     plt.show()
    def plot_overview(self):
        # Connect to the database and fetch all data
        conn = sqlite3.connect('result.db')
        cursor = conn.cursor()
        cursor.execute('SELECT x, y, adc_value, angle FROM adc_values')
        data = cursor.fetchall()
        conn.close()

        if not data:
            messagebox.showinfo("No Data", "No data available to plot.")
            return

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

    def send_message(self):
        message = self.entry.get()
        if message.lower() == 'exit':
            self.master.quit()
            return
        self.entry.delete(0, tk.END)
        data_to_send = message.encode('utf-8')
        write_uuid = "9ecadc24-0ee5-a9e0-93f3-a3b50200406e"
        asyncio.run_coroutine_threadsafe(
            self.client.write_gatt_char(write_uuid, data_to_send, response=False),
            self.loop
        )
        self.append_text(f"Sent: {message}")

    def append_text(self, text):
        self.text_area.insert(tk.END, f"{text}\n")
        self.text_area.see(tk.END)

def run_tkinter_loop(root):
    root.mainloop()

def main():
    root = tk.Tk()
    loop = asyncio.new_event_loop()
    app = BleakApp(root, loop)

    loop_thread = Thread(target=loop.run_forever)
    loop_thread.start()

    run_tkinter_loop(root)
    loop.call_soon_threadsafe(loop.stop)
    loop_thread.join()

if __name__ == "__main__":
    main()
