from player import Player
from team import Team
from state import State, Step
from constants import Timestep
from agent import Agent, MoveDecision, ActionDecision, ActionOrResource
from logger import BaseLogger

class Game:
    def __init__(self, teams : tuple[Team, Team], agents : tuple[Agent, Agent]):
        self.state = State.createStart(teams)
        self.teams = teams
        self.agents = agents
    def play(self, isLiveServer : bool = False, logToTerminal : bool = False) -> bool:
        logger = BaseLogger(self.teams)
        if isLiveServer:
            logger = logger.liveServer()
        if logToTerminal:
            logger = logger.logToTerminal()

        with logger:
            while not self.state.isFinished():
                if self.state.timestep == Timestep.BEGIN:
                    self.agents[self.state.iActive].onBegin( self.state )
                (self.state, step) = self.state.advance(self.agents[self.state.iActive])
                logger.addStep(step)
