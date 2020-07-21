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


//TODO refactor and move all non template functions to .cpp
using Constraints =
class Domain {
    std::vector<int> hidden;
    std::vector<int> states;
    std::vector<int> values;
    // TODO maybe something like set?, debug python code for this
public:
    Domain() = default;

    std::vector<int> get_values() { return values; }

    void remove_value(int value) { values.erase(std::remove(values.begin(), values.end(), value), values.end()); }

    explicit Domain(std::vector<int> vals) : values(std::move(vals)) {};

    void reset_state() {
        for (const auto &item : hidden) {
            values.push_back(item);
        }
        hidden.clear();
        states.clear();
    }

    void pop_state();

    void push_state() {
        states.push_back(values.size());
    }

    void hide_value(int value) {
        remove_value(value);
        hidden.push_back(value);
    }

private:

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
            std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
            std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> vconstraints
    );

    virtual bool call(std::vector<T> vars,
                      std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
                      std::unordered_map<T, int, CustomHasher<T>> assignments);

private:
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
void Constraint<T>::pre_process(std::vector<T> vars, std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
                                std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> constraints,
                                std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> vconstraints) {

    if (vars.size() == 1) {
        auto &variable = vars[0];
        auto &domain = domains[variable];
        for (const auto &value : domain->get_values()) {
            std::unordered_map<T, int, CustomHasher<T>> assignments;
            assignments[variable] = value;
            if (!(this->call(vars, domains, assignments))) {
                domain->remove_value(value);
            }
            // Delete constraint
            auto idx = std::find_if(constraints.begin(), constraints.end(),
                                    [this](auto constraint_pair) { return &(*(constraint_pair.first)) == this; });
            constraints.erase(idx); // TODO test this with more than 1 constraint
            auto v_idx = std::find_if(vconstraints[variable].begin(), vconstraints[variable].end(),
                                      [this](auto constraint_pair) { return &(*(constraint_pair.first)) == this; });
            vconstraints[variable].erase(v_idx);
        }
    }
}

template<typename T>
bool
Constraint<T>::call(std::vector<T> vars, std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
                    std::unordered_map<T, int, CustomHasher<T>> assignments) {
    return true;
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
            std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
            std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> vconstraints
    ) override;

    bool call(std::vector<T> vars,
              std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
              std::unordered_map<T, int, CustomHasher<T>> assignments) override;

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
void MaxSumConstraint<T>::pre_process(std::vector<T> vars, std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
                                      std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> constraints,
                                      std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> vconstraints) {
    Constraint<T>::pre_process(vars, domains, constraints, vconstraints);
    auto maxsum = get_max_sum();
    // Eliminate any variable that would on its own be bigger than the limit
    for (const auto &variable : vars) {
        auto &domain = domains[variable];
        for (const auto &value : domain->get_values()) {
            if (value > maxsum) {
                domain->remove_value(value);
            }
        }
    }
}

template<typename T>
bool MaxSumConstraint<T>::call(std::vector<T> vars, std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains,
                               std::unordered_map<T, int, CustomHasher<T>> assignments) {
    auto total_sum = 0;
    for (const auto &variable : vars) {
        if (assignments.find(variable) != assignments.end()) { // If variable in assignments
            total_sum += assignments.at(variable);
        }
    }
    if (total_sum > max_sum) {
        return false;
    }
    //Forwardchecking
    for (const auto &variable : vars) {
        if (assignments.find(variable) == assignments.end()) { // If variable not in assignments
            auto domain = domains.at(variable);
            for (const auto &value : domain->get_values()) {
                if (total_sum + value > max_sum) {
                    domain->hide_value(value);
                }
            }
            if (domain->get_values().empty()) {
                return false;
            }
        }
    }
    return true;
}

template<typename T>
struct ProblemArgs {
    std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains; // TODO this should became a shared ptr
    std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> processed_constraints;
    std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> processed_vconstraints;
};

template<typename T>
class BacktrackingSolver {
public:
    std::vector<std::unordered_map<T, int, CustomHasher<T>>> get_solutions(ProblemArgs<T> problem_args);
};

