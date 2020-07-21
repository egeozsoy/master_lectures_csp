//
// Created by Ege on 14.07.20.
//

#include "constraint.hpp"


void Domain::pop_state() {
    auto diff = states.back() - get_values().size();
    states.pop_back();
    auto ctr = diff;
    for (auto i = hidden.rbegin(); i != hidden.rend(); ++i) {
        if (ctr <= 0) {
            break;
        }
        values.push_back(*i);
        --ctr;
    }
    hidden.erase(hidden.end() - diff, hidden.end()); // TODO check this is correct
}