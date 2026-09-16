# Game processor for Treasure Island (Tronica)
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class TreasureIslandGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Treasure Island
		self.game_key = "Treasure_Island_Tronica"  # Key from games_path.py
		self.game_folder = "trtreisl"  # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# This is a genuine "Super Tri-Screen" unit (3 separate LCDs side by
		# side: left/middle/right), matching its 3 Visual SVGs - lay them out
		# in one row like Life Boat's 2-screen side-by-side case.
		target.two_in_one_screen = True

		# The auto-generated entry only ever picked up BG-Right.png (the
		# .lay-based background/screen matcher - generate_games_path.py's
		# _extract_view_assets - failed to pair the left and middle <screen>
		# elements with their own background candidates for this specific
		# tri-screen .lay, unlike the simpler 1-2 screen layouts it was built
		# against), so only the rightmost screen ever showed real background
		# art. All three files exist on disk; wire them in explicitly, in the
		# same left/middle/right order as target.visual_paths.
		game_folder = self.base_dir
		target.background_paths = [
			game_folder / "BG-Left.png",
			game_folder / "BG-Center.png",
			game_folder / "BG-Right.png",
		]

		# Leave target.console_path at its auto-generated value (the full
		# trtreisl.png marketing photo) - a cropped button-area strip was
		# tried here, but "console" is also the menu-selection thumbnail
		# (see switch_menu.cpp), and cropping it broke that thumbnail. There
		# is no flat/front-on bezel image anywhere for this game to source a
		# proper strip from (not in our assets, not in MAME's own
		# trtreisl.zip artwork - just the 3 background PNGs, no bezel), so
		# for now this just keeps the working thumbnail over having a
		# strip - reverted per user direction.

		updater.write()
