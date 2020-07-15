//
// Created by Ege on 14.07.20.
//

#ifndef MASTER_CSP_CPP_CONSTRAINT_HPP
#define MASTER_CSP_CPP_CONSTRAINT_HPP

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include "../helpers/lecture.hpp"


class Domain {
    std::vector<std::string> hidden;
    std::vector<std::string> states;
    std::vector<int> values;
    // TODO maybe something like set?, debug python code for this
public:
    Domain() = default;

    explicit Domain(std::vector<int> vals) : values(std::move(vals)) {};
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
struct CustomHasher {
    size_t operator()(T t) const { return std::hash<std::string>()(t.name); }
};

template<typename T>
class Problem {
    BacktrackingSolver solver = BacktrackingSolver();
    std::vector<std::string> constraints;
    std::unordered_map<T, Domain, CustomHasher<T>> variables;

public:
    void add_variable(std::string variable, std::string domain);

    void add_variables(std::vector<T> vars, const std::vector<int> &domain);

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
void Problem<T>::add_variables(std::vector<T> vars, const std::vector<int> &domain_ints) {
    Domain domain(domain_ints);
    for (const auto &var : vars) {
        variables[var] = domain;
    }
    auto a = 1;
}

#endif //MASTER_CSP_CPP_CONSTRAINT_HPP
