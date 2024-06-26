#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <sstream>
#include <memory>  
#include <queue>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <numeric> 
#include <algorithm> 
#include <list>
#include <iterator>
#include <cmath>
#include <climits> 
#include <unistd.h>


int currentTime = 0;
int currentHeadPosition = 0; 
int totalTrack = 0;
int totalBusyTime = 0;
int direction = 1;
int total_movement = 0;

// Define an IO request struct
struct IORequest {
    int id; // Unique identifier for the request
    int arrivalTime;
    int track;
    int startTime; // When the request actually starts processing
    int endTime;   // When the request completesx
};

// Abstract class for IO Scheduling Algorithms
class IOScheduler {
public:
    // Virtual destructor for safe polymorphic deletion
    virtual ~IOScheduler() {}

    // Add a request to the scheduler
    virtual void addRequest(IORequest req) = 0;

    // Get the next request, potentially activating it
    virtual IORequest getNextRequest() = 0;

    // Check if the scheduler has pending requests
    virtual bool hasRequests() const = 0;

    // Check if there is currently an active I/O operation
    virtual bool isActive() const = 0;

    virtual int getActiveTrack() const  = 0;

    virtual bool isComplete(int currentTime) const = 0;

    // Marks the current active request as completed
    virtual void completeCurrentRequest() = 0;

    // Method to simulate the movement of the disk head
    // virtual void moveHead() = 0;

    virtual void startNextRequest() = 0;

    virtual const std::vector<IORequest>& getCompletedRequests() const  = 0;
};

// FIFO scheduling algorithm implementation
class FIFO : public IOScheduler {
private:
    std::queue<IORequest> requestQueue;     // Queue to hold incoming requests
    bool currentActive = false;             // Flag to check if a request is currently being processed
    IORequest activeRequest;                // The request currently being processed
    std::vector<IORequest> completedRequests;   

public:
    void addRequest(IORequest req) override {
        requestQueue.push(req);
    }

    IORequest getNextRequest() override {
        if (!requestQueue.empty() && !currentActive) {  // Only fetch next request if no active request
            activeRequest = requestQueue.front();
            requestQueue.pop();
            activeRequest.startTime = currentTime;
            currentActive = true;
            return activeRequest;
        }
        return IORequest{}; // Return an empty request if one is active
    }

    bool hasRequests() const override {
        return !requestQueue.empty() || currentActive;
    }

    bool isActive() const override {
        return currentActive;
    }

    int getActiveTrack() const {
        if (currentActive) {
            return activeRequest.track;
        }
        return -1; // Return an invalid track number if no active request
    }

    bool isComplete(int currentTime) const override {
        return currentActive && (currentHeadPosition == activeRequest.track); 
    }

    void completeCurrentRequest() override {
        if (currentActive) {
            activeRequest.endTime = currentTime;
            completedRequests.push_back(activeRequest);
            currentActive = false;
        }
    }

    void startNextRequest() override {
        if (!currentActive && !requestQueue.empty()) {
            getNextRequest();  // Start the next request only if there is no active request
        }
    }

    const std::vector<IORequest>& getCompletedRequests() const {
        return completedRequests;
    }
};

// SSTF scheduling algorithm implementation
class SSTF : public IOScheduler {
private:
    std::list<IORequest> requestList; // List to hold all pending requests
    bool currentActive = false;       // Flag to check if there is an active request
    IORequest activeRequest;          // Stores the current active request
    std::vector<IORequest> completedRequests; // List of completed requests for statistics

public:
    // Adding a new IO request to the pool
    void addRequest(IORequest req) override {
        requestList.push_back(req);
    }

