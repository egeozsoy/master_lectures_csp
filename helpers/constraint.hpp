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
    // TODO maybe something like set?, debug python code for this
public:
    Domain() = default;

    std::vector<int> get_values() { return values; }

    void remove_value(int value) { values.erase(std::remove(values.begin(), values.end(), value), values.end()); }

    explicit Domain(std::vector<int> vals) : values(std::move(vals)) {};

    void reset_state() {
        values = hidden;
        hidden.clear();
        states.clear();
    }

private:

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

    virtual bool call(std::vector<T> vars,
                      std::unordered_map<T, Domain, CustomHasher<T>> domains,
                      std::pair<T, int> assignments);

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
void Constraint<T>::pre_process(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains,
                                std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>> constraints,
                                std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<T>>>, CustomHasher<T>> vconstraints) {

    if (vars.size() == 1) {
        auto &variable = vars[0];
        auto &domain = domains[variable];
        for (const auto &value : domain.get_values()) {
            if (!(this->call(vars, domains, std::pair<T, int>(variable, value)))) {
                domain.remove_value(value);
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
bool Constraint<T>::call(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains, std::pair<T, int> assignments) {
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
            std::unordered_map<T, Domain, CustomHasher<T>> domains,
            std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> constraints,
            std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> vconstraints
    ) override;

    bool call(std::vector<T> vars,
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
    auto maxsum = get_max_sum();
    // Eliminate any variable that would on its own be bigger than the limit
    for (const auto &variable : vars) {
        auto &domain = domains[variable];
        for (const auto &value : domain.get_values()) {
            if (value > maxsum) {
                domain.remove_value(value);
            }
        }
    }
}

template<typename T>
bool MaxSumConstraint<T>::call(std::vector<T> vars, std::unordered_map<T, Domain, CustomHasher<T>> domains, std::pair<T, int> assignments) {
//    Constraint<T>::call(vars, domains, assignments);
    auto d = 1;
    return true;
}

template<typename T>
struct ProblemArgs {
    std::unordered_map<T, Domain, CustomHasher<T>> domains;
    std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> processed_constraints;
    std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> processed_vconstraints;
};

template<typename T>
class BacktrackingSolver {
public:
    std::string get_solutions(ProblemArgs<T> problem_args);
};

template<typename T>
std::string BacktrackingSolver<T>::get_solutions(ProblemArgs<T> problem_args) {
    std::unordered_map<T, int, CustomHasher<T>> assignments;
    std::vector<std::tuple<T, std::vector<int>, std::vector<Domain>>> queue;
    while (true) {
        //Mix the Degree and Minimum Remaing Values (MRV) heuristics
        std::vector<std::tuple<int, int, T>> lst;
        for (const auto &variable_pair : problem_args.domains) {
            lst.push_back(std::make_tuple(problem_args.processed_vconstraints[variable_pair.first].size(),
                                          problem_args.domains[variable_pair.first].get_values().size(),
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
        auto a = 1;

    }
    /*
            for item in lst:
                if item[-1] not in assignments:
                    # Found unassigned variable
                    variable = item[-1]
                    values = domains[variable][:]
                    if forwardcheck:
                        pushdomains = [
                            domains[x]
                            for x in domains
                            if x not in assignments and x != variable
                        ]
                    else:
                        pushdomains = None
                    break
            else:
                # No unassigned variables. We've got a solution. Go back
                # to last variable, if there's one.
                yield assignments.copy()
                if not queue:
                    return
                variable, values, pushdomains = queue.pop()
                if pushdomains:
                    for domain in pushdomains:
                        domain.popState()

            while True:
                # We have a variable. Do we have any values left?
                if not values:
                    # No. Go back to last variable, if there's one.
                    del assignments[variable]
                    while queue:
                        variable, values, pushdomains = queue.pop()
                        if pushdomains:
                            for domain in pushdomains:
                                domain.popState()
                        if values:
                            break
                        del assignments[variable]
                    else:
                        return

                # Got a value. Check it.
                assignments[variable] = values.pop()

                if pushdomains:
                    for domain in pushdomains:
                        domain.pushState()

                for constraint, variables in vconstraints[variable]:
                    if not constraint(variables, domains, assignments, pushdomains):
                        # Value is not good.
                        break
                else:
                    break

                if pushdomains:
                    for domain in pushdomains:
                        domain.popState()

            # Push state before looking for next variable.
            queue.append((variable, values, pushdomains))

        raise RuntimeError("Can't happen")
     */
    return std::string();
}


template<typename T>
class Problem {
    BacktrackingSolver<T> solver = BacktrackingSolver<T>();
    std::unordered_map<T, Domain, CustomHasher<T>> variables;
    std::vector<std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>>> constraints;

public:

    void add_variable(std::string variable, std::string domain);

    void add_variables(std::vector<T> vars, const std::vector<int> &domain);

    void add_constraint(std::unique_ptr<Constraint<T>> constraint, std::vector<T> vars);

    std::string get_solutions();


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
        variables[var] = domain;
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
    std::vector<Domain> vals;
    vals.reserve(domains.size());
    for (const auto &kv : domains) {
        vals.push_back(kv.second);
    }
    for (auto &domain: vals) {
        domain.reset_state();
        // TODO do we need a if not domain?
    }
    ProblemArgs<T> problem_args{domains, processed_constraints, vconstraints};
    return problem_args;
}

template<typename T>
std::string Problem<T>::get_solutions() {
    auto problem_args = get_args();
    // TODO do we need check for not domains?
    solver.get_solutions(problem_args);
    return "";
//    domains, constraints, vconstraints = self._getArgs()
//    if not domains:
//    return []
//    return self._solver.getSolutions(domains, constraints, vconstraints)
//    return "Solution";
}


#endif //MASTER_CSP_CPP_CONSTRAINT_HPP
