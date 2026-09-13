#include "events/event_sequence.hpp"

bool EventSequenceValidator::accept(SequenceNumber sequence){
    if(sequence != next_expected_){ return false;   }
    ++next_expected_;
    return true;
}

SequenceNumber EventSequenceValidator::next_expected() const {
    return next_expected_;
}

void EventSequenceValidator::reset(){
    next_expected_=1;
}