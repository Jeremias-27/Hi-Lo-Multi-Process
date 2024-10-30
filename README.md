# Hi-Lo Multi-process game implemented with parent/child processes, signals, and files.
# The two children act as the players, while the parent is the referee of the game
# A total of 10 games will be played and the player with the most wins is declared the winner.
# Files are used to store each player's guesses "secretly" and signals and global flags are used to synchronize the two children with the parent
