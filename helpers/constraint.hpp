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


using Constraints =
class Domain {
    std::vector<int> hidden;
    std::vector<int> states;
    std::vector<int> values;
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

    void hide_value(const int value) {
        remove_value(value);
        hidden.push_back(value);
    }

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

    // Implement default copy and move operations
    Constraint(const Constraint &) = default;

    Constraint &operator=(const Constraint &) = default;

    Constraint(Constraint &&) noexcept = default;

    Constraint &operator=(Constraint &&) noexcept = default;

    virtual std::unique_ptr<Constraint<T>> clone();

    virtual void pre_process(
            const std::vector<T> &vars,
            const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
            std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> &constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> &vconstraints
    );

    virtual bool call(const std::vector<T> &vars,
                      const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
                      const std::unordered_map<T, int, CustomHasher<T>> &assignments) const;

private:
    void forward_check(std::string variables, std::string domains, std::string assignments, std::string vconstraints);
};

template<typename T>
std::unique_ptr<Constraint<T>> Constraint<T>::clone() {
    return std::make_unique<Constraint>(*this);
}

template<typename T>
void Constraint<T>::pre_process(const std::vector<T> &vars,
                                const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
                                std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> &constraints,
                                std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> &vconstraints) {

    if (vars.size() == 1) {
        auto &variable = vars[0];
        auto &domain = domains.at(variable);
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
            auto v_idx = std::find_if(vconstraints.at(variable).begin(), vconstraints.at(variable).end(),
                                      [this](auto constraint_pair) { return &(*(constraint_pair.first)) == this; });
            vconstraints[variable].erase(v_idx);
        }
    }
}

template<typename T>
bool
Constraint<T>::call(const std::vector<T> &/*unused*/,
                    const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &/*unused*/,
                    const std::unordered_map<T, int, CustomHasher<T>> &/*unused*/) const {
    return true; // TODO This call should never be called, maybe check this with assert
}

template<typename T>
class MaxSumConstraint : public Constraint<T> {

public:
    const int max_sum{-1};

    MaxSumConstraint() = default;

    explicit MaxSumConstraint(int m_sum) : max_sum(m_sum) {};

    std::unique_ptr<Constraint<T>> clone() override;

    void pre_process(
            const std::vector<T> &vars,
            const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
            std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> &constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> &vconstraints
    ) override;

    bool call(const std::vector<T> &vars,
              const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
              const std::unordered_map<T, int, CustomHasher<T>> &assignments) const override;
};

template<typename T>
std::unique_ptr<Constraint<T>> MaxSumConstraint<T>::clone() {
    return std::make_unique<MaxSumConstraint>(*this); // https://www.fluentcpp.com/2017/09/08/make-polymorphic-copy-modern-cpp/
}

template<typename T>
void MaxSumConstraint<T>::pre_process(const std::vector<T> &vars,
                                      const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
                                      std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> &constraints,
                                      std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> &vconstraints) {
    Constraint<T>::pre_process(vars, domains, constraints, vconstraints);
    auto maxsum = max_sum;
    // Eliminate any variable that would on its own be bigger than the limit
    for (const auto &variable : vars) {
        auto &domain = domains.at(variable);
        for (const auto &value : domain->get_values()) {
            if (value > maxsum) {
                domain->remove_value(value);
            }
        }
    }
}

template<typename T>
bool MaxSumConstraint<T>::call(const std::vector<T> &vars,
                               const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> &domains,
                               const std::unordered_map<T, int, CustomHasher<T>> &assignments) const {
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
struct ProblemArgs { // TODO continue here
    const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains;
    const std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> processed_constraints;
    const std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> processed_vconstraints;
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
            lst.push_back(std::make_tuple(-problem_args.processed_vconstraints.at(variable_pair.first).size(),
                                          problem_args.domains.at(variable_pair.first)->get_values().size(),
                                          variable_pair.first));
        }
        std::sort(lst.begin(), lst.end()); // Requires T to be sortable
        std::vector<std::shared_ptr<Domain>> pushdomains;
        std::vector<int> values;
        bool unassigned_variables = false;
        const T *variable_ptr = nullptr;
        for (const auto &item : lst) {
            variable_ptr = &(std::get<2>(item));
            const auto &variable = *variable_ptr;
            if (assignments.find(variable) == assignments.end()) { //if not found
                values = problem_args.domains.at(variable)->get_values();
                pushdomains.clear();
                for (const auto &domain_pair : problem_args.domains) {
                    if (assignments.find(domain_pair.first) == assignments.end() && (domain_pair.first.name != variable.name)) {
                        pushdomains.push_back(domain_pair.second);
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
            const auto last_queue_element = queue.back();
            queue.pop_back();
            variable_ptr = &(std::get<0>(last_queue_element));
            values = std::get<1>(last_queue_element);
            pushdomains = std::get<2>(last_queue_element);
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
                    const auto last_element = queue.back();
                    queue.pop_back();
                    variable_ptr = &(std::get<0>(last_element));
                    values = std::get<1>(last_element);
                    pushdomains = std::get<2>(last_element);
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
            auto break_exit = false;
            for (const auto &constraint_var_pair : problem_args.processed_vconstraints.at(variable)) {
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

    void add_variables(const std::vector<T> &vars, const std::vector<int> &domain);

    void add_constraint(std::unique_ptr<Constraint<T>> constraint, const std::vector<T> &vars);

    std::vector<std::unordered_map<T, int, CustomHasher<T>>> get_solutions();


private:
    void reset();


    BacktrackingSolver<T> get_solver();


    std::string get_solution();


    std::string get_solution_iter();

    ProblemArgs<T> get_args();
};

template<typename T>
void Problem<T>::add_variables(const std::vector<T> &vars, const std::vector<int> &domain_ints) {
    Domain domain(domain_ints);
    for (const auto &var : vars) {
        variables[var] = std::make_shared<Domain>(domain);
    }
    // TODO is this function finished?
}

template<typename T>
void Problem<T>::add_constraint(std::unique_ptr<Constraint<T>> constraint_ptr, const std::vector<T> &vars) {
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
    }
    ProblemArgs<T> problem_args{domains, processed_constraints, vconstraints};
    return problem_args;
}

template<typename T>
std::vector<std::unordered_map<T, int, CustomHasher<T>>> Problem<T>::get_solutions() {
    auto problem_args = get_args();
    return solver.get_solutions(problem_args);
}


#endif //MASTER_CSP_CPP_CONSTRAINT_HPP
