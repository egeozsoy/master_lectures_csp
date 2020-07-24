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
#include <memory>
#include <algorithm>


using Constraints =
class Domain {
    std::vector<int> hidden;
    std::vector<int> states;
    std::vector<int> values;
public:
    Domain() = default;

    std::vector<int> &get_values() { return values; }

    void remove_value(int value) { values.erase(std::remove(values.begin(), values.end(), value), values.end()); }

    explicit Domain(std::vector<int> vals) : values(std::move(vals)) {};

    void reset_state() {
        for (const auto &item : hidden) {
            values.push_back(item);
        }
        hidden.clear();
        states.clear();
    }

    void pop_state() {
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
        hidden.erase(hidden.end() - diff, hidden.end());
    }

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
    size_t operator()(const T &t) const { return t.hash; }
};

template<typename T>
struct Proxy {
    T *t_pointer;
    int index;
/*    unsigned long hash; While it is possible to use a "proper" hash, such as from <T>, in the current
 * configuration we can just rely on the index, as Proxy objects are generated only in one location.
*/

public:
    Proxy(T *t_ptr, const int i) : t_pointer(t_ptr), index(i) {}

    bool operator==(const Proxy &other) const {
        return this->index == other.index;
    }

    bool operator!=(const Proxy &other) const {
        return this->index != other.index;
    }

    bool operator<(const Proxy &other) const {
        return this->index < other.index;
    }
};

template<typename T>
struct CustomProxyHasher {
    size_t operator()(const Proxy<T> &t) const { return t.index; }
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
            const std::vector<Proxy<T>> &vars,
            const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
            std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<Proxy<T>>>> &constraints,
            std::unordered_map<Proxy<T>, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<Proxy<T>>>>, CustomProxyHasher<T>> &vconstraints
    );

    virtual bool call(const std::vector<Proxy<T>> &/*unused*/,
                      const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &/*unused*/,
                      const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &/*unused*/) const;

    virtual bool forward_check(const std::vector<Proxy<T>> &vars,
                               const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                               const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments) const;

    virtual bool func(const std::vector<int> &parms, const std::vector<Proxy<T>> &vars) const;

};

template<typename T>
std::unique_ptr<Constraint<T>> Constraint<T>::clone() {
    return std::make_unique<Constraint>(*this);
}

template<typename T>
void Constraint<T>::pre_process(const std::vector<Proxy<T>> &vars,
                                const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                                std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<Proxy<T>>>> &constraints,
                                std::unordered_map<Proxy<T>, std::vector<std::pair<std::shared_ptr<Constraint>, std::vector<Proxy<T>>>>, CustomProxyHasher<T>> &vconstraints) {

    if (vars.size() == 1) {
        auto &variable = vars[0];
        auto &domain = domains.at(variable);
        for (const auto &value : domain->get_values()) {
            std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> assignments;
            assignments[variable] = value;
            if (!(this->call(vars, domains, assignments))) {
                domain->remove_value(value);
            }
            // Delete constraint
            auto idx = std::find_if(constraints.begin(), constraints.end(),
                                    [this](auto constraint_pair) { return &(*(constraint_pair.first)) == this; });
            constraints.erase(idx);
            auto v_idx = std::find_if(vconstraints.at(variable).begin(), vconstraints.at(variable).end(),
                                      [this](auto constraint_pair) { return &(*(constraint_pair.first)) == this; });
            vconstraints[variable].erase(v_idx);
        }
    }
}

template<typename T>
bool
Constraint<T>::call(const std::vector<Proxy<T>> &/*unused*/,
                    const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &/*unused*/,
                    const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &/*unused*/) const {
    throw std::logic_error("This function shouldn't be called");
}

