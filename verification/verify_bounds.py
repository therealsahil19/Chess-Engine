from playwright.sync_api import Page, expect, sync_playwright

def test_bounds_display(page: Page):
    print("Navigating to app...")
    page.goto("http://localhost:5173/Chess-Engine/")

    print("Waiting for board...")
    expect(page.locator(".board-container")).to_be_visible()

    print("Opening PGN section...")
    page.get_by_text("PGN / New Game").click()

    print("Entering PGN...")
    page.get_by_placeholder("Paste PGN here...").fill("1. e4")

    print("Loading PGN...")
    page.get_by_role("button", name="Load PGN").click()

    # Note: handlePgnLoad resets to start position (index -1).
    # We are analyzing the start position (White to move).
    # We forced 'lower' bound.
    # White to move, lower bound -> White >= val.
    # Expect '≥'.

    print("Starting analysis...")
    page.get_by_role("button", name="Analyze Game").click()

    print("Waiting for stats panel...")
    expect(page.locator(".stats-panel")).to_be_visible(timeout=10000)

    print("Checking for bound symbol...")
    expect(page.locator(".stat-value").first).to_contain_text("≥")

    print("Taking screenshot...")
    page.screenshot(path="/home/jules/verification/verification.png")
    print("Done.")

if __name__ == "__main__":
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()
        try:
            test_bounds_display(page)
        except Exception as e:
            print(f"Test failed: {e}")
            page.screenshot(path="/home/jules/verification/failure.png")
            raise e
        finally:
            browser.close()
