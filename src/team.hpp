#ifndef REVELATION_TEAM_H
#define REVELATION_TEAM_H

#include "character.hpp"

class Team {
	std::string name;
	Character characters[6];

public:
	Team(const std::string& name, Character characters[6]) : name(name), characters(characters) {}
	/*def serialize(self):
		return { 'name' : self.name, 'characters' : self.characters }*/
};

#endif //REVELATION_TEAM_H