template<typename T>
bool Constraint<T>::forward_check(const std::vector<Proxy<T>> &vars,
                                  const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                                  const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments) const {

    std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> tmp_assignments = assignments; // This makes copies but keeps function const, not sure if it is the best trade-off
    // We represent unassigned always with -1
    auto unassigned_variable = vars[0]; // As a placeholder
    auto unassigned_variable_count = 0;
    for (const auto &var : vars) {
        if (tmp_assignments.count(var) == 0) {//if variable not in assignments
            ++unassigned_variable_count;
            unassigned_variable = var;
            if (unassigned_variable_count > 1) {
                break;
            }
        }
    }
    if (unassigned_variable_count == 1) {
        auto &domain = domains.at(unassigned_variable);
        if (!domain->get_values().empty()) {
            for (const auto &value : domain->get_values()) {
                tmp_assignments[unassigned_variable] = value;
                if (!call(vars, domains, tmp_assignments)) {
                    domain->hide_value(value);
                }
            }
            tmp_assignments.erase(unassigned_variable);
        }
        if (domain->get_values().empty()) {
            return false;
        }
    }
    return true;
}

template<typename T>
bool Constraint<T>::func(const std::vector<int> &/*unused*/, const std::vector<Proxy<T>> &/*unused*/) const {
    throw std::logic_error("This function shouldn't be called");
}

template<typename T>
class MaxSumConstraint : public Constraint<T> {

public:
    const int max_sum{-1};

    MaxSumConstraint() = default;

    explicit MaxSumConstraint(int m_sum) : max_sum(m_sum) {};

    std::unique_ptr<Constraint<T>> clone() override;

    void pre_process(
            const std::vector<Proxy<T>> &vars,
            const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
            std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>> &constraints,
            std::unordered_map<Proxy<T>, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>>, CustomProxyHasher<T>> &vconstraints
    ) override;

    bool call(const std::vector<Proxy<T>> &vars,
              const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
              const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments) const override;
};

template<typename T>
std::unique_ptr<Constraint<T>> MaxSumConstraint<T>::clone() {
    return std::make_unique<MaxSumConstraint>(*this); // https://www.fluentcpp.com/2017/09/08/make-polymorphic-copy-modern-cpp/
}

