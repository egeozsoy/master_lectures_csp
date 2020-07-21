//
// Created by Ege on 12.07.20.
//
#include <iostream>
#include <utility>

#ifndef MASTER_CSP_CPP_LECTURE_HPP
#define MASTER_CSP_CPP_LECTURE_HPP

struct Lecture {
    std::string name;
    int ec;
    std::string area;
    bool theo;
    double grade;
public:
    Lecture(std::string _name, int _ec, std::string _area, bool _theo, double _grade = -1) :
            name(_name), ec(_ec), area(_area), theo(_theo), grade(_grade) {}

    explicit Lecture() = default;
    bool operator==(const Lecture &other) const {
        return this->name == other.name;
    }

    bool operator!=(const Lecture &other) const {
        return this->name != other.name;
    }

    bool operator<(const Lecture &other) const {
        return this->name < other.name;
    }
};

#endif //MASTER_CSP_CPP_LECTURE_HPP
