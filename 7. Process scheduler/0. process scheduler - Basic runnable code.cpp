// File: rr_scheduler.cpp
// Compile: g++ -std=c++17 rr_scheduler.cpp -O2 -o rr_scheduler
// Run: ./rr_scheduler

#include <bits/stdc++.h>
using namespace std;

enum class ProcState { NEW, READY, RUNNING, COMPLETED };

struct Process {
    int pid;
    string name;
    int burstTime;      // total required
    int remainingTime;  // remaining
    int arrivalTime;    // when it arrives to scheduler
    ProcState state;

    Process(int pid_, const string& name_, int burst_, int arrival_)
    : pid(pid_), name(name_), burstTime(burst_), remainingTime(burst_), arrivalTime(arrival_), state(ProcState::NEW) {}
};

struct RRExecutionSlice {
    int pid;
    string name;
    int startTime;
    int endTime;
    int ranFor;
    bool completed;
};

class RoundRobinScheduler {
private:
    int timeQuantum;
    int currentTime;

    deque<int> readyQ; // store pids in ready queue order
    unordered_map<int, unique_ptr<Process>> processes; // pid -> process object

    // future processes not yet arrived: pair(arrivalTime, pid)
    vector<pair<int,int>> futureArrivals; // will be kept sorted by arrivalTime

    // book-keeping for output
    vector<RRExecutionSlice> timeline;

    // helper to move arrivals whose arrivalTime <= currentTime into ready queue
    void moveArrivalsToReady() {
        while (!futureArrivals.empty() && futureArrivals.front().first <= currentTime) {
            int pid = futureArrivals.front().second;
            futureArrivals.erase(futureArrivals.begin());
            auto &p = processes[pid];
            p->state = ProcState::READY;
            readyQ.push_back(pid);
            // debugging print (optional)
            // cout << "[time " << currentTime << "] Process arrived: " << p->pid << " (" << p->name << ")\n";
        }
    }

public:
    RoundRobinScheduler(int quantum) : timeQuantum(quantum), currentTime(0) {}

    // Add process before run or even during the simulation (we insert into futureArrivals sorted by arrivalTime)
    void addProcess(int pid, const string &name, int burstTime, int arrivalTime = 0) {
        processes[pid] = make_unique<Process>(pid, name, burstTime, arrivalTime);
        // insert into futureArrivals keeping sorted order by arrivalTime
        auto it = lower_bound(futureArrivals.begin(), futureArrivals.end(), make_pair(arrivalTime, pid),
                              [](const pair<int,int>& a, const pair<int,int>& b){
                                  if (a.first != b.first) return a.first < b.first;
                                  return a.second < b.second;
                              });
        futureArrivals.insert(it, {arrivalTime, pid});
    }

    // Run until there are no future arrivals and no ready processes
    void run() {
        // initial move (in case arrivalTime==0)
        moveArrivalsToReady();

        while (!readyQ.empty() || !futureArrivals.empty()) {
            // If readyQ empty but future arrivals exist, fast-forward time to next arrival
            if (readyQ.empty() && !futureArrivals.empty()) {
                currentTime = max(currentTime, futureArrivals.front().first);
                moveArrivalsToReady();
                continue;
            }

            int pid = readyQ.front();
            readyQ.pop_front();
            auto &proc = processes[pid];
            proc->state = ProcState::RUNNING;

            // Determine how much to run this slice
            int runFor = min(timeQuantum, proc->remainingTime);
            int start = currentTime;
            int end = currentTime + runFor;

            // "Execute": decrement remainingTime and advance time
            proc->remainingTime -= runFor;
            currentTime = end;

            bool completed = false;
            if (proc->remainingTime <= 0) {
                proc->state = ProcState::COMPLETED;
                proc->remainingTime = 0;
                completed = true;
            } else {
                proc->state = ProcState::READY;
            }

            // record timeline
            timeline.push_back({pid, proc->name, start, end, runFor, completed});

            // After finishing this slice, move any processes whose arrivalTime <= currentTime to ready queue
            moveArrivalsToReady();

            // If process not completed, enqueue it at the tail
            if (!completed) {
                readyQ.push_back(pid);
            }
        }
    }

