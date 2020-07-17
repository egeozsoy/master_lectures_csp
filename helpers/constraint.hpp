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
    std::vector<int> get_values() { return values; }

    explicit Domain(std::vector<int> vals) : values(std::move(vals)) {};
private:
    void reset_state();

    void push_state();

    void pop_state();

    void hide_value(std::string value);
};

template<typename T>
struct CustomHasher {
    size_t operator()(T t) const { return std::hash<std::string>()(t.name); }
};

template<typename T>
class Constraint {
public:
    Constraint() = default;

    virtual ~Constraint() = default;

    virtual int get_max_sum();

    virtual std::unique_ptr<Constraint<T>> clone();

    virtual void pre_process(
            std::vector<T> vars,
            std::unordered_map<T, Domain, CustomHasher<T>> domains,
            std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> vconstraints
    );

    virtual void call(std::vector<T> vars,
                      std::unordered_map<T, Domain, CustomHasher<T>> domains,
                      std::pair<T, int> assignments);

private:
    void call(std::string variables, std::string domains, std::string assignments);


    void forward_check(std::string variables, std::string domains, std::string assignments, std::string vconstraints); // TODO maybe unassigned
};

template<typename T>
int Constraint<T>::get_max_sum() {
    return -1;
}

template<typename T>
std::unique_ptr<Constraint<T>> Constraint<T>::clone() {
    return std::make_unique<Constraint>(*this);
}

template<typename T>
void Constraint<T>::pre_process(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains,
                                std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> constraints,
                                std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> vconstraints) {

    if (vars.size() >= 1) {
        auto variable = vars[0];
        auto domain = domains[variable];
        for (const auto &value : domain.get_values()) {
            if (!(this->call(vars, domain, std::pair<T, int>(variable, value)))) {
//                domain.
            }
            auto a = 1;
        }
    }
//    if len(variables) == 1:
    //    variable = variables[0]
    //    domain = domains[variable]
//    for value in domain[:]:
    //    if not self(variables, domains, {variable: value}):
//          domain.remove(value)
//    constraints.remove((self, variables))
//    vconstraints[variable].remove((self, variables))
}

template<typename T>
void Constraint<T>::call(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains, std::pair<T, int> assignments) {

}

template<typename T>
class MaxSumConstraint : public Constraint<T> {

    //TODO init with maxsum and multipliers
public:
    int max_sum{-1};

    MaxSumConstraint() = default;

    ~MaxSumConstraint() override = default;

    int get_max_sum() override;

    explicit MaxSumConstraint(int m_sum) : max_sum(m_sum) {};

    std::unique_ptr<Constraint<T>> clone() override;

    void pre_process(
            std::vector<T> vars,
            std::unordered_map<T, Domain, CustomHasher<T>> domains,
            std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> vconstraints
    ) override;

    void call(std::vector<T> vars,
              std::unordered_map<T, Domain, CustomHasher<T>> domains,
              std::pair<T, int> assignments) override;

};

template<typename T>
int MaxSumConstraint<T>::get_max_sum() {
    return max_sum;
}

template<typename T>
std::unique_ptr<Constraint<T>> MaxSumConstraint<T>::clone() {
    return std::make_unique<MaxSumConstraint>(*this); // https://www.fluentcpp.com/2017/09/08/make-polymorphic-copy-modern-cpp/
}

template<typename T>
void MaxSumConstraint<T>::pre_process(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains,
                                      std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> constraints,
                                      std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> vconstraints) {
    Constraint<T>::pre_process(vars, domains, constraints, vconstraints);
}

template<typename T>
void MaxSumConstraint<T>::call(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains, std::pair<T, int> assignments) {
//    Constraint<T>::call(vars, domains, assignments);
    auto d = 1;
}

class BacktrackingSolver {
private:
    std::string get_solution_iter(std::string domains, std::string constraints, std::string vconstraints);

    std::string get_solution(std::string domains, std::string constraints, std::string vconstraints);

    std::string get_solutions(std::string domains, std::string constraints, std::string vconstraints);
};


template<typename T>
class Problem {
    BacktrackingSolver solver = BacktrackingSolver();
    std::unordered_map<T, Domain, CustomHasher<T>> variables;
    std::vector<std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>>> constraints;

public:

    void add_variable(std::string variable, std::string domain);

    void add_variables(std::vector<T> vars, const std::vector<int> &domain);

    void add_constraint(std::unique_ptr<Constraint<T>> constraint, std::vector<T> vars);

    std::string get_solutions();


private:
    void reset();


    BacktrackingSolver get_solver();


    std::string get_solution();


    std::string get_solution_iter();

    std::string get_args();
};

template<typename T>
void Problem<T>::add_variables(std::vector<T> vars, const std::vector<int> &domain_ints) {
    Domain domain(domain_ints);
    for (const auto &var : vars) {
        variables[var] = domain;
    }
    auto a = 1; // TODO is this function finished
}

template<typename T>
void Problem<T>::add_constraint(std::unique_ptr<Constraint<T>> constraint_ptr, std::vector<T> vars) {
    std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>> constraint_tuple(std::move(constraint_ptr), vars);
    constraints.push_back(std::move(constraint_tuple));
}

template<typename T>
struct ProblemArgs {
    std::unordered_map<T, Domain, CustomHasher<T>> processed_variables;
    std::vector<std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>>> processed_constraints;
    std::unordered_map<T, std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>>, CustomHasher<T>> processed_vconstraints;
};

template<typename T>
std::string Problem<T>::get_args() {
    // TODO Replicate the python code
    auto domains = variables; // Create a copy of variables
    std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> processed_constraints;
    processed_constraints.reserve(constraints.size());
    for (const auto &elem : constraints) {
        processed_constraints.push_back(std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>(elem.first->clone(), elem.second));
    }
    // We skip one step from the original python code of setting variables if empty, we assume always valid variables
    std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> vconstraints;
    std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> a;
    for (const auto &item : domains) {
        auto &variable = item.first;
        vconstraints[variable] = std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>();
    }
    for (const auto &constraint_tuple : processed_constraints) {
        auto &vars = constraint_tuple.second;
        for (const auto &variable : vars) {
            vconstraints[variable].push_back(constraint_tuple);
        }
    }
    for (const auto &constraint_tuple : processed_constraints) {
        auto &constraint = constraint_tuple.first;
        auto &vars = constraint_tuple.second;
        constraint->pre_process(vars, domains, processed_constraints, vconstraints);
        auto a = 1;
//        constraint.pre_process;
    }
//    for constraint, variables in constraints[:]:
//    constraint.preProcess(variables, domains, constraints, vconstraints)
    auto b = 1;
//    for constraint, variables in constraints:
//    for variable in variables:
//    vconstraints[variable].append((constraint, variables))

//    vconstraints = {}
//    for variable in domains:
//    vconstraints[variable] = []
//    for constraint, variables in constraints:
//    for variable in variables:
//    vconstraints[variable].append((constraint, variables))
    return std::string();
}

template<typename T>
std::string Problem<T>::get_solutions() {
    auto problem_args = get_args();
//    domains, constraints, vconstraints = self._getArgs()
//    if not domains:
//    return []
//    return self._solver.getSolutions(domains, constraints, vconstraints)
//    return "Solution";
}


#endif //MASTER_CSP_CPP_CONSTRAINT_HPP
