from playwright.sync_api import sync_playwright
import os

def run():
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()

        # Navigate to the app
        print("Navigating to app...")
        page.goto("http://localhost:5173/Chess-Engine/")

        # Wait for app to load
        page.wait_for_selector(".app-container")
        print("App loaded.")

        # Click to expand PGN section
        print("Expanding PGN section...")
        # The section is collapsed by default. Text is "PGN / New Game"
        page.get_by_text("PGN / New Game").click()

        # Wait for textarea
        textarea = page.wait_for_selector("textarea")

        pgn = """[Event "Live Chess"]
[Site "Chess.com"]
[Date "2026.02.12"]
[Round "?"]
[White "avgrocketman"]
[Black "kinder921"]
[Result "0-1"]
[TimeControl "180+2"]
[WhiteElo "1238"]
[BlackElo "1265"]
[Termination "kinder921 won on time"]
[ECO "A10"]
[EndTime "20:44:17 GMT+0000"]
[Link "https://www.chess.com/game/live/164604640646"]

1. c4 d5 2. cxd5 Qxd5 3. Nc3 Qa5 4. Nf3 Nf6 5. g3 Bf5 6. Bg2 c6 7. O-O e6 8. d3
Bd6 9. Bd2 Qc7 10. e4 Bg4 11. h3 Bxf3 12. Qxf3 Nfd7 13. Rac1 Ne5 14. Qe2 Nbd7
15. f4 Qb6+ 16. Be3 Bc5 17. Bxc5 Qxc5+ 18. Kh1 Ng6 19. f5 Nge5 20. fxe6 fxe6 21.
Qh5+ Ng6 22. Qf3 Rf8 23. Qg4 Qd6 24. Rxf8+ Kxf8 25. Rf1+ Kg8 26. h4 Nde5 27. Qg5
Qd7 28. h5 Qe7 29. Qe3 Ng4 30. Qd4 Rd8 31. Qxa7 N6e5 32. d4 Nd3 33. d5 exd5 34.
exd5 Qc7 35. dxc6 Qxg3 0-1"""

        print("Filling PGN...")
        textarea.fill(pgn)

        # Click "Load PGN"
        print("Clicking Load PGN...")
        page.get_by_role("button", name="Load PGN").click()

        # Verify moves are loaded
        # Check if move "35." exists in the table
        print("Verifying moves loaded...")
        try:
            page.wait_for_selector(".move-table", timeout=5000)
            page.wait_for_selector("text=35.", timeout=5000)
            print("Moves loaded successfully!")
        except Exception as e:
            print("Failed to load moves or crashed:", e)
            page.screenshot(path="verification/crash.png")
            browser.close()
            return

        # Take screenshot of loaded game
        screenshot_path = "verification/pgn_loaded.png"
        page.screenshot(path=screenshot_path)
        print(f"Screenshot saved to {screenshot_path}")

        browser.close()

if __name__ == "__main__":
    run()
