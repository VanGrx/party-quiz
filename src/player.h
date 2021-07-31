#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player
{
public:
    Player();

    Player(int _id, std::string _username):id{_id},username{_username},score{0}{};

    std::string getUsername(){
        return username;
    }

    unsigned int getScore(){
        return score;
    }

private:
    int id;
    std::string username;
    unsigned int score;
};

#endif // PLAYER_H