    // Fetching the next request based on the shortest seek time
    IORequest getNextRequest() override {
        if (!requestList.empty() && !currentActive) {
            auto it = std::min_element(requestList.begin(), requestList.end(),
                [this](const IORequest& a, const IORequest& b) {
                    return std::abs(a.track - currentHeadPosition) < std::abs(b.track - currentHeadPosition);
                });

            activeRequest = *it;
            requestList.erase(it);  // Remove the request from the pool
            activeRequest.startTime = currentTime;  // Set the start time for the request
            currentActive = true;
            return activeRequest;
        }
        return IORequest{};  // Return an empty request if there's already an active request
    }

    // Check if there are pending requests or an active request
    bool hasRequests() const override {
        return !requestList.empty() || currentActive;
    }

    // Check if there's an active request
    bool isActive() const override {
        return currentActive;
    }

    // Retrieve the track of the active request
    int getActiveTrack() const override {
        if (currentActive) {
            return activeRequest.track;
        }
        return -1;
    }

    // Determine if the current request has completed
    bool isComplete(int currentTime) const override {
        return currentActive && (currentHeadPosition == activeRequest.track);
    }

    // Mark the current request as completed
    void completeCurrentRequest() override {
        if (currentActive) {
            activeRequest.endTime = currentTime;
            completedRequests.push_back(activeRequest);
            currentActive = false;
        }
    }

    // Attempt to start the next request if no current active request
    void startNextRequest() override {
        if (!currentActive && !requestList.empty()) {
            getNextRequest();
        }
    }

    // Access completed requests for statistics
    const std::vector<IORequest>& getCompletedRequests() const override {
        return completedRequests;
    }
};

// LOOK scheduling algorithm implementation
class LOOK : public IOScheduler {
private:
    std::list<IORequest> requests; // A list to hold all pending requests
    bool currentActive = false;
    IORequest activeRequest;
    std::vector<IORequest> completedRequests;
    int direction = 1; // 1 for increasing track numbers, -1 for decreasing

public:
    // Add a new IO request to the pool
    void addRequest(IORequest req) override {
        requests.push_back(req);
        requests.sort([](const IORequest& a, const IORequest& b) { return a.track < b.track; }); // Keep list sorted by track number for LOOK efficiency
    }

    // Fetching the next request based on LOOK strategy
    IORequest getNextRequest() override {
        if (!requests.empty() && !currentActive) {
            auto it = findNextRequest();
            if (it != requests.end()) {
                activeRequest = *it;
                requests.erase(it);
                activeRequest.startTime = currentTime;
                currentActive = true;
                return activeRequest;
            }
        }
        return IORequest{}; // Return an empty request if no appropriate one is found
    }

    // Helper function to find the next request in the current direction
    std::list<IORequest>::iterator findNextRequest() {
        std::list<IORequest>::iterator selected = requests.end();
        int distance = INT_MAX;

        for (auto it = requests.begin(); it != requests.end(); ++it) {
            int dist = std::abs(it->track - currentHeadPosition);
            if ((direction == 1 && it->track >= currentHeadPosition) || 
            (direction == -1 && it->track <= currentHeadPosition)) {
                if (dist < distance) {
                    distance = dist;
                    selected = it;
                }
            }
        }

        // If no request found in the current direction, change direction
        if (selected == requests.end()) {
            direction = -direction; // Reverse the direction
            return findNextRequest(); // Recursive call to find in the new direction
        }

        return selected;
    }

    // Check if there are pending requests or an active request
    bool hasRequests() const override {
        return !requests.empty() || currentActive;
    }

    // Check if there's an active request
    bool isActive() const override {
        return currentActive;
    }

    // Retrieve the track of the active request
    int getActiveTrack() const override {
        if (currentActive) {
            return activeRequest.track;
        }
        return -1;
    }

    // Determine if the current request has completed
    bool isComplete(int currentTime) const override {
        return currentActive && (currentHeadPosition == activeRequest.track);
    }

    // Mark the current request as completed
    void completeCurrentRequest() override {
        if (currentActive) {
            activeRequest.endTime = currentTime;
            completedRequests.push_back(activeRequest);
            currentActive = false;
        }
    }