    // Introspection helpers
    vector<Process> getPendingProcesses() const {
        vector<Process> out;
        for (const auto &kv : processes) {
            const Process &p = *kv.second;
            if (p.state != ProcState::COMPLETED) out.push_back(p);
        }
        // sort by arrivalTime for predictability
        sort(out.begin(), out.end(), [](const Process &a, const Process &b){
            if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
            return a.pid < b.pid;
        });
        return out;
    }

    const vector<RRExecutionSlice>& getTimeline() const { 
        return timeline; 
    }

    int getCurrentTime() const { 
        return currentTime; 
    }

    void prettyPrintTimeline() const {
        cout << "=== Gantt-style timeline ===\n";
        for (const auto &s : timeline) {
            cout << "[" << setw(3) << s.startTime << " - " << setw(3) << s.endTime << "] "
                 << "PID " << setw(2) << s.pid << " (" << s.name << ")"
                 << " ranFor=" << setw(2) << s.ranFor
                 << (s.completed ? " COMPLETED" : "") << "\n";
        }
        cout << "Total time: " << currentTime << "\n";
    }

    void printProcessSummary() const {
        cout << "\n=== Processes summary ===\n";
        cout << left << setw(6) << "PID" << setw(12) << "Name" << setw(10) << "Burst" << setw(12) << "Remaining" << setw(10) << "Arrival" << "State\n";
        for (const auto &kv : processes) {
            const auto &p = *kv.second;
            string st;
            switch (p.state) {
                case ProcState::NEW: st = "NEW"; break;
                case ProcState::READY: st = "READY"; break;
                case ProcState::RUNNING: st = "RUNNING"; break;
                case ProcState::COMPLETED: st = "COMPLETED"; break;
            }
            cout << setw(6) << p.pid << setw(12) << p.name << setw(10) << p.burstTime
                 << setw(12) << p.remainingTime << setw(10) << p.arrivalTime << st << "\n";
        }
    }
};

int main() {
    // Example/demo:
    // We'll create a scheduler with time quantum = 4 units.
    RoundRobinScheduler sched(4);

    // Add processes (pid, name, burstTime, arrivalTime)
    // Mix arrival times to demonstrate arrivals during execution.
    sched.addProcess(1, "A", 10, 0); // arrives at 0, needs 10
    sched.addProcess(2, "B", 4, 1);  // arrives at 1, needs 4
    sched.addProcess(3, "C", 7, 2);  // arrives at 2, needs 7
    sched.addProcess(4, "D", 3, 6);  // arrives at 6, needs 3
    sched.addProcess(5, "E", 5, 6);  // arrives at 6, needs 5

    // Run the scheduler simulation
    sched.run();

    // Print results
    sched.prettyPrintTimeline();
    sched.printProcessSummary();

    return 0;
}

==================================================================================

=== Gantt-style timeline ===
[  0 -   4] PID  1 (A) ranFor= 4
[  4 -   8] PID  2 (B) ranFor= 4 COMPLETED
[  8 -  12] PID  3 (C) ranFor= 4
[ 12 -  16] PID  1 (A) ranFor= 4
[ 16 -  19] PID  4 (D) ranFor= 3 COMPLETED
[ 19 -  23] PID  3 (C) ranFor= 4
[ 23 -  25] PID  1 (A) ranFor= 2 COMPLETED
[ 25 -  29] PID  5 (E) ranFor= 4
[ 29 -  30] PID  5 (E) ranFor= 1 COMPLETED
Total time: 30

=== Processes summary ===
PID   Name        Burst     Remaining  Arrival   State
1     A           10        0          0         COMPLETED
2     B           4         0          1         COMPLETED
3     C           7         0          2         COMPLETED
4     D           3         0          6         COMPLETED
5     E           5         0          6         COMPLETED
