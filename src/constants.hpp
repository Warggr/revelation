constexpr int MAX_RESOURCES = 3;
constexpr int MAX_ACTIONS = 4;
constexpr int MAX_ABILITIES = 4;

constexpr int ARMY_WIDTH = 3;
constexpr int HALF_BOARD_WIDTH = ARMY_WIDTH + 2;
constexpr int FULL_BOARD_WIDTH = 2*HALF_BOARD_WIDTH;
constexpr int NB_CHARACTERS = 2*ARMY_WIDTH;

enum Faction {
    NONE, BLOOD, MERCURY, HORROR, SPECTRUM, ETHER
};

enum Timestep {
    BEGIN, DREW, DISCARDED, MOVEDfirst, MOVEDlast, ABILITYCHOSEN, ACTED
};

enum AtkType {
    HARD, SOFT
};