    // Attempt to start the next request if no current active request
    void startNextRequest() override {
        if (!currentActive && !requests.empty()) {
            getNextRequest();
        }
    }

    // Access completed requests for statistics
    const std::vector<IORequest>& getCompletedRequests() const override {
        return completedRequests;
    }
};

// CLOOK scheduling algorithm implementation
class CLOOK : public IOScheduler {
private:
    std::list<IORequest> requestList; // List to hold all pending requests
    bool currentActive = false;       // Flag to check if there is an active request
    IORequest activeRequest;          // Stores the current active request
    std::vector<IORequest> completedRequests; // List of completed requests for statistics

public:
    // Adding a new IO request to the pool
    void addRequest(IORequest req) override {
        requestList.push_back(req);
        requestList.sort([](const IORequest& a, const IORequest& b) { return a.track < b.track; });
    }

    // Fetching the next request based on C-LOOK strategy
    IORequest getNextRequest() override {
        if (!requestList.empty() && !currentActive) {
            // If we are at the end or the next request would be in the opposite direction
            auto it = std::find_if(requestList.begin(), requestList.end(),
                                   [this](const IORequest& req) { return req.track >= currentHeadPosition; });
            
            if (it == requestList.end()) {  // If no suitable request is found, wrap around
                it = requestList.begin();
            }

            activeRequest = *it;
            requestList.erase(it);  // Remove the request from the pool
            activeRequest.startTime = currentTime;  // Set the start time for the request
            currentActive = true;
            return activeRequest;
        }
        return IORequest{};  // Return an empty request if there's already an active request
    }

    bool hasRequests() const override {
        return !requestList.empty() || currentActive;
    }

    bool isActive() const override {
        return currentActive;
    }

    int getActiveTrack() const override {
        if (currentActive) {
            return activeRequest.track;
        }
        return -1;
    }

    bool isComplete(int currentTime) const override {
        return currentActive && (currentHeadPosition == activeRequest.track);
    }

    void completeCurrentRequest() override {
        if (currentActive) {
            activeRequest.endTime = currentTime;
            completedRequests.push_back(activeRequest);
            currentActive = false;
        }
    }

    void startNextRequest() override {
        if (!currentActive && !requestList.empty()) {
            getNextRequest();
        }
    }

    const std::vector<IORequest>& getCompletedRequests() const override {
        return completedRequests;
    }
};

// Comparator class to manage comparison of IORequest based on track proximity
class TrackComparator {
public:
    int headPosition;
    int direction;

    TrackComparator(int headPos, int dir) : headPosition(headPos), direction(dir) {}

    bool operator()(const IORequest& lhs, const IORequest& rhs) {
        // Compare based on proximity to the current head position
        int distL = std::abs(lhs.track - headPosition);
        int distR = std::abs(rhs.track - headPosition);

        // If both requests are in the same direction as the head is moving
        if ((direction == 1 && lhs.track >= headPosition && rhs.track >= headPosition) ||
            (direction == -1 && lhs.track <= headPosition && rhs.track <= headPosition)) {
            return distL < distR; // The one closer to the head comes first
        }
        return lhs.track < rhs.track; // If equal, sort by track number
    }
};

// FLOOK scheduling algorithm implementation
class FLOOK : public IOScheduler {
private:
    std::list<IORequest> activeQueue; // Queue currently being serviced
    std::list<IORequest> incomingQueue; // Queue for incoming requests
    IORequest activeRequest;
    bool currentActive = false;
    std::vector<IORequest> completedRequests;
    int direction = 1; // 1 for increasing track numbers, -1 for decreasing

public:
    void addRequest(IORequest req) override {
        incomingQueue.push_back(req);
    }

