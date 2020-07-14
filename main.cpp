#include <iostream>
#include <vector>
#include <string>
#include "helpers/lecture.hpp"
#include "xlnt/xlnt.hpp"
#include <chrono>  // for high_resolution_clock

class ProblemWrapper {

    static constexpr int credit_limit = 120;
    static constexpr int theo_limit = 10;
    static constexpr int highest_ec_limit = 18;
    static constexpr int normal_ec_limit = 8;
    std::vector<int> area_limit = {highest_ec_limit, normal_ec_limit, normal_ec_limit};
    static constexpr int max_allowed_lectures = 8;
    // TODO should be a set
    std::vector<std::string> taken_lecture_names = {"Computer Vision I: Variational Methods",
                                                    "Computer Vision II: Multiple View Geometry",
                                                    "Natural Language Processing", "Introduction to Deep Learning",
                                                    "Advanced Deep Learning for Computer Vision"};

    std::vector<std::string> preferred_areas = {"COMPUTER GRAPHICS AND VISION", "MACHINE LEARNING AND ANALYTICS",
                                                "DIGITAL BIOLOGY AND DIGITAL MEDICINE"};

    static auto create_lectures() {
        xlnt::workbook wb;
        wb.load("tum_lectures.xlsx");
        auto ws = wb.active_sheet();
        std::string current_area = "none";
        std::vector<Lecture> lectures;
        for (auto row : ws.rows(false)) {
            auto first_column = row[0].to_string();
            if (first_column.find("ELECTIVE MODULES OF THE AREA") != std::string::npos) {
                auto begin_quote_idx = first_column.find('\"');
                auto end_quote_idx = first_column.find('\"', begin_quote_idx + 1);
                current_area = first_column.substr(begin_quote_idx + 1, end_quote_idx - begin_quote_idx - 1);
            }
            if (first_column.substr(0, 2) == "IN") {
                std::string name = row[2].to_string();
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
        auto lectures = create_lectures();
        std::vector<Lecture> taken_lectures;
        for (const auto &lecture : lectures) {
            if (std::find(taken_lecture_names.cbegin(), taken_lecture_names.cend(), lecture.name) !=
                taken_lecture_names.cend()) {
                taken_lectures.push_back(lecture);
            }
        }
        for (const auto &taken_lecture : taken_lectures) {
            lectures.erase(std::remove(lectures.begin(), lectures.end(), taken_lecture), lectures.end());
        }

        std::vector<Lecture> additional_taken_lectures = {
                Lecture("Seminar", 5, "None", false), Lecture("Practical Course", 10, "None", false), Lecture("IDP", 16, "None", false),
                Lecture("Guided Research", 10, "None", false), Lecture("Thesis", 30, "None", false),
                Lecture("Language", 6, "None", false)
        };
        taken_lectures.insert(taken_lectures.end(), additional_taken_lectures.begin(), additional_taken_lectures.end());
        auto a = 1;
        // TODO taken lectures vector
        // TODO delete lectures
        // TODO start working on constraint library
    };


};

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    xlnt::workbook wb;
    auto problem_wrapper = ProblemWrapper();
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    return 0;
}
