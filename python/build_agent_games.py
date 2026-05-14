from pathlib import Path

import pandas as pd


PROJECT_ROOT = Path(__file__).resolve().parents[1]

MAIN_STATES_PATH = PROJECT_ROOT / "data" / "states.csv"
RANDOM_EXTRA_PATH = PROJECT_ROOT / "data" / "ra_states.csv"
OUTPUT_PATH = PROJECT_ROOT / "artifacts" / "agent_games.csv"

CELL_COLUMNS = [f"c{i}" for i in range(16)]


def assign_agent(game_id: int) -> str:
    if 0 <= game_id < 1000:
        return "random"
    if 1000 <= game_id < 4000:
        return "heuristic"
    if 4000 <= game_id < 7000:
        return "expectimax"
    if 10000 <= game_id < 12000:
        return "random"

    return "unused"


def load_and_merge_states() -> pd.DataFrame:
    main_df = pd.read_csv(MAIN_STATES_PATH)
    random_extra_df = pd.read_csv(RANDOM_EXTRA_PATH)

    return pd.concat([main_df, random_extra_df], ignore_index=True)


def build_agent_games(states: pd.DataFrame) -> pd.DataFrame:
    states = states.copy()

    states["final_score"] = states["score"] + states["target"]
    states["row_max_tile"] = states[CELL_COLUMNS].max(axis=1)
    states["agent"] = states["game_id"].apply(assign_agent)

    states = states[states["agent"] != "unused"]

    games = (
        states.groupby(["agent", "game_id"])
        .agg(
            final_score=("final_score", "first"),
            max_tile=("row_max_tile", "max"),
            steps=("step", "max"),
        )
        .reset_index()
    )

    return games.sort_values(["agent", "game_id"]).reset_index(drop=True)


def main() -> None:
    states = load_and_merge_states()
    games = build_agent_games(states)

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    games.to_csv(OUTPUT_PATH, index=False)

    print(f"Saved {len(games)} games to {OUTPUT_PATH}")
    print(games.groupby("agent")["game_id"].count())


if __name__ == "__main__":
    main()
