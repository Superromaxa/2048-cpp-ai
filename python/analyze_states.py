from pathlib import Path

import pandas as pd

from train_models import build_features, validate_dataframe
from sklearn.ensemble import RandomForestRegressor


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DATA_PATH = PROJECT_ROOT / "data" / "states.csv"
OUTPUT_DIR = PROJECT_ROOT / "artifacts" / "analytics"

# Important features to check
KEY_FEATURES = [
    "empty_count",
    "mergeable_count",
    "monotonicity",
    "max_tile_in_corner",
    "max_tile_log2",
    "score",
    "step",
    "sum_tiles",
    "non_zero_count",
]


# Loads DataFrame, builds features
def load_states(path: Path) -> pd.DataFrame:
    # Loading data
    df = pd.read_csv(path)

    # Validating data and building features
    validate_dataframe(df)
    df = build_features(df)

    return df


# Computes pearson (linear) and spearman (monotonic) correlations for key_features in df
def compute_correlations(df: pd.DataFrame) -> pd.DataFrame:
    # Check whether target is in df
    if "target" not in df.columns:
        raise ValueError("DataFrame must contain 'target' column")

    features = KEY_FEATURES
    rows = []

    # Computing correlations
    for feature in features:
        pearson_corr = df[feature].corr(df["target"], method="pearson")
        spearman_corr = df[feature].corr(df["target"], method="spearman")

        rows.append({
            "feature": feature,
            "pearson_corr": pearson_corr,
            "spearman_corr": spearman_corr,
        })

    # Correlation DataFrame
    result = pd.DataFrame(rows)

    # Sorting df
    result["abs_spearman_corr"] = result["spearman_corr"].abs()
    result = result.sort_values(
        by="abs_spearman_corr",
        ascending=False
    ).drop(columns="abs_spearman_corr").reset_index(drop=True)

    return result


# Trains random forest to evaluate importance of each feature
def train_feature_importance_model(df: pd.DataFrame):
    # Check whether target is in df
    if "target" not in df.columns:
        raise ValueError("DataFrame must contain 'target' column")

    feature_columns = KEY_FEATURES

    # Sampling data because full dataset may be too large for quick analytics
    sample_size = 300_000
    if len(df) > sample_size:
        train_df = df.sample(n=sample_size, random_state=57)
    else:
        train_df = df

    x = train_df[feature_columns]
    y = train_df["target"]

    model = RandomForestRegressor(
        n_estimators=100,
        max_depth=10,
        min_samples_leaf=50,
        random_state=57,
        n_jobs=-1,
    )

    model.fit(x, y)

    return model, feature_columns


# Created DataFrame with features and their importance
def compute_feature_importance(model, feature_columns: list[str]) -> pd.DataFrame:
    importances = model.feature_importances_

    result = pd.DataFrame({
        "feature": feature_columns,
        "importance": importances,
    })

    result = result.sort_values(
        by="importance",
        ascending=False
    ).reset_index(drop=True)

    return result


# Saves results to artifacts
def save_results(
    correlations: pd.DataFrame,
    importances: pd.DataFrame,
    output_dir: Path,
) -> None:
    # Save data
    output_dir.mkdir(parents=True, exist_ok=True)
    correlations.to_csv(output_dir / "state_correlations.csv", index=False)
    importances.to_csv(output_dir / "state_feature_importance.csv", index=False)



def main() -> None:
    df = load_states(DATA_PATH)

    correlations = compute_correlations(df)

    model, feature_columns = train_feature_importance_model(df)
    importances = compute_feature_importance(model, feature_columns)

    save_results(correlations, importances, OUTPUT_DIR)

    print(f"Saved results to {OUTPUT_DIR}")



if __name__ == "__main__":
    main()