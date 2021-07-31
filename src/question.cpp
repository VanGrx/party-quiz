
#include <random>
#include <algorithm>
#include <iostream>

#include "question.h"

Question::Question()
{

}


Question::Question(const std::string& q, const std::string& a1, const std::string& a2, const std::string& a3, const std::string& correctA):question{q}
{
    answers.emplace_back(a1);
    answers.emplace_back(a2);
    answers.emplace_back(a3);
    answers.emplace_back(correctA);

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(answers.begin(), answers.end(), g);

    for(size_t i=0;i<answers.size();i++)
        if(answers[i] == correctA)
        {
            correctAnswerIndex = i;
            break;
        }
}




void Question::print(){


    std::cout<<question<<std::endl;
    std::cout<<"Answers:"<<std::endl;
    for(size_t i=0; i< answers.size();i++)
        std::cout<<i<<" "<<answers[i]<<std::endl;

    std::cout<<"Correct answer: "<<answers[correctAnswerIndex]<<std::endl;

    std::cout<<"########################################################"<<std::endl;


}
