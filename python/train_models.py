from __future__ import annotations

import json
import math
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import pandas as pd
from scipy.stats import spearmanr
from sklearn.compose import ColumnTransformer
from sklearn.ensemble import HistGradientBoostingRegressor
from sklearn.linear_model import Ridge
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
from sklearn.model_selection import GroupShuffleSplit
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler


DATA_PATH = Path("states copy.csv")
ARTIFACTS_DIR = Path("artifacts")
RANDOM_STATE = 57

CELL_COLUMNS = [f"c{i}" for i in range(16)]
BASE_FEATURE_COLUMNS = [f"log_c{i}" for i in range(16)] + ["score", "step", "max_tile_in_corner"]
EXTENDED_FEATURE_COLUMNS = BASE_FEATURE_COLUMNS + [
    "empty_count",
    "max_tile_log2",
    "sum_tiles",
    "mergeable_count",
    "monotonicity",
    "max_tile_position",
    "max_tile_on_edge",
    "non_zero_count",
]


@dataclass
class SplitData:
    train_df: pd.DataFrame
    valid_df: pd.DataFrame
    test_df: pd.DataFrame


# returns log_2(value) or 0 if the value was 0
def tile_to_log2(value: int) -> int:
    if value == 0:
        return 0
    if value < 0 or (value & (value - 1)) != 0:
        raise ValueError(f"Tile value must be 0 or a power of two, got {value}")
    return int(math.log2(value))


# counts possible merges on the board
def count_mergeable(board: np.ndarray) -> int:
    mergeable = 0
    grid = board.reshape(4, 4)
    for i in range(4):
        for j in range(4):
            cur = grid[i, j]
            if cur == 0:
                continue
            if i + 1 < 4 and grid[i + 1, j] == cur:
                mergeable += 1
            if j + 1 < 4 and grid[i, j + 1] == cur:
                mergeable += 1
    return mergeable


# counts how many monotonic cells are on the board
def count_monotonicity(board: np.ndarray) -> int:
    bonus = 0
    grid = board.reshape(4, 4)

    for i in range(4):
        dec = 0
        inc = 0
        for j in range(3):
            a = grid[i, j]
            b = grid[i, j + 1]
            if a >= b:
                dec += a - b
            else:
                inc += b - a
        bonus += max(dec, inc)

    for j in range(4):
        dec = 0
        inc = 0
        for i in range(3):
            a = grid[i, j]
            b = grid[i + 1, j]
            if a >= b:
                dec += a - b
            else:
                inc += b - a
        bonus += max(dec, inc)

    return bonus


# checks if the board is correct
def validate_dataframe(df: pd.DataFrame) -> None:
    for col in CELL_COLUMNS:
        vals = df[col].to_numpy(dtype=np.int64)
        ok = (vals == 0) | ((vals & (vals - 1)) == 0)
        if not np.all(ok):
            bad = vals[~ok][:5]
            raise ValueError(f"{col} contains invalid tile values: {bad}")

    if (df["target"] < 0).any():
        raise ValueError("target must be non-negative")


# creates dataframe with all the features
def build_features(df: pd.DataFrame) -> pd.DataFrame:
    out = df.copy()

    board_values = out[CELL_COLUMNS].to_numpy(dtype=np.int64)
    log_board = np.vectorize(tile_to_log2)(board_values)

    for idx, col in enumerate(CELL_COLUMNS):
        out[f"log_{col}"] = log_board[:, idx]

    out["empty_count"] = (board_values == 0).sum(axis=1)
    out["non_zero_count"] = 16 - out["empty_count"]
    out["sum_tiles"] = board_values.sum(axis=1)
    out["max_tile"] = board_values.max(axis=1)
    out["max_tile_log2"] = np.vectorize(tile_to_log2)(out["max_tile"].to_numpy(dtype=np.int64))
    out["max_tile_position"] = board_values.argmax(axis=1)
    out["max_tile_on_edge"] = out["max_tile_position"].isin([0, 1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 15]).astype(int)

    mergeable = np.zeros(len(out), dtype=np.int64)
    monotonicity = np.zeros(len(out), dtype=np.int64)
    for i, row in enumerate(board_values):
        mergeable[i] = count_mergeable(row)
        monotonicity[i] = count_monotonicity(row)

    out["mergeable_count"] = mergeable
    out["monotonicity"] = monotonicity

    return out


