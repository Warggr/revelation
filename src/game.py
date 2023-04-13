from player import Player
from team import Team
from state import State, Step
from constants import Timestep
from agent import Agent, MoveDecision, ActionDecision, ActionOrResource
from logger import Logger, FileLogger, PrintLogger, LiveServerAndLogger
from network import Server
import random
from time import time
from contextlib import nullcontext

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

        resources = nullcontext
        if isLiveServer:
            resources = Server()
            logger.addSubLogger(LiveServerAndLogger, resources)
        if logToTerminal:
            logger.addSubLogger(PrintLogger)

        with resources:
            with logger:
                while not self.state.isFinished():
                    if self.state.timestep == Timestep.BEGIN:
                        self.agents[self.state.iActive].onBegin( self.state )
                    (self.state, step) = self.state.advance(self.agents[self.state.iActive])
                    logger.addStep(step)
