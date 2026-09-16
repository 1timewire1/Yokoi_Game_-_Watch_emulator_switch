# Game processor for Bassmate Computer
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class BassmateGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Bassmate Computer
		self.game_key = "Bassmate_Computer_Telko_Nintendo"  # Key from games_path.py
		self.game_folder = "bassmate"  # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# Bassmate's "on" segments are large filled rectangles sitting
		# directly over printed colour/label art (its option grid), unlike
		# the thin digit strokes most games use. Rendering them with plain
		# opaque alpha blending erases the label and colour underneath
		# instead of darkening it in place, which is how the real device
		# (and MAME's own artwork compositing) looks. multiply_segments
		# switches the Switch renderer to a multiply blend for this game's
		# "on" segments - see switch_screen.cpp's multiply_darken.
		target.multiply_segments = True

		# MAME's own default.lay confirms the real compositing order per
		# screen: Screen-Top(NS) (background) -> <screen blend="multiply">
		# (the live LCD segments) -> TEXT-Top drawn LAST, on top of
		# everything, at a fixed alpha of 0.7 - so the option-grid labels
		# stay legible even over a darkened/"on" cell, instead of getting
		# baked into the static background where a lit segment would erase
		# them. Background must be the blank (no-label) NS art; the labels
		# themselves belong in Overlay, drawn after segments - see
		# convert_switch.py's overlay_data_file / switch_screen.cpp's
		# multiply_darken overlay pass.
		game_folder = self.base_dir
		target.background_paths = [
			game_folder / "Screen-TopNS.png",
			game_folder / "Screen-BottomNS.png",
		]
		target.overlay_paths = [
			game_folder / "TEXT-Top.png",
			game_folder / "TEXT-Bottom.png",
		]

		# background_data_file()'s alpha computation (img_manipulation.py's
		# make_alpha) is tuned for ink-on-white line art: it brightens a
		# copy of the image by alpha_bright, then treats "all channels
		# above seuil_tr(170)" as transparent (paper) and "darkest channel
		# below seuil_semi_tr(100)" as opaque (ink), fading everything
		# between. Bassmate's background is a flat, fully-opaque printed
		# colour card with no ink/paper distinction at all - at the
		# default alpha_bright=1.7, its pink cells (raw ~RGB(188,112,135))
		# brighten past 170 on every channel and read as fully
		# transparent, and its blue cells fade to ~17% opacity, letting
		# the base fill bleed through and muting every colour. A low
		# alpha_bright keeps every real pixel's brightened minimum channel
		# under seuil_semi_tr so it's classified opaque and keeps its own
		# (already-opaque) alpha untouched; fond_bright=1.0 reproduces the
		# scanned colours as-is rather than the default's unrelated 35%
		# boost, which was never calibrated for this art in the first
		# place.
		target.alpha_bright = 0.5
		target.fond_bright = 1.0

		updater.write()
