"""

Ponto de entrada do InstagramStories.

Uso:
    python main.py                       # abre a interface
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from instagram_stories.gui import main  # noqa: E402

if __name__ == "__main__":
    main()
