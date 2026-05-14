from __future__ import annotations

from pathlib import Path

import pandas as pd


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT_PATH = PROJECT_ROOT / "artifacts" / "agent_games.csv"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "artifacts" / "analytics"

REQUIRED_COLUMNS = ["agent", "game_id", "final_score", "max_tile", "steps"]
MILESTONES = [256, 512, 1024, 2048]


def load_session_data(path: Path = DEFAULT_INPUT_PATH) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(
            f"Session data file not found: {path}. "
            "Expected columns: agent, game_id, final_score, max_tile, steps."
        )

    df = pd.read_csv(path)
    missing_columns = [column for column in REQUIRED_COLUMNS if column not in df.columns]
    if missing_columns:
        raise ValueError(f"Missing required columns: {missing_columns}")

    df = df.copy()
    df["agent"] = df["agent"].astype(str)
    df["final_score"] = pd.to_numeric(df["final_score"], errors="raise")
    df["max_tile"] = pd.to_numeric(df["max_tile"], errors="raise")
    df["steps"] = pd.to_numeric(df["steps"], errors="raise")

    if df.empty:
        raise ValueError("Session data file is empty")
    if df["agent"].isna().any():
        raise ValueError("Column 'agent' must not contain missing values")
    if (df["final_score"] < 0).any():
        raise ValueError("Column 'final_score' must be non-negative")
    if (df["max_tile"] < 0).any():
        raise ValueError("Column 'max_tile' must be non-negative")
    if (df["steps"] < 0).any():
        raise ValueError("Column 'steps' must be non-negative")

    return df


def compute_funnel(df: pd.DataFrame, milestones: list[int] = MILESTONES) -> pd.DataFrame:
    rows = []

    for agent, agent_df in df.groupby("agent"):
        total_games = len(agent_df)
        for tile in milestones:
            reached_games = int((agent_df["max_tile"] >= tile).sum())
            rows.append(
                {
                    "agent": agent,
                    "milestone_tile": tile,
                    "total_games": total_games,
                    "reached_games": reached_games,
                    "conversion_rate": reached_games / total_games,
                }
            )

    return pd.DataFrame(rows)


def compute_session_summary(df: pd.DataFrame) -> pd.DataFrame:
    summary = (
        df.groupby("agent")
        .agg(
            n_sessions=("game_id", "count"),
            expected_final_score=("final_score", "mean"),
            median_final_score=("final_score", "median"),
            expected_session_length=("steps", "mean"),
            median_session_length=("steps", "median"),
            mean_max_tile=("max_tile", "mean"),
            median_max_tile=("max_tile", "median"),
        )
        .reset_index()
    )

    return summary.sort_values("expected_final_score", ascending=False).reset_index(drop=True)


def compute_survival_curve(df: pd.DataFrame) -> pd.DataFrame:
    rows = []

    for agent, agent_df in df.groupby("agent"):
        total_games = len(agent_df)
        max_steps = int(agent_df["steps"].max())

        for step in range(max_steps + 1):
            alive_games = int((agent_df["steps"] >= step).sum())
            rows.append(
                {
                    "agent": agent,
                    "step": step,
                    "total_games": total_games,
                    "alive_games": alive_games,
                    "survival_probability": alive_games / total_games,
                }
            )

    return pd.DataFrame(rows)


def save_results(
    funnel: pd.DataFrame,
    session_summary: pd.DataFrame,
    survival_curve: pd.DataFrame,
    output_dir: Path = DEFAULT_OUTPUT_DIR,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    funnel.to_csv(output_dir / "session_funnel.csv", index=False)
    session_summary.to_csv(output_dir / "session_summary.csv", index=False)
    survival_curve.to_csv(output_dir / "session_survival_curve.csv", index=False)


def main() -> None:
    df = load_session_data(DEFAULT_INPUT_PATH)
    funnel = compute_funnel(df)
    session_summary = compute_session_summary(df)
    survival_curve = compute_survival_curve(df)
    save_results(funnel, session_summary, survival_curve, DEFAULT_OUTPUT_DIR)
    print(f"Saved session metrics to {DEFAULT_OUTPUT_DIR}")


if __name__ == "__main__":
    main()
