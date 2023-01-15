#!/usr/bin/python3
import argparse, sys

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import json as JSON

from sklearn.preprocessing import PolynomialFeatures
from sklearn.linear_model import LogisticRegression, LinearRegression
from sklearn.model_selection import train_test_split

HERO_STATS = ['maxHP', 'softAtk', 'hardAtk', 'mov', 'rng']

TRUE_UNITS = ['ar-kni','OP','fana','lordOf','canon','crbow','kni','sara','arabOf','capt','archer','baby']

def read_file(file, old_format):
    reading_games = False
    games = []
    heroes = []
    hero_names = {}
    for line in file:
        line = line.rstrip('\n')
        try:
            if line == '':
                reading_games = not reading_games
            elif not reading_games:
                name, json = line.split(':', 1)
                hero_names[name] = len(heroes)
                heroes.append(JSON.loads(json))
            else:
                if old_format:
                    header, teams = line.split(',', 1)
                    team1, team2, _ = teams.split(',END')
                else:
                    header, team1, team2 = line.split('[')
                winner = int(header[0]) - 1
                if winner == -1:
                    continue
                units = [0 for i in hero_names]
                for iteam, team in enumerate([team1, team2]):
                    if old_format:
                        team = team.strip(',').split(',')
                    else:
                        assert team[-1] == ']'
                        team = team[:-1]
                        team = team.strip(' ')
                        team = team.split()
                    for unit in team:
                        units[hero_names[unit]] += (1 if iteam == 0 else 1)
                games.append({ 'winner' : winner, 'units' : units })
        except Exception as err:
            print(err, '\n@', line)
            raise err

    # Inverting lines and columns
    games_inv = { slug: [ game['units'][i] for game in games ] for (i, slug) in enumerate(hero_names) }
    heroes_inv = { key: [ hero[key] for hero in heroes ] for key in HERO_STATS }

    X = pd.DataFrame(games_inv)
    Y = pd.Series([ game['winner'] for game in games ])

    hero_X = pd.DataFrame(heroes_inv)

    return (X, Y, hero_X, hero_names)

def fit(classifier, X, Y):
    X_train, X_test, Y_train, Y_test = train_test_split(X, Y, random_state=0)
    classifier.fit(X_train, Y_train)
    print('Accuracy: {:.2f}'.format(classifier.score(X_test, Y_test)))
    return classifier

def try_fit_stats(classifier, feature_names, X, Y):
    fit(classifier, X, Y)

    print('-----')

    print('Base:', classifier.intercept_)
    for i, stat in enumerate(feature_names):
        print(stat, ':', classifier.coef_[i])

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('input', nargs='?', type=argparse.FileType(), default=sys.stdin)
    p.add_argument('--old', action='store_true')
    p.set_defaults(old=False)
    args = p.parse_args()

    X, Y, hero_X, hero_names = read_file(args.input, args.old)

    classifier = LogisticRegression(solver='lbfgs',random_state=0)
    fit(classifier, X, Y)

    print('-----')

    hero_Y = []
    old_0 = classifier.coef_[0][ hero_names['capt'] ]
    new_0 = 288.7
    old_1 = classifier.coef_[0][ hero_names['kni'] ]
    new_1 = 520.5
    rescaling =  (new_1 - new_0) / (old_1 - old_0)
    for (name, i) in hero_names.items():
        coef = classifier.coef_[0][i]
        coef = (coef - old_0) * rescaling + new_0
        hero_Y.append(coef)
        if name in TRUE_UNITS:
            print(name, ':', coef)
    hero_Y = pd.Series(hero_Y)

    print('-----')

    classifier = LinearRegression(fit_intercept=False)
    try_fit_stats(classifier, HERO_STATS, hero_X, hero_Y)

    print('-----')

    classifier = LinearRegression()
    try_fit_stats(classifier, HERO_STATS, hero_X, hero_Y)

    print('-----')

    features = HERO_STATS
    hero_X['maxHP ^2'] = [ hero_X['maxHP'][i]**2 for i in range(len(hero_X)) ]
    features.append('maxHP ^2')
    hero_X['softAtk ^2'] = [ hero_X['softAtk'][i]**2 for i in range(len(hero_X)) ]
    features.append('softAtk ^2')
    hero_X['softAtk x maxHP'] = [ hero_X['softAtk'][i]*hero_X['maxHP'][i] for i in range(len(hero_X)) ]
    features.append('maxHP * softAtk')
    hero_X['hardAtk ^2'] = [ hero_X['hardAtk'][i]**2 for i in range(len(hero_X)) ]
    features.append('hardAtk ^2')
    hero_X['hardAtk x maxHP'] = [ hero_X['hardAtk'][i]*hero_X['maxHP'][i] for i in range(len(hero_X)) ]
    features.append('maxHP * hardAtk')
    try_fit_stats(classifier, features, hero_X, hero_Y)
