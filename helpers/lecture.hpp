//
// Created by Ege on 12.07.20.
//
#include <iostream>
#include <utility>

#ifndef MASTER_CSP_CPP_LECTURE_HPP
#define MASTER_CSP_CPP_LECTURE_HPP

class Lecture {
    const std::string name;
    int ec;
    const std::string area;
    bool theo;
    double grade;
public:
    Lecture(std::string _name, int _ec, std::string _area, bool _theo, double _grade = -1) :
            name(std::move(_name)), ec(_ec), area(std::move(_area)), theo(_theo), grade(_grade) {}
};

#endif //MASTER_CSP_CPP_LECTURE_HPP