    IORequest getNextRequest() override {
        if (!currentActive) {
            if (activeQueue.empty()) {
                std::swap(activeQueue, incomingQueue);
                activeQueue.sort(TrackComparator(currentHeadPosition, direction));
            }

            auto it = findNextRequest();
            if (it != activeQueue.end()) {
                activeRequest = *it;
                activeQueue.erase(it);
                activeRequest.startTime = currentTime;
                currentActive = true;
                return activeRequest;
            }
        }
        return IORequest{}; // Return an empty request if no appropriate one is found
    }

    std::list<IORequest>::iterator findNextRequest() {
        auto selected = activeQueue.end();
        int distance = INT_MAX;

        for (auto it = activeQueue.begin(); it != activeQueue.end(); ++it) {
            int dist = std::abs(it->track - currentHeadPosition);
            if ((direction == 1 && it->track >= currentHeadPosition) || (direction == -1 && it->track <= currentHeadPosition)) {
                if (dist < distance) {
                    distance = dist;
                    selected = it;
                }
            }
        }

        // If no request found in the current direction, change direction
        if (selected == activeQueue.end()) {
            direction *= -1; // Reverse the direction
            return findNextRequest(); // Recursive call to find in the new direction
        }

        return selected;
    }

    bool hasRequests() const override {
        return !activeQueue.empty() || !incomingQueue.empty() || currentActive;
    }

    bool isActive() const override {
        return currentActive;
    }

    int getActiveTrack() const override {
        if (currentActive) {
            return activeRequest.track;
        }
        return -1;
    }

    bool isComplete(int currentTime) const override {
        return currentActive && (currentHeadPosition == activeRequest.track);
    }

    void completeCurrentRequest() override {
        if (currentActive) {
            activeRequest.endTime = currentTime;
            completedRequests.push_back(activeRequest);
            currentActive = false;
        }
    }

    void startNextRequest() override {
        if (!currentActive && (!activeQueue.empty() || !incomingQueue.empty())) {
            getNextRequest();
        }
    }

    const std::vector<IORequest>& getCompletedRequests() const override {
        return completedRequests;
    }
};

// Function to read IO requests from file
std::vector<IORequest> readRequests(const std::string& filename) {
    std::vector<IORequest> requests;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return requests;
    }

    std::string line;
    int requestId = 0; // Start an ID counter for each request
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        std::istringstream iss(line);
        IORequest req;
        if (iss >> req.arrivalTime >> req.track) {
            req.id = requestId++; // Assign an incrementing ID to each request
            // Initialize startTime and endTime with default values if needed
            req.startTime = 0; // This will be set when the request is processed
            req.endTime = 0;   // This will be set when the request completes
            requests.push_back(req);
        }
    }
    return requests;
}

// Function to print IO requests
void printRequests(const std::vector<IORequest>& requests) {
    std::cout << "Total IO Requests: " << requests.size() << std::endl;
    for (const auto& req : requests) {
        std::cout << "Arrival Time: " << req.arrivalTime << ", Track: " << req.track << std::endl;
    }
}
void printStatistics(const std::vector<IORequest>& completedRequests) {
    // First, sort the requests by ID
    std::vector<IORequest> sortedRequests = completedRequests; // Make a copy to sort
    std::sort(sortedRequests.begin(), sortedRequests.end(), [](const IORequest& a, const IORequest& b) {
        return a.id < b.id; // Sorting criterion
    });

    // Now print each request in order of their ID
    for (const auto& req : sortedRequests) {
        std::cout << std::right << std::setw(5) << req.id << ":"
                  << std::setw(6) << req.arrivalTime
                  << std::setw(6) << req.startTime
                  << std::setw(6) << req.endTime << std::endl;
    }

    if (!completedRequests.empty()) {
        int total_time = completedRequests.back().endTime;
        int total_movement = totalTrack;
        // for (size_t i = 1; i < completedRequests.size(); ++i) {
        //     total_movement += std::abs(completedRequests[i].track - completedRequests[i - 1].track);
        // }
        double io_utilization = (double)total_movement / (currentTime - 1);
        double total_turnaround_time = 0;
        for (const auto& req : completedRequests) {
            total_turnaround_time += req.endTime - req.arrivalTime;
        }
        double avg_turnaround = total_turnaround_time / completedRequests.size();

        // Calculate average wait time
        double total_wait_time = 0;
        for (const auto& req : completedRequests) {
            total_wait_time += req.startTime - req.arrivalTime;
        }
        double avg_waittime = total_wait_time / completedRequests.size();

        int max_waittime = 0;
        for (const auto& req : completedRequests) {
            int waitTime = req.startTime - req.arrivalTime;
            if (waitTime > max_waittime) {
                max_waittime = waitTime;
            }
        }

        std::cout << "SUM: " << total_time << " " << total_movement << " " << std::fixed << std::setprecision(4) << io_utilization << " "
                  << std::fixed << std::setprecision(2) << avg_turnaround << " " << avg_waittime << " " << max_waittime <<"\n";
    }
}

