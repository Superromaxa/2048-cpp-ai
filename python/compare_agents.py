from __future__ import annotations

from itertools import combinations
from pathlib import Path

import numpy as np
import pandas as pd
from scipy.stats import mannwhitneyu, ttest_ind


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT_PATH = PROJECT_ROOT / "artifacts" / "agent_games.csv"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "artifacts" / "analytics"
RANDOM_STATE = 57

REQUIRED_COLUMNS = ["agent", "game_id", "final_score", "max_tile", "steps"]


def load_agent_results(path: Path = DEFAULT_INPUT_PATH) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(
            f"Agent results file not found: {path}. "
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
        raise ValueError("Agent results file is empty")
    if df["agent"].isna().any():
        raise ValueError("Column 'agent' must not contain missing values")
    if (df["final_score"] < 0).any():
        raise ValueError("Column 'final_score' must be non-negative")
    if (df["max_tile"] < 0).any():
        raise ValueError("Column 'max_tile' must be non-negative")
    if (df["steps"] < 0).any():
        raise ValueError("Column 'steps' must be non-negative")

    return df


def summarize_agents(df: pd.DataFrame) -> pd.DataFrame:
    summary = (
        df.groupby("agent")
        .agg(
            n_games=("game_id", "count"),
            mean_score=("final_score", "mean"),
            median_score=("final_score", "median"),
            std_score=("final_score", "std"),
            min_score=("final_score", "min"),
            max_score=("final_score", "max"),
            mean_max_tile=("max_tile", "mean"),
            median_max_tile=("max_tile", "median"),
            mean_steps=("steps", "mean"),
            median_steps=("steps", "median"),
        )
        .reset_index()
    )

    return summary.sort_values("mean_score", ascending=False).reset_index(drop=True)


def bootstrap_mean_ci(
    values: pd.Series,
    n_bootstrap: int = 5_000,
    confidence: float = 0.95,
    random_state: int = RANDOM_STATE,
) -> tuple[float, float]:
    scores = values.to_numpy(dtype=float)
    if len(scores) == 0:
        raise ValueError("Cannot bootstrap an empty score array")

    rng = np.random.default_rng(random_state)
    bootstrap_means = np.empty(n_bootstrap, dtype=float)

    for i in range(n_bootstrap):
        sample = rng.choice(scores, size=len(scores), replace=True)
        bootstrap_means[i] = sample.mean()

    alpha = 1.0 - confidence
    low = np.quantile(bootstrap_means, alpha / 2.0)
    high = np.quantile(bootstrap_means, 1.0 - alpha / 2.0)
    return float(low), float(high)


def add_bootstrap_intervals(
    summary: pd.DataFrame,
    df: pd.DataFrame,
    n_bootstrap: int = 5_000,
    confidence: float = 0.95,
) -> pd.DataFrame:
    intervals = []
    for row_idx, agent in enumerate(summary["agent"]):
        agent_scores = df.loc[df["agent"] == agent, "final_score"]
        low, high = bootstrap_mean_ci(
            agent_scores,
            n_bootstrap=n_bootstrap,
            confidence=confidence,
            random_state=RANDOM_STATE + row_idx,
        )
        intervals.append(
            {
                "agent": agent,
                "mean_score_ci_low": low,
                "mean_score_ci_high": high,
            }
        )

    return summary.merge(pd.DataFrame(intervals), on="agent", how="left")


def compare_agent_pairs(df: pd.DataFrame) -> pd.DataFrame:
    agents = sorted(df["agent"].unique())
    rows = []

    for agent_a, agent_b in combinations(agents, 2):
        scores_a = df.loc[df["agent"] == agent_a, "final_score"].to_numpy(dtype=float)
        scores_b = df.loc[df["agent"] == agent_b, "final_score"].to_numpy(dtype=float)

        mean_a = scores_a.mean()
        mean_b = scores_b.mean()

        if len(scores_a) >= 2 and len(scores_b) >= 2:
            ttest_p_value = ttest_ind(scores_a, scores_b, equal_var=False).pvalue
        else:
            ttest_p_value = np.nan

        if len(scores_a) >= 1 and len(scores_b) >= 1:
            mannwhitney_p_value = mannwhitneyu(scores_a, scores_b, alternative="two-sided").pvalue
        else:
            mannwhitney_p_value = np.nan

        rows.append(
            {
                "agent_a": agent_a,
                "agent_b": agent_b,
                "n_games_a": len(scores_a),
                "n_games_b": len(scores_b),
                "mean_score_a": mean_a,
                "mean_score_b": mean_b,
                "mean_score_diff_a_minus_b": mean_a - mean_b,
                "ttest_p_value": float(ttest_p_value),
                "mannwhitney_p_value": float(mannwhitney_p_value),
            }
        )

    return pd.DataFrame(rows)


def save_results(summary: pd.DataFrame, comparisons: pd.DataFrame, output_dir: Path = DEFAULT_OUTPUT_DIR) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    summary.to_csv(output_dir / "agent_summary.csv", index=False)
    comparisons.to_csv(output_dir / "agent_pairwise_tests.csv", index=False)


def main() -> None:
    df = load_agent_results(DEFAULT_INPUT_PATH)
    summary = summarize_agents(df)
    summary = add_bootstrap_intervals(summary, df)
    comparisons = compare_agent_pairs(df)
    save_results(summary, comparisons, DEFAULT_OUTPUT_DIR)
    print(f"Saved agent comparison results to {DEFAULT_OUTPUT_DIR}")


if __name__ == "__main__":
    main()
