import tkinter as tk
from tkinter import messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
from math import log2
from threading import Thread
import time
import psutil

import globals

class GUI:
    def __init__(self, root):
        self.root = root
        self.root.title("VanitySearch Monitor for TTD Client")
        
        self.percent_label = tk.Label(root, text=f"Range Progress: {globals.percent_complete}%", font=("Helvetica", 20))
        self.percent_label.pack(side=tk.TOP, pady=10)
        
        keys_temp = int(globals.percent_complete*globals.KEYS_PER_RANGE/100)
        if keys_temp <= 0: keys_temp = 1 # avoid inputting logarithm argument outside of its domain
        self.progress_label = tk.Label(root, text=f"Keys Checked: ~{keys_temp} (2^{round(log2(keys_temp), 2)}) / {globals.KEYS_PER_RANGE} (2^{round(log2(globals.KEYS_PER_RANGE), 2)})", font=("Helvetica", 14))
        self.progress_label.pack(side=tk.TOP)
        
        # create matplotlib figure and axes
        self.figure, self.ax = plt.subplots()
        self.canvas = FigureCanvasTkAgg(self.figure, master=root)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)
        
        # frame for process name input
        self.control_frame = tk.Frame(root)
        self.control_frame.pack(side=tk.TOP, fill=tk.X)
        
        self.label = tk.Label(self.control_frame, text="TTD Client Process Name (ex. ttdclientVS66.7.exe):")
        self.label.pack(side=tk.LEFT)
        
        self.process_entry = tk.Entry(self.control_frame)
        self.process_entry.pack(side=tk.LEFT)
        
        # look for running process that may be ttdclient to fill process entry by default
        for proc in psutil.process_iter(['pid', 'name']):
            try:
                proc_name = proc.info['name']
                if proc_name.startswith("ttdclient"):
                    self.process_entry.insert('end', proc_name)
                    messagebox.showinfo("Alert", "Detected TTD Client. If the process name is correct, press begin.")
                    break
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass
        
        self.apply_button = tk.Button(self.control_frame, text="Begin", command=self.apply_init_process)
        self.apply_button.pack(side=tk.LEFT)
        
        # frame for update interval input
        self.control_frame = tk.Frame(root)
        self.control_frame.pack(side=tk.BOTTOM, fill=tk.X)
        
        self.label = tk.Label(self.control_frame, text="Update Interval [Default is 500, 50-150 is recommended] (>49 ms):")
        self.label.pack(side=tk.LEFT)
        
        self.interval_entry = tk.Entry(self.control_frame)
        self.interval_entry.pack(side=tk.LEFT)
        self.interval_entry.insert(0, "500")  # default value
        
        self.apply_button = tk.Button(self.control_frame, text="Apply", command=self.apply_interval)
        self.apply_button.pack(side=tk.LEFT)
        
        self.x_data = []
        self.y_data = []
        self.start_time = time.time()
        
        self.update_interval = 500  # default update interval, 50-150 is best but 500 means no changes
                                    # have to occur in VanitySearch at this program's runtime
        
        # labels and title for the plot
        self.ax.set_xlabel('Time (s)')
        self.ax.set_ylabel('Key Rate (BK/s)')
        self.ax.set_title('Key Rate (BK/s) vs Time (s)')
        
        # plot line
        self.plot, = self.ax.plot([], [], 'r-')
        
        # begin plot & percent complete updating processes
        self.update_gui()
        
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing) # on close
    
    def update_gui(self):
        current_time = time.time() - self.start_time
        self.x_data.append(current_time)
        self.y_data.append(globals.key_rate)  #
        
        # Update plot data
        self.plot.set_data(self.x_data, self.y_data)
        
        # adjust x-axis to show the last 15 seconds
        if current_time > 15:
            self.ax.set_xlim(current_time - 15, current_time)
        else:
            self.ax.set_xlim(0, 15)
        
        self.ax.set_ylim(0, 6)
        
        # redraw figure
        self.ax.figure.canvas.draw()
        
        # update percent complete & progress
        if globals.percent_complete > 100.0: # sometimes it will go slightly above 100
            self.percent_label.config(text=f"Range Progress: {100}%")
            self.progress_label.config(text=f"Keys Checked: {globals.KEYS_PER_RANGE} (2^{round(log2(globals.KEYS_PER_RANGE), 2)}) / {globals.KEYS_PER_RANGE} (2^{round(log2(globals.KEYS_PER_RANGE), 2)})")
        else:
            keys_temp = int(globals.percent_complete*globals.KEYS_PER_RANGE/100)
            if keys_temp <= 0: keys_temp = 1 # avoid inputting logarithm argument outside of its domain
            self.percent_label.config(text=f"Range Progress: {globals.percent_complete}%")
            self.progress_label.config(text=f"Keys Checked: {keys_temp} (2^{round(log2(keys_temp), 2)}) / {globals.KEYS_PER_RANGE} (2^{round(log2(globals.KEYS_PER_RANGE), 2)})")
            
        # schedule next update
        self.root.after(self.update_interval, self.update_gui)
    
    def apply_interval(self):
        try:
            new_interval = int(self.interval_entry.get())
            if new_interval < 50: # 50ms minimum
                messagebox.showinfo("Alert", "Update interval must be an integer greater than 49!")
                self.interval_entry.delete(0, 'end')
                self.interval_entry.insert(0, str(self.update_interval)) # revert back
                return
            self.update_interval = new_interval
            globals.conn.change_update_delay(new_interval)
        except ValueError:
            messagebox.showinfo("Alert", "Update interval must be an integer greater than 49!")
            self.interval_entry.delete(0, 'end')
            self.interval_entry.insert(0, str(self.update_interval)) # revert back
    
    def apply_init_process(self):
        if self.process_entry.get()[-4:] != ".exe":
            messagebox.showinfo("Alert", "Process name must match what is on the disk. It must end in '.exe'!")
            return
        globals.conn.init_process(self.process_entry.get())
    
    def on_closing(self):
        # close server
        for proc in psutil.process_iter(['pid', 'name']):
            try:
                if proc.info['name'] == globals.server_process_name:
                    proc.terminate()
                    proc.wait()
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass
    
        self.root.quit()
        self.root.destroy()

def gui():
    root = tk.Tk()
    app = GUI(root)
    root.mainloop()

def initialize_globals_and_start_server():
    if not globals.global_init():
        exit(0)
        
    subprocess.Popen(globals.server_process_name, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    globals.conn.connect()
    Thread(target=globals.conn.maintain_globals_t).start()

if __name__ == "__main__":
    if not globals.global_init():
        exit(0)

    globals.conn.connect()
    Thread(target=globals.conn.maintain_globals_t).start()
    gui()