template<typename T>
std::vector<std::unordered_map<T, int, CustomHasher<T>>> BacktrackingSolver<T>::get_solutions(ProblemArgs<T> problem_args) {
    std::vector<std::unordered_map<T, int, CustomHasher<T>>> solutions;
    std::unordered_map<T, int, CustomHasher<T>> assignments;
    std::vector<std::tuple<T, std::vector<int>, std::vector<std::shared_ptr<Domain>>>> queue;
    while (true) {
        //Mix the Degree and Minimum Remaing Values (MRV) heuristics
        std::vector<std::tuple<int, int, T>> lst;
        for (const auto &variable_pair : problem_args.domains) {
            lst.push_back(std::make_tuple(-problem_args.processed_vconstraints[variable_pair.first].size(),
                                          problem_args.domains[variable_pair.first]->get_values().size(),
                                          variable_pair.first));
        }
        // TODO test this well
        std::sort(lst.begin(), lst.end(), [](const auto &lhs, const auto &rhs) {
            if (std::get<0>(lhs) == std::get<0>(rhs)) { // Different sorting if the first values are same
                if (std::get<1>(lhs) == std::get<1>(rhs)) { // Different sorting if the second values are also the same
                    return std::get<2>(lhs).name < std::get<2>(rhs).name;
                }
                return std::get<1>(lhs) < std::get<1>(rhs);
            }
            return std::get<0>(lhs) < std::get<0>(rhs);
        });
        std::vector<std::shared_ptr<Domain>> pushdomains;
        std::vector<int> values;
        bool unassigned_variables = false;
        const T *variable_ptr = nullptr;
        for (const auto &item : lst) {
            variable_ptr = &(std::get<2>(item));
            const auto &variable = *variable_ptr;
            if (assignments.find(variable) == assignments.end()) { //if not found
                values = problem_args.domains[variable]->get_values();
                pushdomains.clear();
                for (const auto &domain_pair : problem_args.domains) {
                    if (assignments.find(domain_pair.first) == assignments.end() && (domain_pair.first.name != variable.name)) { //TODO check this
                        pushdomains.push_back(domain_pair.second); // TODO this shouldn't make a copy but be the same
                    }
                }
                unassigned_variables = true;
                break;
            }
        }
        if (!unassigned_variables) {
            solutions.push_back(assignments);
            if (queue.empty()) {
                return solutions;
            }
            auto last_queue_element = queue.back();
            queue.pop_back();
            variable_ptr = &(std::get<0>(last_queue_element));
            values = std::get<1>(last_queue_element);
            pushdomains = std::get<2>(last_queue_element); // TODO debug this
            if (!pushdomains.empty()) {
                for (auto &p_domain : pushdomains) {
                    p_domain->pop_state();
                }
            }
        }
        while (true) {
            if (values.empty()) {
                const auto &variable = *variable_ptr; // TODO make sure it can't be nullptr
                assignments.erase(variable);
                while (!queue.empty()) {
                    auto last_element = queue.back();
                    queue.pop_back();
                    variable_ptr = &(std::get<0>(last_element));
                    values = std::get<1>(last_element);
                    pushdomains = std::get<2>(last_element); // TODO debug this
                    if (!pushdomains.empty()) {
                        for (auto &p_domain : pushdomains) {
                            p_domain->pop_state();
                        }
                    }
                    if (!values.empty()) {
                        break;
                    }
                    const auto &variable_new = *variable_ptr;
                    assignments.erase(variable_new);
                }
                if (queue.empty()) {
                    return solutions;
                }
            }
            const auto &variable = *variable_ptr; // TODO this probably doesn't work correctly if values empty, make sure it is never the case
            assignments[variable] = values.back();
            values.pop_back();

            if (!pushdomains.empty()) {
                for (auto &p_domain : pushdomains) {
                    p_domain->push_state();
                }
            }
            //TODO problem_args.domain is still different than pushback.domain. Fix unique_ptr solution we have right now, it probably should be a shared ptr
            auto break_exit = false;
            for (const auto &constraint_var_pair : problem_args.processed_vconstraints[variable]) {
                if (!(constraint_var_pair.first->call(constraint_var_pair.second, problem_args.domains, assignments))) {
                    //value is not good
                    break_exit = true;
                    break;
                }
            }
            if (!break_exit) {
                break;
            }
            if (!pushdomains.empty()) {
                for (auto &p_domain : pushdomains) {
                    p_domain->pop_state();
                }
            }
        }
        queue.push_back(std::make_tuple(*variable_ptr, values, pushdomains));
    }
    return solutions;
}


template<typename T>
class Problem {
    BacktrackingSolver<T> solver = BacktrackingSolver<T>();
    std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> variables;
    std::vector<std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>>> constraints;

public:

    void add_variable(std::string variable, std::string domain);

    void add_variables(std::vector<T> vars, const std::vector<int> &domain);

    void add_constraint(std::unique_ptr<Constraint<T>> constraint, std::vector<T> vars);

    std::vector<std::unordered_map<T, int, CustomHasher<T>>> get_solutions();


private:
    void reset();


    BacktrackingSolver<T> get_solver();


    std::string get_solution();


    std::string get_solution_iter();

    ProblemArgs<T> get_args();
};

template<typename T>
void Problem<T>::add_variables(std::vector<T> vars, const std::vector<int> &domain_ints) {
    Domain domain(domain_ints);
    for (const auto &var : vars) {
        variables[var] = std::make_shared<Domain>(domain);
    }
    auto a = 1; // TODO is this function finished?
}

template<typename T>
void Problem<T>::add_constraint(std::unique_ptr<Constraint<T>> constraint_ptr, std::vector<T> vars) {
    std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>> constraint_tuple(std::move(constraint_ptr), vars);
    constraints.push_back(std::move(constraint_tuple));
}


template<typename T>
ProblemArgs<T> Problem<T>::get_args() {
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
    }
    std::vector<std::shared_ptr<Domain>> vals;
    vals.reserve(domains.size());
    for (const auto &kv : domains) {
        vals.push_back(kv.second);
    }
    for (auto &domain: vals) {
        domain->reset_state();
        // TODO do we need a if not domain?
    }
    ProblemArgs<T> problem_args{domains, processed_constraints, vconstraints};
    return problem_args;
}

template<typename T>
std::vector<std::unordered_map<T, int, CustomHasher<T>>> Problem<T>::get_solutions() {
    auto problem_args = get_args();
    // TODO do we need check for not domains?
    // TODO continue testing from here
    return solver.get_solutions(problem_args);
//    domains, constraints, vconstraints = self._getArgs()
//    if not domains:
//    return []
//    return self._solver.getSolutions(std::shared_ptr<Domain>s, constraints, vconstraints)
//    return "Solution";
}


#endif //MASTER_CSP_CPP_CONSTRAINT_HPP
