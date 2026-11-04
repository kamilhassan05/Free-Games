# Make a game for Free Games

Want your own game on Free Games? Build it, send it to me, and I'll review it and add it with **your name on it**.

## How to send it

- **Instagram:** message me at **@YOUR_INSTAGRAM**
- **Email:** **YOUR_EMAIL**
- **GitHub:** open a pull request that adds your file to `source/FreeGamesAP/data/games/`

Tell me your name (or nickname) and, if you like, a link to show next to it.

## Rules

1. **One HTML file.** Put the CSS and JavaScript inside it. Keep it **under 60 KB**, because the board has limited space.
2. **Works offline.** No external scripts, images, fonts, ads, trackers or network requests. Players may have no internet.
3. **Works on a phone.** Touch controls, and it fits a small screen without sideways scrolling.
4. **Original work.** Don't use other companies' characters, names, music or art (no Mario, Pokemon and so on). Make your own.
5. **Safe for everyone.** No violence, adult content, hate or gambling.
6. **Credit.** Keep the credit footer from the template: "Made by YOUR NAME. Hosted on Free Games by Kamil Hassan".
7. **Permission.** By sending it you agree that I may host it on this project and on the ESP32 boards. You keep the credit as the author.
8. **Wrap storage in try/catch** if you save scores with `localStorage`.

## Start from the template

Copy `templates/game-template.html`, rename it, and change:

- `AUTHOR` at the top of the script (your name and optional link)
- the title and the game itself

It already has the Free Games look, a back button, Easy / Medium / Hard, sound with a mute button, and a saved best score.

## What happens next

1. I play it on a phone and read the code.
2. I may ask you to change things (for example, a bug or the size).
3. I add it to the site and the next firmware release, with your name on its card and inside the game.
