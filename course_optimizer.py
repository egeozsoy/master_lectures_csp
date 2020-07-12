'''
Constraint Satisfaction Problem to find best lectures to complete
Needs to be modified to suit a certain need. Is more of a template now
'''

from collections import defaultdict

import constraint  # http://labix.org/doc/constraint/
import pandas as pd

from helpers.lecture import Lecture


class ProblemWrapper:
    def __init__(self):
        self.credit_limit = 120
        self.theo_limit = 10
        self.area_limit = [18, 8, 8]
        self.max_allowed_lectures = 8  # In Master it is unrealistic to do more than N lectures (except thesis practical courses seminars etc.)
        self.taken_lecture_names = ['Computer Vision I: Variational Methods', 'Computer Vision II: Multiple View Geometry', 'Natural Language Processing',
                                    'Introduction to Deep Learning', 'Advanced Deep Learning for Computer Vision']

        self.preferred_areas = ['COMPUTER GRAPHICS AND VISION', 'MACHINE LEARNING AND ANALYTICS', 'DIGITAL BIOLOGY AND DIGITAL MEDICINE']

        self.problem = constraint.Problem()
        self.lectures = self.create_lectures()
        self.taken_lectures = []
        for lecture in self.lectures:
            if lecture.name in self.taken_lecture_names:
                self.taken_lectures.append(lecture)
        for taken_lecture in self.taken_lectures:
            self.lectures.remove(taken_lecture)

        self.taken_lectures.extend(  # Not classical lectures
            [Lecture(name='Seminar', ec=5, area='None'), Lecture(name='Practical Course', ec=10, area='None'), Lecture(name='IDP', ec=16, area='None'),
             Lecture(name='Guided Research', ec=10, area='None'), Lecture(name='Thesis', ec=30, area='None'), Lecture(name='Language', ec=6, area='None')])

        self.exitings_credits = 0
        self.existing_theo_credits = 0
        for taken_lecture in self.taken_lectures:
            self.exitings_credits += taken_lecture.ec
            if taken_lecture.theo:
                self.existing_theo_credits += taken_lecture.ec
        self.problem.addVariables(self.lectures, [0, 1])
        self.problem.addConstraint(constraint.MaxSumConstraint(self.max_allowed_lectures - len(self.taken_lecture_names)), self.lectures)
        self.problem.addConstraint(constraint.FunctionConstraint(self.pruning_constraint), self.lectures)
        self.problem.addConstraint(constraint.FunctionConstraint(self.area_constraint), self.lectures)
        self.problem.addConstraint(constraint.FunctionConstraint(self.credit_constraint), self.lectures)
        self.problem.addConstraint(constraint.FunctionConstraint(self.theo_constraint), self.lectures)

        solutions = self.problem.getSolutions()
        solutions = self.solution_sorting(solutions)
        print(len(solutions))
        for i in range(100):
            print(self.get_credits_for_solution(solutions[i]))
            print(self.solution_to_str(solutions[i]))

    def create_lectures(self):
        excel = pd.read_excel('tum_lectures.xlsx')
        current_area = None
        lectures = []
        for idx, row in excel.iterrows():
            if not isinstance(row[0], str):
                continue
            if 'ELECTIVE MODULES OF THE AREA' in row[0]:
                current_area = row[0].split('ELECTIVE MODULES OF THE AREA')[-1].split('"')[1]

            if row['ID'][:2] == 'IN':
                lectures.append(Lecture(name=row['Title'], ec=int(row['Credits']), area=current_area, theo=row['THEO'] == 'THEO'))
        return lectures

    def pruning_constraint(self, *bools):
        total_credits = self.exitings_credits
        for idx in range(len(bools)):
            if bools[idx]:
                total_credits += self.lectures[idx].ec
        to_many_credits = total_credits > 60  # As the rest of the credits are guaranteed from thesis etc. Never a need for this many lecture credits
        if to_many_credits:
            return False

        return True

    def area_constraint(self, *bools):
        areas = defaultdict(int)
        for idx in range(len(bools)):
            if bools[idx]:
                areas[self.lectures[idx].area] += self.lectures[idx].ec
        for taken_lecture in self.taken_lectures:
            areas[taken_lecture.area] += taken_lecture.ec
        areas.pop('None', None)
        values = areas.values() if self.preferred_areas is None else [value for key, value in areas.items() if key in self.preferred_areas]
        if len(values) < 3:
            return False
        sorted_areas = sorted(values, reverse=True)
        if sorted_areas[0] >= 18 and sorted_areas[1] >= 8 and sorted_areas[2] >= 8:
            return True
        else:
            return False

    def credit_constraint(self, *bools):
        total_sum = self.exitings_credits
        for idx in range(len(bools)):
            if bools[idx]:
                total_sum += self.lectures[idx].ec

        return total_sum >= self.credit_limit

    def theo_constraint(self, *bools):
        theo_credits = self.existing_theo_credits
        for idx in range(len(bools)):
            if bools[idx] and self.lectures[idx].theo:
                theo_credits += self.lectures[idx].ec

        return theo_credits >= self.theo_limit

    def get_credits_for_solution(self, solution):
        total_ec = 0
        for key, value in solution.items():
            if value == 0:
                continue

            total_ec += key.ec

        return total_ec

    def solution_sorting(self, solutions):
        ecs = []
        for solution in solutions:
            ecs.append(self.get_credits_for_solution(solution))

        solutions = [x for _, x in sorted(zip(ecs, solutions), key=lambda pair: pair[0])]

        return solutions

    def solution_to_str(self, solution):
        out = ''
        for key, value in solution.items():
            if value:
                out += f'{key}\n'

        return out


if __name__ == '__main__':
    problem_wrapper = ProblemWrapper()
