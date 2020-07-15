//
// Created by Ege on 14.07.20.
//

#ifndef MASTER_CSP_CPP_CONSTRAINT_HPP
#define MASTER_CSP_CPP_CONSTRAINT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "../helpers/lecture.hpp"


class Domain {
    std::vector<std::string> hidden;
    std::vector<std::string> states;
    // TODO maybe something like set?, debug python code for this

private:
    void reset_state();

    void push_state();

    void pop_state();

    void hide_value(std::string value);
};

class Constraint {
private:
    void call(std::string variables, std::string domains, std::string assignments);

    void pre_process(std::string variables, std::string domains, std::string constraints, std::string vconstraints);

    void forward_check(std::string variables, std::string domains, std::string assignments, std::string vconstraints); // TODO maybe unassigned
};

class FunctionConstraint : public Constraint {
    //TODO init with func and assigned
private:
    void call(std::string variables, std::string domains, std::string assignments);
};

class MaxSumConstraint : public Constraint {
    //TODO init with maxsum and multipliers
private:
    void pre_process(std::string variables, std::string domains, std::string constraints, std::string vconstraints);

    void call(std::string variables, std::string domains, std::string assignments);
};

class BacktrackingSolver {
private:
    std::string get_solution_iter(std::string domains, std::string constraints, std::string vconstraints);

    std::string get_solution(std::string domains, std::string constraints, std::string vconstraints);

    std::string get_solutions(std::string domains, std::string constraints, std::string vconstraints);
};

template<typename T>
class Problem {
    BacktrackingSolver solver = BacktrackingSolver();
    std::vector<std::string> constraints;
    std::map<T, int> variables;
    // TODO create object Variable use it instead of Lecture. Lecture should inherit from Variable

public:
    void add_variable(std::string variable, std::string domain);

    void add_variables(std::vector<T> variables, std::vector<int> domain);

private:
    void reset();

    BacktrackingSolver get_solver();


    void add_constraint(std::string constraint, std::string variables);

    std::string get_solution();

    std::string get_solutions();

    std::string get_solution_iter();

    std::string get_args();
};

template<typename T>
void Problem<T>::add_variables(std::vector<T> variables, std::vector<int> domain) {

}

#endif //MASTER_CSP_CPP_CONSTRAINT_HPP
