import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import subprocess
import json
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

class CPUSchedulerSimulator:
    def __init__(self, root):
        self.root = root
        self.root.title("CPU Scheduling Simulator")
        self.root.geometry("1200x800")
        
        self.processes = []
        self.compiled = False
        
        self.setup_ui()
        self.compile_scheduler()
    
    def setup_ui(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        
        title_label = ttk.Label(main_frame, text="CPU Scheduling Simulator", 
                                font=('Arial', 20, 'bold'))
        title_label.grid(row=0, column=0, columnspan=2, pady=10)
        
        input_frame = ttk.LabelFrame(main_frame, text="Process Input", padding="10")
        input_frame.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        
        ttk.Label(input_frame, text="Process ID:").grid(row=0, column=0, sticky=tk.W, pady=2)
        self.pid_entry = ttk.Entry(input_frame, width=15)
        self.pid_entry.grid(row=0, column=1, pady=2, padx=5)
        
        ttk.Label(input_frame, text="Arrival Time:").grid(row=1, column=0, sticky=tk.W, pady=2)
        self.arrival_entry = ttk.Entry(input_frame, width=15)
        self.arrival_entry.grid(row=1, column=1, pady=2, padx=5)
        
        ttk.Label(input_frame, text="Burst Time:").grid(row=2, column=0, sticky=tk.W, pady=2)
        self.burst_entry = ttk.Entry(input_frame, width=15)
        self.burst_entry.grid(row=2, column=1, pady=2, padx=5)
        
        ttk.Label(input_frame, text="Priority (1=high):").grid(row=3, column=0, sticky=tk.W, pady=2)
        self.priority_entry = ttk.Entry(input_frame, width=15)
        self.priority_entry.grid(row=3, column=1, pady=2, padx=5)
        
        btn_frame = ttk.Frame(input_frame)
        btn_frame.grid(row=4, column=0, columnspan=2, pady=10)
        
        ttk.Button(btn_frame, text="Add Process", command=self.add_process).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Clear All", command=self.clear_processes).pack(side=tk.LEFT, padx=5)
        
        process_list_frame = ttk.LabelFrame(input_frame, text="Process List", padding="5")
        process_list_frame.grid(row=5, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        self.process_listbox = tk.Listbox(process_list_frame, height=8, width=40)
        self.process_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(process_list_frame, orient=tk.VERTICAL, command=self.process_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.process_listbox.config(yscrollcommand=scrollbar.set)
        
        algorithm_frame = ttk.LabelFrame(main_frame, text="Algorithm Selection", padding="10")
        algorithm_frame.grid(row=2, column=0, sticky=(tk.W, tk.E), padx=5, pady=5)
        
        ttk.Label(algorithm_frame, text="Select Algorithm:").grid(row=0, column=0, sticky=tk.W, pady=2)
        self.algorithm_var = tk.StringVar(value="FCFS")
        algorithms = [
            ("FCFS - First Come First Serve", "FCFS"),
            ("SJF (Non-Preemptive)", "SJF_NP"),
            ("SJF (Preemptive)", "SJF_P"),
            ("Priority (Non-Preemptive)", "PRIORITY_NP"),
            ("Priority (Preemptive)", "PRIORITY_P"),
            ("Round Robin", "RR")
        ]
        
        for i, (text, value) in enumerate(algorithms):
            ttk.Radiobutton(algorithm_frame, text=text, variable=self.algorithm_var, 
                           value=value).grid(row=i+1, column=0, sticky=tk.W, pady=2)
        
        ttk.Label(algorithm_frame, text="Time Quantum (for RR):").grid(row=7, column=0, sticky=tk.W, pady=2)
        self.quantum_entry = ttk.Entry(algorithm_frame, width=10)
        self.quantum_entry.insert(0, "2")
        self.quantum_entry.grid(row=7, column=1, pady=2, padx=5, sticky=tk.W)
        
        ttk.Button(algorithm_frame, text="Run Simulation", 
                  command=self.run_simulation, 
                  style='Accent.TButton').grid(row=8, column=0, columnspan=2, pady=15)
        
        results_frame = ttk.LabelFrame(main_frame, text="Results", padding="10")
        results_frame.grid(row=1, column=1, rowspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(1, weight=1)
        
        self.canvas_frame = ttk.Frame(results_frame)
        self.canvas_frame.pack(fill=tk.BOTH, expand=True)
        
        metrics_frame = ttk.Frame(results_frame)
        metrics_frame.pack(fill=tk.X, pady=5)
        
        self.metrics_label = ttk.Label(metrics_frame, text="", font=('Arial', 10))
        self.metrics_label.pack()
        
        self.results_text = scrolledtext.ScrolledText(results_frame, height=10, width=70)
        self.results_text.pack(fill=tk.BOTH, expand=True, pady=5)
    
    def compile_scheduler(self):
        try:
            result = subprocess.run(['gcc', '-o', 'scheduler', 'scheduler.c'], 
                                  capture_output=True, text=True, timeout=10)
            if result.returncode == 0:
                self.compiled = True
            else:
                messagebox.showerror("Compilation Error", 
                                   f"Failed to compile scheduler:\n{result.stderr}")
        except Exception as e:
            messagebox.showerror("Error", f"Compilation failed: {str(e)}")
    
    def add_process(self):
        try:
            pid = int(self.pid_entry.get())
            arrival = int(self.arrival_entry.get())
            burst = int(self.burst_entry.get())
            priority = int(self.priority_entry.get())
            
            if burst <= 0:
                messagebox.showerror("Error", "Burst time must be positive!")
                return
            
            process = {
                'pid': pid,
                'arrival': arrival,
                'burst': burst,
                'priority': priority
            }
            self.processes.append(process)
            
            self.process_listbox.insert(tk.END, 
                f"P{pid}: AT={arrival}, BT={burst}, Priority={priority}")
            
            self.pid_entry.delete(0, tk.END)
            self.arrival_entry.delete(0, tk.END)
            self.burst_entry.delete(0, tk.END)
            self.priority_entry.delete(0, tk.END)
            
            self.pid_entry.insert(0, str(pid + 1))
            
        except ValueError:
            messagebox.showerror("Error", "Please enter valid numbers!")
    
    def clear_processes(self):
        self.processes = []
        self.process_listbox.delete(0, tk.END)
        self.pid_entry.delete(0, tk.END)
        self.pid_entry.insert(0, "1")
    
    def run_simulation(self):
        if not self.compiled:
            messagebox.showerror("Error", "Scheduler not compiled!")
            return
        
        if not self.processes:
            messagebox.showerror("Error", "Please add at least one process!")
            return
        
        algorithm = self.algorithm_var.get()
        quantum = self.quantum_entry.get()
        
        if algorithm == "RR":
            try:
                quantum_val = int(quantum)
                if quantum_val <= 0:
                    messagebox.showerror("Error", "Time quantum must be a positive integer!")
                    return
            except ValueError:
                messagebox.showerror("Error", "Time quantum must be a valid positive integer!")
                return
        
        try:
            input_data = ""
            for p in self.processes:
                input_data += f"{p['pid']} {p['arrival']} {p['burst']} {p['priority']}\n"
            
            cmd = ['./scheduler', algorithm, str(len(self.processes))]
            if algorithm == "RR":
                cmd.append(quantum)
            
            result = subprocess.run(cmd, input=input_data, capture_output=True, 
                                  text=True, timeout=5)
            
            if result.returncode != 0:
                messagebox.showerror("Error", f"Simulation failed:\n{result.stderr}")
                return
            
            data = json.loads(result.stdout)
            self.display_results(data, algorithm)
            
        except json.JSONDecodeError as e:
            messagebox.showerror("Error", f"Failed to parse results:\n{str(e)}\n{result.stdout}")
        except Exception as e:
            messagebox.showerror("Error", f"Simulation failed: {str(e)}")
    
    def display_results(self, data, algorithm):
        for widget in self.canvas_frame.winfo_children():
            widget.destroy()
        
        fig = Figure(figsize=(10, 4), dpi=100)
        ax = fig.add_subplot(111)
        
        gantt = data['gantt']
        colors = plt.cm.Set3(range(len(self.processes)))
        color_map = {}
        
        for i, p in enumerate(self.processes):
            color_map[p['pid']] = colors[i]
        
        for entry in gantt:
            pid = entry['pid']
            start = entry['start']
            end = entry['end']
            duration = end - start
            
            ax.barh(0, duration, left=start, height=0.5, 
                   color=color_map.get(pid, 'gray'),
                   edgecolor='black', linewidth=1.5)
            ax.text(start + duration/2, 0, f'P{pid}', 
                   ha='center', va='center', fontweight='bold', fontsize=10)
        
        ax.set_ylim(-0.5, 0.5)
        ax.set_xlabel('Time', fontsize=12, fontweight='bold')
        ax.set_ylabel('')
        ax.set_yticks([])
        ax.set_title(f'Gantt Chart - {algorithm}', fontsize=14, fontweight='bold', pad=20)
        ax.grid(axis='x', alpha=0.3)
        
        fig.tight_layout()
        
        canvas = FigureCanvasTkAgg(fig, master=self.canvas_frame)
        canvas.draw()
        canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
        avg_waiting = data['avg_waiting']
        avg_turnaround = data['avg_turnaround']
        avg_response = data['avg_response']
        
        metrics_text = f"Avg Waiting Time: {avg_waiting:.2f}  |  " \
                      f"Avg Turnaround Time: {avg_turnaround:.2f}  |  " \
                      f"Avg Response Time: {avg_response:.2f}"
        self.metrics_label.config(text=metrics_text)
        
        self.results_text.delete(1.0, tk.END)
        self.results_text.insert(tk.END, "Process Details:\n")
        self.results_text.insert(tk.END, "=" * 80 + "\n")
        self.results_text.insert(tk.END, f"{'PID':<6} {'Arrival':<10} {'Burst':<10} {'Priority':<10} "
                                        f"{'Completion':<12} {'Waiting':<10} {'Turnaround':<12} {'Response':<10}\n")
        self.results_text.insert(tk.END, "=" * 80 + "\n")
        
        for p in data['processes']:
            self.results_text.insert(tk.END, 
                f"P{p['pid']:<5} {p['arrival']:<10} {p['burst']:<10} {p['priority']:<10} "
                f"{p['completion']:<12} {p['waiting']:<10} {p['turnaround']:<12} {p['response']:<10}\n")

if __name__ == "__main__":
    root = tk.Tk()
    app = CPUSchedulerSimulator(root)
    root.mainloop()