# splits dataframe so that each game belongs to only 1 part
def split_by_game_id(df: pd.DataFrame) -> SplitData:
    groups = df["game_id"].to_numpy()
    # shuffle games by game_id
    splitter_1 = GroupShuffleSplit(n_splits=1, test_size=0.30, random_state=RANDOM_STATE)
    train_idx, holdout_idx = next(splitter_1.split(df, groups=groups))

    train_df = df.iloc[train_idx].reset_index(drop=True)
    holdout_df = df.iloc[holdout_idx].reset_index(drop=True)

    holdout_groups = holdout_df["game_id"].to_numpy()
    splitter_2 = GroupShuffleSplit(n_splits=1, test_size=0.50, random_state=RANDOM_STATE)
    valid_idx, test_idx = next(splitter_2.split(holdout_df, groups=holdout_groups))

    valid_df = holdout_df.iloc[valid_idx].reset_index(drop=True)
    test_df = holdout_df.iloc[test_idx].reset_index(drop=True)

    # verify that games are not overlapping
    assert set(train_df["game_id"]).isdisjoint(valid_df["game_id"])
    assert set(train_df["game_id"]).isdisjoint(test_df["game_id"])
    assert set(valid_df["game_id"]).isdisjoint(test_df["game_id"])

    return SplitData(train_df=train_df, valid_df=valid_df, test_df=test_df)


# prepares pipeline for training
def build_linear_pipeline(feature_columns: list[str]) -> Pipeline:
    preprocessor = ColumnTransformer(
        transformers=[
            ("scale", StandardScaler(), feature_columns),
        ],
        remainder="drop",
    )
    return Pipeline(
        steps=[
            ("preprocessor", preprocessor),
            ("model", Ridge(alpha=10.0)),
        ]
    )


# creates boosting model
def build_boosting_model() -> HistGradientBoostingRegressor:
    return HistGradientBoostingRegressor(
        loss="squared_error",
        learning_rate=0.05,
        max_iter=300,
        max_depth=6,
        min_samples_leaf=100,
        l2_regularization=1.0,
        early_stopping=True,
        validation_fraction=0.1,
        n_iter_no_change=20,
        random_state=RANDOM_STATE,
    )


# evaluation of prediction
def evaluate_predictions(y_true: np.ndarray, y_pred: np.ndarray) -> dict[str, float]:
    rmse = math.sqrt(mean_squared_error(y_true, y_pred))
    spearman = spearmanr(y_true, y_pred).statistic
    return {
        "mae": float(mean_absolute_error(y_true, y_pred)),
        "rmse": float(rmse),
        "r2": float(r2_score(y_true, y_pred)),
        "spearman": float(0.0 if np.isnan(spearman) else spearman),
    }


# evaluates models in the beginning, middle, and end parts
def evaluate_by_stage(df: pd.DataFrame, y_true: np.ndarray, y_pred: np.ndarray) -> dict[str, dict[str, float]]:
    buckets = {
        "early": df["step"] < 150,
        "mid": (df["step"] >= 150) & (df["step"] < 500),
        "late": df["step"] >= 500,
    }
    result: dict[str, dict[str, float]] = {}
    for name, mask in buckets.items():
        if int(mask.sum()) == 0:
            continue
        result[name] = evaluate_predictions(y_true[mask], y_pred[mask])
    return result


