# Cpu_Scheduling_Simulator
Cpu Scheduling Simulator implementing the processes FCFS, SJF (both preemptive and non preemptive), Priority (both preemptive and non preemptive) and round robin using C as backend and python as the frontend for visualisation.
This simulator allows users to input processes with attributes such as:
Process ID (PID), Arrival Time, Burst Time and Priority Value (for priority scheduling)

Backend (C): Core Logic 

The backend is built entirely in C, ensuring:
Fast execution of scheduling logic, Deterministic and OS-independent behaviour and Efficient handling of multiple processes and time slices.

The C module performs:
Timeline generation (Gantt chart structure), Preemption handling, Queue and priority management and Computation of all performance metrics.

Frontend (Python): Interface & Visualization

The frontend is implemented in Python, selected for:
Clean UI possibilities, Better plotting/visualization libraries and Simpler interaction for users

The Python program:

Accepts process input from users, Sends requests to the C backend, Receives scheduling results, Visualizes Gantt charts, Displays computed metrics in tables and Allows comparison between algorithms.

