package.loadlib('./cmake-build-release/src/librevelation-lua.so', 'luaopen_revelation')()

repo = UnitsRepository.new()
file = io.open('./resources/teams.txt')
content = file:read('*a')
repo:parse(content)
file:close()
agents = { nil, nil }
gen = Generator.new()
for i = 1,2 do
    heur = Heuristic.pxt()
    logger = ProgressLogger.noop()
    policy = StaticDFS.new(logger, heur)
    -- policy:get():setOpponentsTurn( StaticDFS.new(logger, heur) )
    agents[i] = Agent.search(i, policy, heur)
end

seed = gen()

creator = RandomUnitCreator.new(repo)
for i=1,390 do creator:getRandom(gen) end

for key, crea in pairs(repo:getCharacters()) do
    io.write(string.format('%7s:%s\n', crea.slug, crea))
end
print('\n')

NB_GAMES=10000
MAX_STEPS=5000
teams = { nil, nil }
for i = 1,NB_GAMES do
    io.stderr:write(string.format('%d/%d\r', i, NB_GAMES))
    for i = 1,2 do teams[i] = repo:mkRandomTeam(gen, repo, 3) end
    result = Game.new(teams, agents, gen()):play(MAX_STEPS)
    io.stderr:write('\r')
    io.write( string.format("%d (%4d)  ", result.whoWon, result.nbSteps) )
    for key, team in pairs(teams) do
	io.write("[ ")
	for key, c in pairs(team:getUnits()) do
	    io.write(string.format("%7s ", c.slug))
        end
	io.write("]")
    end
    io.write('\n')
end
