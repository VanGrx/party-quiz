#ifndef QUESTION_H
#define QUESTION_H

#include <string>
#include <vector>

class Question {
public:
  Question();

  Question(const std::string &q, const std::string &a1, const std::string &a2,
           const std::string &a3, const std::string &correctA);

  void print();

private:
  std::string question;
  std::vector<std::string> answers;
  int correctAnswerIndex;
};

#endif // QUESTION_H
