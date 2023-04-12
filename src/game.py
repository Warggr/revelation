from player import Player
from team import Team
from state import State, Step
from constants import Timestep
from agent import Agent, MoveDecision, ActionDecision, ActionOrResource
from logger import Logger, FileLogger, PrintLogger, LiveServerAndLogger
import random
from time import time

class Game:
    def __init__(self, teams, agents, seed=None):
        self.state = State.createStart(teams)
        self.teamNames = [ team.name for team in teams ]
        self.agents = agents

        if seed is None:
            seed = int(time() * 256)
            print('Using', seed, 'as master seed')
        else:
            print('Reusing seed', seed)
        random.seed(seed)

    def serialize(self):
        return { "teamNames" : self.teamNames }
    def play(self, isLiveServer : bool = False, logToTerminal : bool = False) -> bool:
        logger = Logger(self.state, self)
        logger.addSubLogger(FileLogger)
        if isLiveServer:
            logger.addSubLogger(LiveServerAndLogger)
        if logToTerminal:
            logger.addSubLogger(PrintLogger)

        with logger:
            while not self.state.isFinished():
                if self.state.timestep == Timestep.BEGIN:
                    self.agents[self.state.iActive].onBegin( self.state )
                (self.state, step) = self.state.advance(self.agents[self.state.iActive])
                logger.addStep(step)
            print('Finished! Exiting logger...')
        print('Finished, logger exited!')