// Main function that sets up and runs the simulation
int main(int argc, char* argv[]) {
    std::string inputFileName;
    char schedulerType = 'N';  // Default scheduler type set to 'N' for FIFO
    int opt;

    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
            case 's':
                schedulerType = optarg[0];
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -s <schedalgo> <inputfile>\n";
                return EXIT_FAILURE;
        }
    }

    // The input file should be the next argument after the options
    if (optind < argc) {
        inputFileName = argv[optind];
    } else {
        std::cerr << "Expected input file\n";
        return EXIT_FAILURE;
    }

    auto requests = readRequests(inputFileName);
    std::unique_ptr<IOScheduler> scheduler;

    // Instantiate the appropriate scheduler
    switch (schedulerType) {
        case 'N':
            scheduler.reset(new FIFO());
            break;
        case 'S':
            scheduler.reset(new SSTF());
            break;
        case 'L':
            scheduler.reset(new LOOK());
            break;
        case 'C':
            scheduler.reset(new CLOOK());
            break;
        case 'F':
            scheduler.reset(new FLOOK());
            break;
        default:
            std::cerr << "Invalid scheduler type specified: " << schedulerType << "\n";
            return EXIT_FAILURE;
    }
    size_t requestIndex = 0;
    bool allRequestsProcessed = false;

    // Print requests to check input parsing
    // printRequests(requests);
    
    while (!allRequestsProcessed) {
        int isMoveHead = 0;
        int edgeFlag = 1;
        
        // Process any new arrivals at the current time
        if (requests[requestIndex].arrivalTime == currentTime) {
            scheduler->addRequest(requests[requestIndex]);
            requestIndex++;
        }

        // Check if the current IO operation is complete
        if (scheduler->isActive()) { //
            if (scheduler->isComplete(currentTime)) {
                scheduler->completeCurrentRequest();
            } 
        }

        // If no IO request is active and there are pending requests, start a new IO operation
        if (!scheduler->isActive() && scheduler->hasRequests()) {
            scheduler->startNextRequest();
            isMoveHead = 1;
        }

        // Check if all requests are processed
        if (!scheduler->hasRequests() && requestIndex >= requests.size()) {
            allRequestsProcessed = true;
        }

        if (scheduler->isActive()){
            int activeTrack = scheduler->getActiveTrack();
            if(currentHeadPosition != activeTrack){
                if (currentHeadPosition < activeTrack) direction = 1;
                else if (currentHeadPosition > activeTrack) direction = -1;
                isMoveHead = 1;
            }
            else{
                direction = 0;
                edgeFlag = 0;
            } 
        }
        if (edgeFlag){
            currentTime++;  // Increment the simulation time
        }
        else totalTrack--;

        if (isMoveHead){
            totalTrack++;
            currentHeadPosition += direction*1;
            // total_movement++;
        } 
    }

    // Print final statistics here
    printStatistics(scheduler->getCompletedRequests());

    return 0;
}