template<typename T>
void MaxSumConstraint<T>::pre_process(const std::vector<Proxy<T>> &vars,
                                      const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                                      std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>> &constraints,
                                      std::unordered_map<Proxy<T>, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>>, CustomProxyHasher<T>> &vconstraints) {
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
bool MaxSumConstraint<T>::call(const std::vector<Proxy<T>> &vars,
                               const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                               const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments) const {
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
class FunctionConstraint : public Constraint<T> {
public:
    std::unique_ptr<Constraint<T>> clone() override;

    bool call(const std::vector<Proxy<T>> &vars,
              const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
              const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments) const override;

    bool func(const std::vector<int> &parms, const std::vector<Proxy<T>> &vars) const override;

private:
};

template<typename T>
std::unique_ptr<Constraint<T>> FunctionConstraint<T>::clone() {
    return std::make_unique<FunctionConstraint>(*this);
}

template<typename T>
bool FunctionConstraint<T>::call(const std::vector<Proxy<T>> &vars,
                                 const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                                 const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments) const {

    std::vector<int> parms;
    for (const auto &item : vars) {
        if (assignments.count(item) > 0) {
            parms.push_back(assignments.at(item));
        } else {
            parms.push_back(-1);
        }
    }
    auto missing = std::count(parms.begin(), parms.end(), -1);
    if (missing > 0) {
        return (missing != 1) || (this->forward_check(vars, domains, assignments)); // maybe make forward checking optional
    }
    return func(parms, vars);

}

template<typename T>
bool FunctionConstraint<T>::func(const std::vector<int> &/*unused*/, const std::vector<Proxy<T>> &/*unused*/) const {
    throw std::logic_error("This function shouldn't be called");
}


template<typename T>
struct ProblemArgs {
    const std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> domains;
    const std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>> processed_constraints;
    const std::unordered_map<T, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<T>>>, CustomHasher<T>> processed_vconstraints;
};

template<typename T>
class BacktrackingSolver {
public:
    std::vector<std::unordered_map<T, int, CustomHasher<T>>> get_solutions(ProblemArgs<T> &problem_args);

private:
    std::tuple<Proxy<T>, std::vector<int>, bool> pushback_domains(const std::vector<std::tuple<int, int, Proxy<T>>> &lst,
                                                                  const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments,
                                                                  const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                                                                  std::vector<std::shared_ptr<Domain>> &pushdomains);

    std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>, bool>
    deal_values_empty(std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments,
                      std::vector<std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>>> &queue);

    std::vector<std::unordered_map<T, int, CustomHasher<T>>>
    return_original_data_solution(const std::vector<std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>>> &solutions);
};

template<typename T>
std::tuple<Proxy<T>, std::vector<int>, bool> BacktrackingSolver<T>::pushback_domains(const std::vector<std::tuple<int, int, Proxy<T>>> &lst,
                                                                                     const std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments,
                                                                                     const std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> &domains,
                                                                                     std::vector<std::shared_ptr<Domain>> &pushdomains
) {
    for (const auto &item : lst) {
        auto &variable = std::get<2>(item);
        if (assignments.count(variable) == 0) { //if not found
            auto &values = domains.at(variable)->get_values();
            pushdomains.clear();
            for (const auto &domain_pair : domains) {
                if (assignments.find(domain_pair.first) == assignments.end() && (domain_pair.first.index != variable.index)) {
                    pushdomains.push_back(domain_pair.second);
                }
            }
            return std::tuple<Proxy<T>, std::vector<int>, bool>(variable, values, true);
        }
    }
    auto &variable = std::get<2>(lst.back());
    return std::tuple<Proxy<T>, std::vector<int>, bool>(variable, domains.at(variable)->get_values(),
                                                        false); // Second value indicates unassigned variables
}

template<typename T>
std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>, bool>
BacktrackingSolver<T>::deal_values_empty(std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> &assignments,
                                         std::vector<std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>>> &queue) {
    auto &dummy_last_element = queue.back();
    auto dummy_variable = std::get<0>(dummy_last_element);
    auto dummy_values = std::get<1>(dummy_last_element);
    auto dummy_pushdomains = std::get<2>(dummy_last_element);
    while (!queue.empty()) {
        auto &last_element = queue.back();
        auto &variable = std::get<0>(last_element);
        auto &values = std::get<1>(last_element);
        auto &pushdomains = std::get<2>(last_element);
        if (!pushdomains.empty()) {
            for (auto &p_domain : pushdomains) {
                p_domain->pop_state();
            }
        }
        if (!values.empty()) {
            auto return_tpl = std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>, bool>(variable, values, pushdomains, true);
            queue.pop_back();
            return return_tpl;
        }
        assignments.erase(variable);
        queue.pop_back();

    }
    return std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>, bool>(dummy_variable, dummy_values, dummy_pushdomains, false);
}


template<typename T>
std::vector<std::unordered_map<T, int, CustomHasher<T>>>
BacktrackingSolver<T>::return_original_data_solution(const std::vector<std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>>> &solutions) {
    std::vector<std::unordered_map<T, int, CustomHasher<T>>> original_solutions;
    for (const auto &solution : solutions) {
        auto &u_map = original_solutions.emplace_back();
        u_map.reserve(solution.size());
        for (const auto &item : solution) {
            u_map[*item.first.t_pointer] = item.second;
        }
    }
    return original_solutions;
}

template<typename T>
std::vector<std::unordered_map<T, int, CustomHasher<T>>> BacktrackingSolver<T>::get_solutions(ProblemArgs<T> &problem_args) {
    int count = 0;
    std::vector<T> original_datas;

    for (const auto &variable_pair : problem_args.domains) {
        original_datas.emplace_back(variable_pair.first);
    }
    std::sort(original_datas.begin(), original_datas.end());
    std::vector<Proxy<T>> proxies;
    int idx = 0;
    for (auto &original_data : original_datas) {
        proxies.emplace_back(&original_data, idx);
        ++idx;
    }
    std::unordered_map<Proxy<T>, std::shared_ptr<Domain>, CustomProxyHasher<T>> domains;
    std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>> processed_constraints;
    std::unordered_map<Proxy<T>, std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>>, CustomProxyHasher<T>> processed_vconstraints;
    for (size_t i = 0; i < problem_args.domains.size(); ++i) {
        domains[proxies[i]] = problem_args.domains.at(original_datas[i]);
    }
    for (const auto &processed_constraint : problem_args.processed_constraints) {
        processed_constraints.emplace_back(processed_constraint.first, proxies);
    }
    for (size_t i = 0; i < problem_args.processed_vconstraints.size(); ++i) {
        auto &var = problem_args.processed_vconstraints.at(original_datas[i]);
        std::vector<std::pair<std::shared_ptr<Constraint<T>>, std::vector<Proxy<T>>>> vec;
        for (const auto &item : var) {
            vec.emplace_back(item.first, proxies);
        }
        processed_vconstraints[proxies[i]] = std::move(vec);
    }

    std::vector<std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>>> solutions;
    std::unordered_map<Proxy<T>, int, CustomProxyHasher<T>> assignments;
    std::vector<std::tuple<Proxy<T>, std::vector<int>, std::vector<std::shared_ptr<Domain>>>> queue;
    std::vector<std::tuple<int, int, Proxy<T>>> lst;
    std::vector<std::shared_ptr<Domain>> pushdomains;
    std::vector<int> values;

    while (true) {
        //Mix the Degree and Minimum Remaing Values (MRV) heuristics
        lst.clear();
        for (const auto &variable_pair : domains) {
            lst.emplace_back(-processed_vconstraints.at(variable_pair.first).size(),
                             domains.at(variable_pair.first)->get_values().size(),
                             variable_pair.first);
        }
        // Requires T to be sortable
        std::sort(lst.begin(), lst.end());
        pushdomains.clear();
        values.clear();
        auto pushback_domains_tpl = pushback_domains(lst, assignments, domains, pushdomains);
        auto variable = std::get<0>(pushback_domains_tpl);
        values = std::get<1>(pushback_domains_tpl);
        auto unassigned_variables = std::get<2>(pushback_domains_tpl);

        if (!unassigned_variables) {
            count += 1;

            solutions.push_back(assignments);
            if (queue.empty()) {
                return return_original_data_solution(solutions);
            }
            const auto &last_queue_element = queue.back();
            variable = std::get<0>(last_queue_element);
            values = std::get<1>(last_queue_element);
            pushdomains = std::get<2>(last_queue_element);
            queue.pop_back();
            if (!pushdomains.empty()) {
                for (auto &p_domain : pushdomains) {
                    p_domain->pop_state();
                }
            }
        }
        while (true) {
            if (values.empty()) {
                assignments.erase(variable);
                auto deal_tpl = deal_values_empty(assignments, queue);
                variable = std::get<0>(deal_tpl);
                values = std::get<1>(deal_tpl);
                pushdomains = std::get<2>(deal_tpl);
                auto break_exit = std::get<3>(deal_tpl);
                if (!break_exit) {
                    return return_original_data_solution(solutions);
                }
            }
            assignments[variable] = values.back();
            values.pop_back();

            if (!pushdomains.empty()) {
                for (auto &p_domain : pushdomains) {
                    p_domain->push_state();
                }
            }
            auto break_exit = false;
            for (const auto &constraint_var_pair : processed_vconstraints.at(variable)) {
                if (!(constraint_var_pair.first->call(constraint_var_pair.second, domains, assignments))) {
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
        queue.emplace_back(variable, values, pushdomains);
    }
    return return_original_data_solution(solutions);
}

template<typename T>
class Problem {
    BacktrackingSolver<T> solver = BacktrackingSolver<T>();
    std::unordered_map<T, std::shared_ptr<Domain>, CustomHasher<T>> variables;
    std::vector<std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>>> constraints;

public:
    void add_variables(const std::vector<T> &vars, const std::vector<int> &domain);

    void add_constraint(std::unique_ptr<Constraint<T>> constraint, const std::vector<T> &vars);

    std::vector<std::unordered_map<T, int, CustomHasher<T>>> get_solutions();


private:

    ProblemArgs<T> get_args();
};

template<typename T>
void Problem<T>::add_variables(const std::vector<T> &vars, const std::vector<int> &domain_ints) {
    Domain domain(domain_ints);
    for (const auto &var : vars) {
        variables[var] = std::make_shared<Domain>(domain);
    }
}

template<typename T>
void Problem<T>::add_constraint(std::unique_ptr<Constraint<T>> constraint_ptr, const std::vector<T> &vars) {
    std::pair<std::unique_ptr<Constraint<T>>, std::vector<T>> constraint_tuple(std::move(constraint_ptr), vars);
    constraints.push_back(std::move(constraint_tuple));
}


template<typename T>
ProblemArgs<T> Problem<T>::get_args() {
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
