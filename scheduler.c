#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    int first_run;
} Process;

typedef struct {
    int pid;
    int start_time;
    int end_time;
} GanttEntry;

void print_gantt_json(GanttEntry *gantt, int gantt_size) {
    printf("\"gantt\": [");
    for (int i = 0; i < gantt_size; i++) {
        printf("{\"pid\": %d, \"start\": %d, \"end\": %d}",
               gantt[i].pid, gantt[i].start_time, gantt[i].end_time);
        if (i < gantt_size - 1) printf(", ");
    }
    printf("], ");
}

void print_results_json(Process *processes, int n, GanttEntry *gantt, int gantt_size) {
    float avg_waiting = 0, avg_turnaround = 0, avg_response = 0;
    
    printf("{");
    print_gantt_json(gantt, gantt_size);
    
    printf("\"processes\": [");
    for (int i = 0; i < n; i++) {
        printf("{\"pid\": %d, \"arrival\": %d, \"burst\": %d, \"priority\": %d, "
               "\"completion\": %d, \"waiting\": %d, \"turnaround\": %d, \"response\": %d}",
               processes[i].pid, processes[i].arrival_time, processes[i].burst_time,
               processes[i].priority, processes[i].completion_time, processes[i].waiting_time,
               processes[i].turnaround_time, processes[i].response_time);
        if (i < n - 1) printf(", ");
        
        avg_waiting += processes[i].waiting_time;
        avg_turnaround += processes[i].turnaround_time;
        avg_response += processes[i].response_time;
    }
    printf("], ");
    
    avg_waiting /= n;
    avg_turnaround /= n;
    avg_response /= n;
    
    printf("\"avg_waiting\": %.2f, \"avg_turnaround\": %.2f, \"avg_response\": %.2f}\n",
           avg_waiting, avg_turnaround, avg_response);
}

void fcfs(Process *processes, int n) {
    GanttEntry gantt[1000];
    int gantt_size = 0;
    int current_time = 0;
    
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }
        
        processes[i].response_time = current_time - processes[i].arrival_time;
        gantt[gantt_size].pid = processes[i].pid;
        gantt[gantt_size].start_time = current_time;
        current_time += processes[i].burst_time;
        gantt[gantt_size].end_time = current_time;
        gantt_size++;
        
        processes[i].completion_time = current_time;
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
    }
    
    print_results_json(processes, n, gantt, gantt_size);
}

void sjf_non_preemptive(Process *processes, int n) {
    GanttEntry gantt[1000];
    int gantt_size = 0;
    int completed = 0, current_time = 0;
    int is_completed[100] = {0};
    
    while (completed < n) {
        int idx = -1;
        int min_burst = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && !is_completed[i]) {
                if (processes[i].burst_time < min_burst) {
                    min_burst = processes[i].burst_time;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            current_time++;
            continue;
        }
        
        processes[idx].response_time = current_time - processes[idx].arrival_time;
        gantt[gantt_size].pid = processes[idx].pid;
        gantt[gantt_size].start_time = current_time;
        current_time += processes[idx].burst_time;
        gantt[gantt_size].end_time = current_time;
        gantt_size++;
        
        processes[idx].completion_time = current_time;
        processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
        processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
        is_completed[idx] = 1;
        completed++;
    }
    
    print_results_json(processes, n, gantt, gantt_size);
}

void sjf_preemptive(Process *processes, int n) {
    GanttEntry gantt[1000];
    int gantt_size = 0;
    int completed = 0, current_time = 0;
    int prev_pid = -1;
    
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_run = -1;
    }
    
    while (completed < n) {
        int idx = -1;
        int min_remaining = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0) {
                if (processes[i].remaining_time < min_remaining) {
                    min_remaining = processes[i].remaining_time;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            current_time++;
            continue;
        }
        
        if (processes[idx].first_run == -1) {
            processes[idx].first_run = current_time;
            processes[idx].response_time = current_time - processes[idx].arrival_time;
        }
        
        if (prev_pid != processes[idx].pid) {
            if (gantt_size > 0 && prev_pid != -1) {
                gantt[gantt_size - 1].end_time = current_time;
            }
            gantt[gantt_size].pid = processes[idx].pid;
            gantt[gantt_size].start_time = current_time;
            gantt_size++;
            prev_pid = processes[idx].pid;
        }
        
        processes[idx].remaining_time--;
        current_time++;
        
        if (processes[idx].remaining_time == 0) {
            processes[idx].completion_time = current_time;
            processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed++;
        }
    }
    
    if (gantt_size > 0) {
        gantt[gantt_size - 1].end_time = current_time;
    }
    
    print_results_json(processes, n, gantt, gantt_size);
}

