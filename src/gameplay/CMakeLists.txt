add_library(gameplay STATIC
	character.cpp
	deck.cpp
	player.cpp
	state.cpp
)
target_link_libraries(gameplay PRIVATE logging)
