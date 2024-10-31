# Hi-Lo Multi-process game implemented with parent/child processes, signals, and files
# The two children act as the players, while the parent is the referee of the game
# The referee generates a random number to be the target of the game which is from 1 to 100
# Player 1's strategy is to take the average of its max and min bounds which are originally the bounds of the target guess
# Player 2's strategy is to just guess a random number between its max and min bounds
# Each player's guess is written into their own private file for "secrecy"
# After each guess the referee informs the players whether their guesses were high, low, or correct and they adjust their bounds accordingly
# This feedback is done through signals and global flags, with each player having their own corresponding global flags and signal handlers
# A total of 10 games is played and the player with the most wins is declared the winner
