# Project structure and core concepts

## include

The `include` folder contains useful headers and simple `struct`'s and `typedef`'s.

## setup

This folder defines how to set up a game, e.g. how to read cards descriptions from a file.

## gameplay

The `gameplay` folders contains the implementation of the game rules. The game progresses as follows:

- The game is in a certain state.
- A player takes a decision.
- As a result of this decision, the game is a new, different state.

(more details in [gameplay/README.md](gameplay/README.md))

## control

`class Agent` from [control/agent.hpp](control/agent.hpp) represents anything that could take decisions;
for example a human player, a computer playing random moves, or an AI/bot playing actually though-out moves.
The `control` folder defines Agents and implements the following high-level game loop:

- ask a player to choose an action
- execute that action
- repeat until the game is over

## logging

This folder defines how game actions can be serialized and logged to a console.

## search

This folder implements AIs/bots that can play the game. More details in [search/README.md](search/README.md).

## network

This folder contains the code for both a simple and a complex server.
The simple server just broadcasts a game to clients.
The complex server allows clients to connect to it and launch games remotely, and then broadcasts those games to them.

# Entrypoints

There are three entrypoints to the program:

- [main.cpp](main.cpp) demonstrates how to create bots and human players, and launch a game with both.
- [main.server.cpp](main.server.cpp) contains a `main` function that launches a (complex) server.
The server creates games on the request of clients.
- [lua.module.cpp](lua.module.cpp) is compiled as a library (it does not have a `main` function)
and exports most game concepts as a Lua library. This allows to write scripts in the [Lua language](https://www.lua.org).
