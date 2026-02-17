from playwright.sync_api import sync_playwright, expect
import time
import re
import os

PGN_CONTENT = """[Event "Live Chess"]
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

def run():
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        context = browser.new_context()
        page = context.new_page()

        print("Navigating to app...")
        app_url = os.environ.get("APP_URL", "http://localhost:5173/Chess-Engine/")
        page.goto(app_url)

        # Wait for app load
        page.wait_for_selector(".app-container")
        print("App loaded.")

        # Open PGN section
        print("Opening PGN input...")
        page.get_by_text("PGN / New Game").click()

        # Fill PGN
        print("Filling PGN...")
        page.fill("textarea", PGN_CONTENT)

        # Load PGN
        print("Loading PGN...")
        page.get_by_role("button", name="Load PGN").click()

        # Wait for moves to appear
        page.wait_for_selector(".move-table")
        print("Moves loaded.")

        # Set Analysis Mode to Low Reasoning (Fast)
        print("Setting Low Reasoning mode...")
        page.select_option("select", "low")

        # Start Analysis
        print("Starting Analysis...")
        page.get_by_role("button", name="Analyze Game").click()

        # Check Move 5 first
        print("Checking Move 5 (index 8)...")
        page.locator(".move-cell").nth(8).click()

        # Wait for depth on Move 5
        max_retries = 30
        success = False

        for i in range(max_retries):
            try:
                depth_text = page.locator(".stat-item", has_text="Depth").locator(".stat-value").inner_text(timeout=2000)
                print(f"Move 5 Depth: {depth_text}")
                if depth_text and depth_text.isdigit() and int(depth_text) >= 12:
                    success = True
                    break
            except Exception as e:
                print(f"Waiting for Move 5 analysis... ({e})")
            time.sleep(1)

        if not success:
            print("FAILURE: Analysis failed to complete for Move 5.")
            page.screenshot(path="verification/failure_move5.png")
            browser.close()
            return

        # Verify evaluation on Move 5
        eval_text = page.locator(".stat-item", has_text="Evaluation").locator(".stat-value").inner_text()
        print(f"Move 5 Evaluation: {eval_text}")

        # Check Move 15
        print("Checking Move 15 (index 28)...")
        page.locator(".move-cell").nth(28).click()

        success = False
        for i in range(max_retries):
            try:
                depth_text = page.locator(".stat-item", has_text="Depth").locator(".stat-value").inner_text(timeout=2000)
                print(f"Move 15 Depth: {depth_text}")
                if depth_text and depth_text.isdigit() and int(depth_text) >= 12:
                    success = True
                    break
            except Exception as e:
                print(f"Waiting for Move 15 analysis... ({e})")
            time.sleep(1)

        if not success:
            print("FAILURE: Analysis failed to complete for Move 15.")
            page.screenshot(path="verification/failure_move15.png")
            browser.close()
            return

        eval_text_15 = page.locator(".stat-item", has_text="Evaluation").locator(".stat-value").inner_text()
        print(f"Move 15 Evaluation: {eval_text_15}")

        # Check Move 35 (Last)
        print("Checking Move 35 (index 68)...") # 35th white move is index 68. 35... Qxg3 is index 69?
        # Let's use last cell again
        page.locator(".move-cell").last.click()

        success = False
        for i in range(max_retries):
            try:
                depth_text = page.locator(".stat-item", has_text="Depth").locator(".stat-value").inner_text(timeout=2000)
                print(f"Last Move Depth: {depth_text}")
                if depth_text and depth_text.isdigit() and int(depth_text) >= 12:
                    success = True
                    break
            except Exception as e:
                print(f"Waiting for Last Move analysis... ({e})")
            time.sleep(1)

        if not success:
            print("FAILURE: Analysis failed to complete for Last Move.")
            page.screenshot(path="verification/failure_lastmove.png")
            browser.close()
            return

        eval_text_last = page.locator(".stat-item", has_text="Evaluation").locator(".stat-value").inner_text()
        print(f"Last Move Evaluation: {eval_text_last}")

        # Compare evaluations
        if eval_text == eval_text_15 and eval_text == eval_text_last:
             print("FAILURE: Evaluations are identical across different moves! The bug is reproduced.")
             browser.close()
             return

        print("SUCCESS: Evaluations detected and changing across moves.")
        browser.close()

if __name__ == "__main__":
    run()
