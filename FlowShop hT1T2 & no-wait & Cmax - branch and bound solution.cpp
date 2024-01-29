#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <functional>
#include <chrono>

using namespace std;

// A structure to represent a node in the branch and bound tree
struct Node {
    int level; // the level of the node in the tree
    int cmax; // the makespan of the partial schedule
    int lb; // the lower bound of the node
    vector<int> schedule; // the partial schedule of the node
    vector<bool> visited; // the vector of visited jobs
};

// A function to calculate the lower bound of a node
int lower_bound(Node node, int n, const vector<int>& p1, const vector<int>& p2, const vector<int>& l, int h, int tau) {
    // Initialize the lower bound as the current makespan
    int lb = node.cmax;

    // Initialize the remaining processing times on both machines
    int r1 = 0, r2 = 0;

    // Loop through the unvisited jobs
    for (int i = 0; i < n; i++) {
        if (!node.visited[i]) {
            // Add the processing times of the unvisited jobs to the remaining processing times
            r1 += p1[i];
            r2 += p2[i];

            // Add the time delay of the unvisited job to the lower bound
            lb += l[i];
        }
    }

    // Add the maximum of the remaining processing times to the lower bound
    lb += max(r1, r2);

    // Add the minimum number of breaks required on both machines to the lower bound
    int b1 = (r1 + node.cmax) / tau;
    int b2 = (r2 + node.cmax) / tau;
    lb += h * max(b1, b2);

    // Return the lower bound
    return lb;
}

// A function to check if a node is a leaf node
bool is_leaf(Node node, int n) {
    // Loop through the visited vector
    for (int i = 0; i < n; i++) {
        // If any job is unvisited, return false
        if (!node.visited[i]) {
            return false;
        }
    }

    // If all jobs are visited, return true
    return true;
}

// A function to calculate the makespan of a node
int makespan(Node node, int n, const vector<int>& p1, const vector<int>& p2, const vector<int>& l, int h, int tau) {
    // Initialize the completion times on both machines
    int c1 = 0, c2 = 0;

    // Initialize the number of breaks on both machines
    int b1 = 0, b2 = 0;

    // Loop through the partial schedule
    for (int i = 0; i < node.level; i++) {
        // Get the current job
        int j = node.schedule[i];

        // Update the completion time on machine 1
        c1 += p1[j];

        // Check if a break is needed on machine 1
        if (c1 - b1 * h > tau) {
            // Update the completion time and the number of breaks on machine 1
            c1 += h;
            b1++;
        }

        // Update the completion time on machine 2
        c2 = max(c2 + p2[j], c1 + l[j]);

        // Check if a break is needed on machine 2
        if (c2 - b2 * h > tau) {
            // Update the completion time and the number of breaks on machine 2
            c2 += h;
            b2++;
        }
    }

    // Return the maximum of the completion times on both machines
    return max(c1, c2);
}

// A function to print the schedule of a node
int print_schedule(const Node& node, int n, const vector<int>& p1, const vector<int>& p2, const vector<int>& l, int h, int tau, bool includeSchedule = true) {
    // Initialize the start times and the end times on both machines
    vector<int> s1(n), e1(n), s2(n), e2(n);

    // Initialize the completion times on both machines
    int c1 = 0, c2 = 0;

    // Initialize the number of breaks on both machines
    int b1 = 0, b2 = 0;

    // Loop through the schedule
    for (int i = 0; i < n; i++) {
        // Get the current job
        int j = node.schedule[i];

        // Update the start time and the end time on machine 1
        s1[j] = c1;
        e1[j] = c1 + p1[j];

        // Update the completion time on machine 1
        c1 += p1[j];

        // Check if a break is needed on machine 1
        if (c1 - b1 * h > tau) {
            // Update the completion time and the number of breaks on machine 1
            c1 += h;
            b1++;

            // Update the start time of the next job on machine 1
            s1[j] = c1;
            e1[j] = c1 + p1[j];
        }

        // Update the start time and the end time on machine 2
        s2[j] = max(c2, c1 + l[j]);
        e2[j] = s2[j] + p2[j];

        // Update the completion time on machine 2
        c2 = e2[j];

        // Check if a break is needed on machine 2
        if (c2 - b2 * h > tau) {
            // Update the completion time and the number of breaks on machine 2
            c2 += h;
            b2++;
        }
    }

    // Print the schedule on machine 1
    if (includeSchedule) {
        cout << "Machine 1: ";
        for (int i = 0; i < n; i++) {
            int j = node.schedule[i];

            int start_time = s2[j] - p1[i];

            cout << "Task_" << j + 1 << " : " << start_time << ", ";
            if (i < n - 1 && s1[node.schedule[i + 1]] > e1[j]) {
                // Print the break
                cout << "Break : " << e1[j] << ", ";
            }
        }
    }

    // Print the schedule on machine 2
    cout << "\nMachine 2: ";
    int j;
    for (int i = 0; i < n; i++) {
        j = node.schedule[i];
        if (i == 0 || s2[j] > e2[node.schedule[i - 1]]) {
            // Print the break
            cout << "Break : " << s2[j] - h << ", ";
        }
        cout << "Task_" << j + 1 << " : " << s2[j] << ", ";
    }
    cout << "\n";

    return e2[j];
}

