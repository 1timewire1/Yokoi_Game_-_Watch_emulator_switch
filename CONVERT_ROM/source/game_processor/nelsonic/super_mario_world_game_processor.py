# Game processor for Super Mario World (Nelsonic)
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class SuperMarioWorldGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		self.game_key = "Super_Mario_World_Nelsonic" # Key from games_path.py
		self.game_folder = "nsmw" # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# See StarFoxGameProcessor's comment - same photographic-background
		# problem as the other two Nelsonic games.
		target.alpha_bright = 0.8
		target.fond_bright = 1.0

		updater.write()
