#!/usr/bin/python

from game import Game
from team import Team
from agent import SearchAgent
from character import Character
from constants import ARMY_WIDTH

armies = [
    [
        "Near East", [
            ["Mounted archers"    , 60, 30, 10, 3, 3, 534.90, "Ranged attack (Arc 3)" ],
            ["Captives"           , 20, 20, 10, 2, 1, 288.70 ],
            ["Captives"           , 20, 20, 10, 2, 1, 288.70 ],
            ["Saracens"           , 80, 50, 20, 2, 1, 454.90, "Defense 20(light)" ],
            ["Canons"             , 60, 70, 0, 1, 4, 551.60, "Ranged attack(Straight 4)" ],
            ["Arab officer"       , 100, 10, 10, 2, 1, 355.30 ]
        ]
    ], [
        "Europe", [
            ["Crossbowman"        , 40, 40, 20, 2, 3, 489.10, "Ranged attack(straight 3)" ],
            ["Armored knight"     , 100, 60, 30, 1, 1, 449.90, "Defense 20(light)" ],
            ["Fanatics"           , 20, 20, 10, 2, 1, 288.70 ],
            ["Fanatics"           , 20, 20, 10, 2, 1, 288.70 ],
            ["Fanatics"           , 20, 20, 10, 2, 1, 288.70 ],
            ["Knight"             , 90, 50, 10, 3, 1, 520.50, "Defense 20(light)" ],
            ["Lord officer"       , 100, 10, 10, 2, 1, 355.30 ]
        ]
    ], [
        "Far East", [
            ["Powder bombs"       , 50, 30, 0, 2, 2, 386.80, "Ranged attack (Arc 2) Chooses 2 targets" ],
            ["Ji Warriors"        , 40, 30, 10, 2, 1, 339.10 ],
            ["Ji Warriors"        , 40, 30, 10, 2, 1, 339.10 ],
            ["Ji Warriors"        , 40, 30, 10, 2, 1, 339.10 ],
            ["Kyudoka"            , 80, 30, 20, 2, 3, 499.90, "Ranged attack (Arc 3) " ],
            ["Ronin"              , 80, 50, 50, 2, 1, 499.90 ],
            ["Jutsu"              , 100, 10, 10, 2, 1, 355.30 ]
        ]
    ], [
        "Northmen", [
            ["Slingers"           , 50, 30, 10, 2, 2, 401.80, "Ranged attack(straight 2)" ],
            ["Reavers"            , 50, 30, 10, 2, 1, 349.30 ],
            ["Reavers"            , 50, 30, 10, 2, 1, 349.30 ],
            ["Reavers"            , 50, 30, 10, 2, 1, 349.30 ],
            ["Warriors with maces", 80, 50, 30, 2, 1, 469.90 ],
            ["Berserk"            , 100, 30, 20, 2, 1, 415.30, "+10 to the damage for each 20 HP lost" ],
            ["Northmen officer"   , 100, 10, 10, 2, 1, 355.30 ]
        ]
    ], [
        "Slow Near East", [
            ["Mounted archers"    , 60, 30, 10, 1, 3, 534.90, "Ranged attack (Arc 3)" ],
            ["Captives"           , 20, 20, 10, 1, 1, 288.70 ],
            ["Captives"           , 20, 20, 10, 1, 1, 288.70 ],
            ["Saracens"           , 80, 50, 20, 1, 1, 454.90, "Defense 20(light)" ],
            ["Canons"             , 60, 70, 0, 0, 4, 551.60, "Ranged attack(Straight 4)" ],
            ["Arab officer"       , 100, 10, 10, 1, 1, 355.30 ]
        ]
    ], [
        "Slow Europe", [
            ["Crossbowman"        , 40, 40, 20, 1, 3, 489.10, "Ranged attack(straight 3)" ],
            ["Armored knight"     , 100, 60, 30, 1, 1, 449.90, "Defense 20(light)" ],
            ["Fanatics"           , 20, 20, 10, 1, 1, 288.70 ],
            ["Fanatics"           , 20, 20, 10, 1, 1, 288.70 ],
            ["Fanatics"           , 20, 20, 10, 1, 1, 288.70 ],
            ["Knight"             , 90, 50, 10, 0, 1, 520.50, "Defense 20(light)" ],
            ["Lord officer"       , 100, 10, 10, 1, 1, 355.30 ]
        ]
    ]
]

teams = { 
    army[0] : Team(
        army[0],
        [
            [
                Character(i, *(army[1][row*ARMY_WIDTH + i])) 
                for i in range(ARMY_WIDTH)
            ]
            for row in range(2)
        ]
    ) 
    for army in armies 
}

agents = ( SearchAgent(0), SearchAgent(1) )

game = Game( (teams['Near East'], teams['Europe']), agents )

if __name__ == '__main__':
    game.play(isLiveServer=True, logToTerminal=True)