// A function to solve the Flow Shop scheduling problem using the branch and bound method
void solve(int n, const vector<int>& p1, const vector<int>& p2, const vector<int>& l, int h, int tau, bool includeSchedule = true) {
    // Create a priority queue to store the nodes
    priority_queue<Node, vector<Node>, function<bool(Node, Node)>> pq([](const Node& a, const Node& b) {
        // Compare the nodes by their lower bounds
        return a.lb > b.lb;
    });

    // Create a dummy node and push it to the queue
    Node dummy;
    dummy.level = 0;
    dummy.cmax = 0;
    dummy.lb = 0;
    dummy.schedule = vector<int>();
    dummy.visited = vector<bool>(n, false);
    pq.push(dummy);

    // Initialize the best node and the best makespan
    Node best;
    int best_cmax = numeric_limits<int>::max();

    // Loop until the queue is empty
    while (!pq.empty()) {
        // Get the current node from the queue
        Node curr = pq.top();
        pq.pop();

        // Check if the current node is a leaf node
        if (is_leaf(curr, n)) {
            // Check if the current node has a better makespan than the best node
            if (curr.cmax < best_cmax) {
                // Update the best node and the best makespan
                best = curr;
                best_cmax = curr.cmax;
            }
        }
        else {
            // Loop through the unvisited jobs
            for (int i = 0; i < n; i++) {
                if (!curr.visited[i]) {
                    // Create a child node
                    Node child;
                    child.level = curr.level + 1;
                    child.schedule = curr.schedule;
                    child.schedule.push_back(i);
                    child.visited = curr.visited;
                    child.visited[i] = true;
                    child.cmax = makespan(child, n, p1, p2, l, h, tau);
                    child.lb = lower_bound(child, n, p1, p2, l, h, tau);
        
                    // Check if the child node has a better lower bound than the best makespan
                    if (child.lb < best_cmax) {
                        // Push the child node to the queue
                        pq.push(child);
                    }
                }
            }
        }
    }

    // Print the best schedule and the best makespan
    cout << "\nThe best schedule is:\n";
    int Cmax = print_schedule(best, n, p1, p2, l, h, tau, includeSchedule);
    cout << "The best makespan (minimal Cmax) is: " << Cmax << "\n";
}

int main() {
    int n;
    cout << "Would you like to use given test set? y/n \n";
    char answer;
    cin >> answer;
    
    // USER PROMPT
    if (answer == 'n') {
    cout << "Enter the number of tasks: ";
    cin >> n;

    vector<int> p1(n), p2(n), l(n);

    for (int i = 0; i < n; ++i) {
        cout << "Enter the execution times for task " << i + 1 << " on machines 1 and 2: ";
        cin >> p1[i] >> p2[i];
    }

    cout << "Enter the length of periodic breaks (h): ";
    int h;
    cin >> h;

    cout << "Enter the maximum time between periodic breaks (tau): ";
    int tau;
    cin >> tau;
    
     // Check if tau is smaller than any entered execution times for tasks
    if (tau < *max_element(p1.begin(), p1.end()) || tau < *max_element(p2.begin(), p2.end())) {
        cerr << "Error: Maximum time between breaks (tau) should be greater than or equal to the maximum execution time for any task.\n";
        return 1;  // Return an error code
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();

    solve(n, p1, p2, l, h, tau);

    // Stop measuring time
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Print the time taken to execute the solve function
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;

    return 0;
    }
    
    // TESTING 
    else {
        srand(time(nullptr));  // Seed for random number generation
    
        int num_tests = 10;  // Number of tests to perform
        int total_duration = 0;  // Total duration for averaging
    
        for (int test = 1; test <= num_tests; ++test) {
            // Randomly generate n tasks with execution times in the range [1, 20]
            n = 8; // -- NUMBER OF TASKS
    
            vector<int> p1(n), p2(n), l(n);
    
            //  -- TASKS EXECUTION TIME
            for (int i = 0; i < n; ++i) {
                // Randomly choose execution times for each task in the range [1, 20]
                p1[i] = rand() % 10 + 5;
                p2[i] = rand() % 10 + 5;
            }
    
            //  -- BREAK TIME AND TAU (maximal time between 2 periodic breaks)
            int h = 5;
            int tau = 15;
    
            auto start_time = std::chrono::high_resolution_clock::now();
    
            // Call the solve function with randomly generated inputs
            solve(n, p1, p2, l, h, tau, false);
    
            // Stop measuring time
            auto end_time = std::chrono::high_resolution_clock::now();
    
            // Calculate the elapsed time
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
            // Print the time taken to execute the solve function for each test
            std::cout << "Test " << test << " - Time taken: " << duration.count() << " ms" << std::endl;
    
            // Accumulate the duration for averaging
            total_duration += duration.count();
        }
    
        // Calculate and print the average time across all tests
        std::cout << "\n\nAverage time across " << num_tests << " tests: " << total_duration / num_tests << " ms" << std::endl;
    }

}