# Game processor for Star Fox (Nelsonic)
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class StarFoxGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		self.game_key = "Star_Fox_Nelsonic" # Key from games_path.py
		self.game_folder = "nstarfox" # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# The default alpha_bright/fond_bright pair (tuned for ink-on-white
		# line art) is far too aggressive on this game's photographic
		# background scan: most of the sky/ground color gets classified as
		# "background" and wiped to transparent, leaving only dark speckled
		# outlines. Same fix as the Elektronika batch's photographic scans.
		target.alpha_bright = 0.8
		target.fond_bright = 1.0

		updater.write()
