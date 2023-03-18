#!/usr/bin/lua
package.loadlib('./cmake-build-release/src/librevelation-lua.so', 'luaopen_revelation')()

repo = UnitsRepository.new()
file = io.open('./resources/teams.txt')
content = file:read('*a')
repo:parse(content)
file:close()
gen = Generator.new()
function mkAgent(depth, i)
    heur = Heuristic.pxt()
    logger = ProgressLogger.noop()
    policy = StaticDFS.new(logger, heur)
    iter = policy:get()
    for j = 1,depth-1 do
	iter = iter:setOpponentsTurn(StaticDFS.new(logger, heur))
    end
    return Agent.search(i, policy, heur)
end

teams = { repo:getTeams()['Europe'], repo:getTeams()['Near East'] }

function playGame(i1, i2, seed)
	agents = { mkAgent(i1, 0), mkAgent(i2, 1) }
	io.write(string.format("seed %d\n", seed))
	result = Game.new(teams, agents, seed):play(2000)
	io.write(string.format('%i (%3.5f s) vs. %i (%3.5f s)\n', i1, result.agents[1].total_time, i2, result.agents[2].total_time, seed))
	if result.whoWon == 1 then io.write('---------------\n')
	elseif result.whoWon == 2 then io.write('                   --------------\n')
	else io.write('              -----\n')
	end
end

for i1 = 1,4 do
    for i2 = i1,4 do
        playGame(i1, i2, gen())
	playGame(i2, i1, gen())
    end
end
