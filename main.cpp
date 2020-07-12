#include <iostream>
#include <vector>
#include <string>
#include "helpers/lecture.hpp"
#include "xlnt/xlnt.hpp"

class ProblemWrapper {
    int credit_limit = 120;
    int theo_limit = 10;
    std::vector<int> area_limit = {18, 8, 8};
    int max_allowed_lectures = 8;
    std::vector<std::string> taken_lecture_names = {"Computer Vision I: Variational Methods",
                                                    "Computer Vision II: Multiple View Geometry",
                                                    "Natural Language Processing", "Introduction to Deep Learning",
                                                    "Advanced Deep Learning for Computer Vision"};

    std::vector<std::string> preferred_areas = {"COMPUTER GRAPHICS AND VISION", "MACHINE LEARNING AND ANALYTICS",
                                                "DIGITAL BIOLOGY AND DIGITAL MEDICINE"};

    Lecture lecture = Lecture("Computer Vision I: Variational Methods", 8, "COMPUTER GRAPHICS AND VISION",
                              true, 3.0);

    int a = 1;
    int b = 2;
    // TODO constraint library
//    Problem problem = constraint.Problem();
//    std::vector<Lecture> lectures = create_lectures()

    void create_lectures() {

    }
};

int main() {
//    std::cout << "Hello, World!" << std::endl;
    xlnt::workbook wb;
    auto problem_wrapper = ProblemWrapper();
    return 0;
}
