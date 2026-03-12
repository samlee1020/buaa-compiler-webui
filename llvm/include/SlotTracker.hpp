#ifndef SLOTTRACKER_HPP
#define SLOTTRACKER_HPP

#include "IrForward.hpp"
#include <unordered_map>

/*
    SlotTracker负责给每一个函数作用域内的value从0开始编号
*/
class SlotTracker {
    public:
        void Trace(FunctionPtr function);
        int Slot(ValuePtr value);
    
    private: 
        std::unordered_map<ValuePtr, int> _slot;
};

using SlotTrackerPtr = SlotTracker *;

#endif