void priority_non_preemptive(Process *processes, int n) {
    GanttEntry gantt[1000];
    int gantt_size = 0;
    int completed = 0, current_time = 0;
    int is_completed[100] = {0};
    
    while (completed < n) {
        int idx = -1;
        int highest_priority = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && !is_completed[i]) {
                if (processes[i].priority < highest_priority) {
                    highest_priority = processes[i].priority;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            current_time++;
            continue;
        }
        
        processes[idx].response_time = current_time - processes[idx].arrival_time;
        gantt[gantt_size].pid = processes[idx].pid;
        gantt[gantt_size].start_time = current_time;
        current_time += processes[idx].burst_time;
        gantt[gantt_size].end_time = current_time;
        gantt_size++;
        
        processes[idx].completion_time = current_time;
        processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
        processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
        is_completed[idx] = 1;
        completed++;
    }
    
    print_results_json(processes, n, gantt, gantt_size);
}

void priority_preemptive(Process *processes, int n) {
    GanttEntry gantt[1000];
    int gantt_size = 0;
    int completed = 0, current_time = 0;
    int prev_pid = -1;
    
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_run = -1;
    }
    
    while (completed < n) {
        int idx = -1;
        int highest_priority = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0) {
                if (processes[i].priority < highest_priority) {
                    highest_priority = processes[i].priority;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            current_time++;
            continue;
        }
        
        if (processes[idx].first_run == -1) {
            processes[idx].first_run = current_time;
            processes[idx].response_time = current_time - processes[idx].arrival_time;
        }
        
        if (prev_pid != processes[idx].pid) {
            if (gantt_size > 0 && prev_pid != -1) {
                gantt[gantt_size - 1].end_time = current_time;
            }
            gantt[gantt_size].pid = processes[idx].pid;
            gantt[gantt_size].start_time = current_time;
            gantt_size++;
            prev_pid = processes[idx].pid;
        }
        
        processes[idx].remaining_time--;
        current_time++;
        
        if (processes[idx].remaining_time == 0) {
            processes[idx].completion_time = current_time;
            processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed++;
        }
    }
    
    if (gantt_size > 0) {
        gantt[gantt_size - 1].end_time = current_time;
    }
    
    print_results_json(processes, n, gantt, gantt_size);
}

void round_robin(Process *processes, int n, int quantum) {
    if (quantum <= 0) {
        fprintf(stderr, "Error: Time quantum must be positive\n");
        exit(1);
    }
    
    GanttEntry gantt[1000];
    int gantt_size = 0;
    int current_time = 0, completed = 0;
    int queue[100], front = 0, rear = 0;
    int in_queue[100] = {0};
    
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_run = -1;
    }
    
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival_time == 0) {
            queue[rear++] = i;
            in_queue[i] = 1;
        }
    }
    
    while (completed < n) {
        if (front == rear) {
            current_time++;
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival_time <= current_time && !in_queue[i] && processes[i].remaining_time > 0) {
                    queue[rear++] = i;
                    in_queue[i] = 1;
                }
            }
            continue;
        }
        
        int idx = queue[front++];
        in_queue[idx] = 0;
        
        if (processes[idx].first_run == -1) {
            processes[idx].first_run = current_time;
            processes[idx].response_time = current_time - processes[idx].arrival_time;
        }
        
        gantt[gantt_size].pid = processes[idx].pid;
        gantt[gantt_size].start_time = current_time;
        
        int time_slice = (processes[idx].remaining_time < quantum) ? processes[idx].remaining_time : quantum;
        processes[idx].remaining_time -= time_slice;
        current_time += time_slice;
        
        gantt[gantt_size].end_time = current_time;
        gantt_size++;
        
        for (int i = 0; i < n; i++) {
            if (i != idx && processes[i].arrival_time <= current_time && !in_queue[i] && processes[i].remaining_time > 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
        
        if (processes[idx].remaining_time > 0) {
            queue[rear++] = idx;
            in_queue[idx] = 1;
        } else {
            processes[idx].completion_time = current_time;
            processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed++;
        }
    }
    
    print_results_json(processes, n, gantt, gantt_size);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <algorithm> <num_processes> [quantum]\n", argv[0]);
        return 1;
    }
    
    char *algorithm = argv[1];
    int n = atoi(argv[2]);
    int quantum = (argc > 3) ? atoi(argv[3]) : 2;
    
    Process processes[100];
    
    for (int i = 0; i < n; i++) {
        scanf("%d %d %d %d", &processes[i].pid, &processes[i].arrival_time,
              &processes[i].burst_time, &processes[i].priority);
    }
    
    if (strcmp(algorithm, "FCFS") == 0) {
        fcfs(processes, n);
    } else if (strcmp(algorithm, "SJF_NP") == 0) {
        sjf_non_preemptive(processes, n);
    } else if (strcmp(algorithm, "SJF_P") == 0) {
        sjf_preemptive(processes, n);
    } else if (strcmp(algorithm, "PRIORITY_NP") == 0) {
        priority_non_preemptive(processes, n);
    } else if (strcmp(algorithm, "PRIORITY_P") == 0) {
        priority_preemptive(processes, n);
    } else if (strcmp(algorithm, "RR") == 0) {
        round_robin(processes, n, quantum);
    } else {
        fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        return 1;
    }
    
    return 0;
}