# trains model and evaluates its predictions
def train_and_eval(
    name: str,
    model,
    feature_columns: list[str],
    train_df: pd.DataFrame,
    valid_df: pd.DataFrame,
    test_df: pd.DataFrame,
    use_log_target: bool,
) -> dict:
    # features
    x_train = train_df[feature_columns]
    x_valid = valid_df[feature_columns]
    x_test = test_df[feature_columns]

    # targets
    y_train = train_df["target"].to_numpy(dtype=np.float64)
    y_valid = valid_df["target"].to_numpy(dtype=np.float64)
    y_test = test_df["target"].to_numpy(dtype=np.float64)

    # if asked, transform target scale
    target_transform = np.log1p if use_log_target else (lambda x: x)
    target_inverse = np.expm1 if use_log_target else (lambda x: x)

    # train model
    model.fit(x_train, target_transform(y_train))

    valid_pred = np.clip(target_inverse(model.predict(x_valid)), 0.0, None)
    test_pred = np.clip(target_inverse(model.predict(x_test)), 0.0, None)

    result = {
        "name": name,
        "feature_set": "extended" if feature_columns == EXTENDED_FEATURE_COLUMNS else "base",
        "target_transform": "log1p" if use_log_target else "raw",
        "valid_metrics": evaluate_predictions(y_valid, valid_pred),
        "test_metrics": evaluate_predictions(y_test, test_pred),
        "test_stage_metrics": evaluate_by_stage(test_df, y_test, test_pred),
    }

    return result


# exports data
def export_ridge_model(pipeline: Pipeline, feature_columns: list[str], output_path: Path) -> None:
    scaler: StandardScaler = pipeline.named_steps["preprocessor"].named_transformers_["scale"]
    ridge: Ridge = pipeline.named_steps["model"]

    scale = scaler.scale_.astype(float)
    mean = scaler.mean_.astype(float)
    coef = ridge.coef_.astype(float)
    intercept = float(ridge.intercept_)

    adjusted_weights = coef / scale
    adjusted_bias = intercept - float(np.dot(coef, mean / scale))

    payload = {
        "model_type": "ridge",
        "feature_names": feature_columns,
        "weights": adjusted_weights.tolist(),
        "bias": adjusted_bias,
    }
    output_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def main() -> None:
    ARTIFACTS_DIR.mkdir(exist_ok=True)

    df = pd.read_csv(DATA_PATH)
    validate_dataframe(df)
    df = build_features(df)

    split = split_by_game_id(df)

    # different models, target scales
    experiments = [
        ("ridge_base_raw", build_linear_pipeline(BASE_FEATURE_COLUMNS), BASE_FEATURE_COLUMNS, False),
        ("ridge_extended_raw", build_linear_pipeline(EXTENDED_FEATURE_COLUMNS), EXTENDED_FEATURE_COLUMNS, False),
        ("ridge_extended_log1p", build_linear_pipeline(EXTENDED_FEATURE_COLUMNS), EXTENDED_FEATURE_COLUMNS, True),
        ("hgb_extended_raw", build_boosting_model(), EXTENDED_FEATURE_COLUMNS, False),
        ("hgb_extended_log1p", build_boosting_model(), EXTENDED_FEATURE_COLUMNS, True),
    ]

    all_results = []
    trained_models = {}

    for name, model, feature_columns, use_log_target in experiments:
        result = train_and_eval(
            name=name,
            model=model,
            feature_columns=feature_columns,
            train_df=split.train_df,
            valid_df=split.valid_df,
            test_df=split.test_df,
            use_log_target=use_log_target,
        )
        all_results.append(result)
        trained_models[name] = (model, feature_columns)

    all_results.sort(key=lambda item: item["valid_metrics"]["rmse"])
    (ARTIFACTS_DIR / "metrics.json").write_text(json.dumps(all_results, indent=2), encoding="utf-8")

    best_linear_name = min(
        [name for name, *_ in experiments if name.startswith("ridge")],
        key=lambda name: next(item for item in all_results if item["name"] == name)["valid_metrics"]["rmse"],
    )
    best_linear_model, best_linear_features = trained_models[best_linear_name]
    export_ridge_model(best_linear_model, best_linear_features, ARTIFACTS_DIR / "ridge_model.json")

    print("Finished. Best experiments by validation RMSE:")
    for item in all_results:
        print(
            f"{item['name']}: "
            f"valid_rmse={item['valid_metrics']['rmse']:.3f}, "
            f"test_rmse={item['test_metrics']['rmse']:.3f}, "
            f"test_spearman={item['test_metrics']['spearman']:.4f}"
        )


if __name__ == "__main__":
    main()
