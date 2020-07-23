#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <numeric>
#include "helpers/lecture.hpp"
#include "helpers/constraint.hpp"

#include "xlnt/xlnt.hpp"
#include <chrono>  // for high_resolution_clock

using std::string;
using std::vector;


class AreaConstraint : public Constraint<Lecture> {
    static constexpr auto credit_constraint = 120;
public:
    std::unique_ptr<Constraint<Lecture>> clone() override;

    bool func(const std::vector<int> &parms, const std::vector<Proxy<Lecture>> &vars) const override;
};

std::unique_ptr<Constraint<Lecture>> AreaConstraint::clone() {
    return std::make_unique<AreaConstraint>(*this);
}

bool AreaConstraint::func(const std::vector<int> &parms, const std::vector<Proxy<Lecture>> &vars) const {
    // TODO make this an actual area constraint
    int total_sum = 0; // TODO consider existing credits as well
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == 1) {
            total_sum += vars[i].t_pointer->ec;
        }
    }
    return total_sum >= credit_constraint;
}

class CreditConstraint : public FunctionConstraint<Lecture> {
    static constexpr auto credit_constraint = 120;
    int existing_credits;
public:
    explicit CreditConstraint(int existing_ec) : existing_credits(existing_ec) {};

    std::unique_ptr<Constraint<Lecture>> clone() override;

    bool func(const std::vector<int> &parms, const std::vector<Proxy<Lecture>> &vars) const override;

};

std::unique_ptr<Constraint<Lecture>> CreditConstraint::clone() {
    return std::make_unique<CreditConstraint>(*this);
}

bool CreditConstraint::func(const vector<int> &parms, const vector<Proxy<Lecture>> &vars) const {
    int total_sum = existing_credits;
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == 1) {
            total_sum += vars[i].t_pointer->ec;
        }
    }
    return total_sum >= credit_constraint;
}


class TheoConstraint : public Constraint<Lecture> {
    const std::shared_ptr<vector<Lecture>> lectures;
public:
    explicit TheoConstraint(std::shared_ptr<vector<Lecture>> lects) : lectures(std::move(lects)) {};


};

class ProblemWrapper {

    static constexpr int credit_limit = 120;
    static constexpr int theo_limit = 10;
    static constexpr int highest_ec_limit = 18;
    static constexpr int normal_ec_limit = 8;
    vector<int> area_limit = {highest_ec_limit, normal_ec_limit, normal_ec_limit};
    static constexpr int max_allowed_lectures = 8;
    // TODO should be a set
    vector<string> taken_lecture_names = {"Computer Vision I: Variational Methods",
                                          "Computer Vision II: Multiple View Geometry",
                                          "Natural Language Processing", "Introduction to Deep Learning",
                                          "Advanced Deep Learning for Computer Vision"};

    vector<string> preferred_areas = {"COMPUTER GRAPHICS AND VISION", "MACHINE LEARNING AND ANALYTICS",
                                      "DIGITAL BIOLOGY AND DIGITAL MEDICINE"};

    vector<Lecture> lectures;
    vector<Lecture> taken_lectures;
    Problem<Lecture> problem;

    int existing_credits = 0;
    int existing_theo_credits = 0;

    auto create_lectures() {
        xlnt::workbook wb;
        wb.load("../tum_lectures.xlsx");
        auto ws = wb.active_sheet();
        string current_area = "none";
        for (auto row : ws.rows(false)) {
            auto first_column = row[0].to_string();
            if (first_column.find("ELECTIVE MODULES OF THE AREA") != string::npos) {
                auto begin_quote_idx = first_column.find('\"');
                auto end_quote_idx = first_column.find('\"', begin_quote_idx + 1);
                current_area = first_column.substr(begin_quote_idx + 1, end_quote_idx - begin_quote_idx - 1);
            }
            if (first_column.substr(0, 2) == "IN") {
                string name = row[2].to_string();
                constexpr int ec_col = 5;
                constexpr int theo_col = 7;
                int ec = std::stoi(row[ec_col].to_string());
                bool theo = row[theo_col].to_string() == "THEO";
                lectures.emplace_back(name, ec, current_area, theo);
            }
        }
        return lectures;
    }

public:
    ProblemWrapper() {
        create_lectures();
        taken_lectures.reserve(taken_lecture_names.size());
        for (const auto &lecture : lectures) {
            if (std::find(taken_lecture_names.cbegin(), taken_lecture_names.cend(), lecture.name) !=
                taken_lecture_names.cend()) {
                taken_lectures.push_back(lecture);
            }
        }
        for (const auto &taken_lecture : taken_lectures) {
            lectures.erase(std::remove(lectures.begin(), lectures.end(), taken_lecture), lectures.end());
        }

        vector<Lecture> additional_taken_lectures = {
                Lecture("Seminar", 5, "None", false), Lecture("Practical Course", 10, "None", false), Lecture("IDP", 16, "None", false),
                Lecture("Guided Research", 10, "None", false), Lecture("Thesis", 30, "None", false),
                Lecture("Language", 6, "None", false)
        };
        taken_lectures.insert(taken_lectures.end(), additional_taken_lectures.begin(), additional_taken_lectures.end());


        existing_credits = std::accumulate(taken_lectures.cbegin(), taken_lectures.cend(), 0,
                                           [](int acc, const Lecture &l) { return acc + l.ec; });
        existing_theo_credits = std::accumulate(taken_lectures.cbegin(), taken_lectures.cend(), 0,
                                                [](int acc, const Lecture &l) { return (l.theo) ? acc + l.ec : acc; });

    };

    void define_Problem() {
        problem.add_variables(lectures, {0, 1});
        std::unique_ptr<Constraint<Lecture>> max_sum_constraint = std::make_unique<MaxSumConstraint<Lecture>>(2);
//        std::unique_ptr<Constraint<Lecture>> area_constraint = std::make_unique<AreaConstraint>();
        std::unique_ptr<Constraint<Lecture>> credit_constraint = std::make_unique<CreditConstraint>(existing_credits);
//        std::unique_ptr<Constraint> theo_constraint = std::make_unique<TheoConstraint>(lectures_ptr);
        problem.add_constraint(std::move(max_sum_constraint), lectures);
//        problem.add_constraint(std::move(area_constraint), lectures);
        problem.add_constraint(std::move(credit_constraint), lectures);
//        problem.add_constraint(std::move(theo_constraint), lectures);
//
    }

    void solve_problem() {
        auto solutions = problem.get_solutions();
        std::cout << solutions.size() << std::endl;
    }


};

int main() {
    // TODO create seperate repo for cpp_constraint
    auto problem_wrapper = ProblemWrapper();
    problem_wrapper.define_Problem();
    auto start = std::chrono::high_resolution_clock::now();
    problem_wrapper.solve_problem();
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    return 0;
    // TODO continue with implementing every functionality (mainly function constraints)
}
