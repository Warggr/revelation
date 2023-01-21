# Core concepts

This folder implements `SearchAgent`, an implementation of `class Agent`
which takes decisions automatically and intelligently. `SearchAgent`'s are supposed to always pick the best move.

This can be considered as a search problem: we have a lot of possible futures (depending on which decision we take),
which can be represented as tree of possibilities.
We need to search for the best move from that tree.

A SearchAgent can have different parameters and implementations:

- The search algorithm can vary; both [best-first search](bestfirstsearch.hpp) and [depth-first search](depthfirstsearch.hpp) are implemented.
- Depth-limited search can be used and the search depth can be varied.
- The heuristic according to which states are evaluated can be changed. Heuristics are available in [heuristic.hpp](heuristic.hpp).

Also, [loggers.hpp](loggers.hpp) contains loggers which can be used to see the progress of a search
(e.g. display a progress bar while the computer is thinking).
