#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <queue>        
#include <vector>        
#include "../core/processService.hpp"  // Process definition

/* 
    QueueService - Wrapper for CFS runqueue
  
  The runqueue is a priority queue (min-heap) ordered by vruntime
  The process with the smallest vruntime is at the top
 
 * Why a min-heap?
  - CFS always selects the process with smallest vruntime
  - A min-heap gives O(log n) insertion and O(1) access to the minimum
  - This is efficient for the scheduling loop
  
 */


class QueueService {
private:

    // Compare - Custom comparator for the priority queue
    
    struct Compare {
        bool operator()(Process* a, Process* b) {
            // If vruntime equal, compare priority
            if (a->vruntime == b->vruntime) {
                return a->priority > b->priority;  // Lower number = higher priority
            }

            // Otherwise, larger vruntime = lower priority
            return a->vruntime > b->vruntime;
        }
    };
    
    // The actual priority queue
    // Template: <Type, Container, Comparator>
    std::priority_queue<Process*, std::vector<Process*>, Compare> q;
    
    // Statistics for monitoring
    size_t maxSize = 0;        // Maximum queue size reached
    size_t totalPushed = 0;    // Total number of pushes

public:
    QueueService() = default;
    
    void push(Process* p) {
        q.push(p);
        totalPushed++;
        maxSize = std::max(maxSize, q.size());
    }
    
    void pop() {
        q.pop();
    }

    bool empty() const {
        return q.empty();
    }
    
    size_t size() const {
        return q.size();
    }
    
    Process* top() {
        return q.top();
    }
    
    const Process* top() const {
        return q.top();
    }

    // Get Statistics

    size_t getMaxSize() const { return maxSize; }
    size_t getTotalPushed() const { return totalPushed; }
};

#endif