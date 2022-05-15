from player import Player
from team import Team
from state import State, Step
from agent import Agent, MoveDecision
from logger import Logger, LiveServerAndLogger

class Game:
    def __init__(self, teams : tuple[Team, Team], agents : tuple[Agent, Agent]):
        self.state = State.createStart(teams)
        self.teams = teams
        self.agents = agents
    def play(self, isLiveServer : bool = False) -> bool:
        logger = None
        if isLiveServer:
            logger = LiveServerAndLogger(self.teams)
        else:
            logger = Logger(self.teams)

        with logger:
            iActiveAgent = 0
            while not self.state.isFinished():
            #for _ in range(2):
                agent = self.agents[iActiveAgent]
                for iMovement in range(2):
                    move : MoveDecision = agent.getMovement(self.state)
                    step : Step = self.state.mkStep(move)
                    logger.addStep(step)
                    self.state = self.state.apply(step)

                iActiveAgent = 1 - iActiveAgent
