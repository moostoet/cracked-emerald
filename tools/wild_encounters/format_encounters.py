#!/usr/bin/env python3
"""Render grass and fishing encounters per route as Markdown tables."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Dict, Iterable, List, Sequence, Tuple

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_DATA_PATH = ROOT / "src/data/wild_encounters.json"


def load_encounters(json_path: Path) -> dict:
    with json_path.open() as handle:
        return json.load(handle)


def find_map_group(data: dict) -> dict:
    for group in data.get("wild_encounter_groups", []):
        if group.get("for_maps") and group.get("fields"):
            return group
    raise SystemExit("No map-based wild encounter group with fields was found.")


def normalize_label(label: str) -> str:
    name = re.sub(r"^g", "", label)
    name = name.replace("_", " ")
    name = re.sub(r"(?<=[A-Z])(?=[A-Z][a-z])", " ", name)
    name = re.sub(r"(?<=[a-z])(?=[A-Z])", " ", name)
    name = re.sub(r"(?<=\\D)(?=\\d)", " ", name)
    return " ".join(name.split())


def prettify_species(species: str) -> str:
    if species.startswith("SPECIES_"):
        species = species[len("SPECIES_") :]
    return " ".join(part.title() for part in species.split("_"))


def level_range(mon: dict) -> str:
    min_level = int(mon.get("min_level", mon.get("minLevel", 0)))
    max_level = int(mon.get("max_level", mon.get("maxLevel", min_level)))
    return str(min_level) if min_level == max_level else f"{min_level}-{max_level}"


def percent_string(pct: float) -> str:
    rounded = round(pct, 2)
    text = f"{rounded:.2f}".rstrip("0").rstrip(".")
    return f"{text}%"


def slot_percentages(rates: Sequence[int], indices: Iterable[int] | None = None) -> List[Tuple[int, int, float]]:
    if indices is None:
        indices = range(len(rates))
    chosen = list(indices)
    total = sum(rates[i] for i in chosen)
    if total == 0:
        return []
    return [(idx, rates[idx], (rates[idx] / total) * 100) for idx in chosen]


def markdown_table(headers: Sequence[str], rows: Sequence[Sequence[str]]) -> List[str]:
    if not rows:
        return []
    lines = ["| " + " | ".join(headers) + " |", "| " + " | ".join("---" for _ in headers) + " |"]
    for row in rows:
        lines.append("| " + " | ".join(row) + " |")
    return lines


def build_slot_metadata(fields: Sequence[dict]) -> Tuple[Dict[str, List[int]], Dict[str, Dict[str, List[int]]]]:
    rates_by_type: Dict[str, List[int]] = {}
    groups_by_type: Dict[str, Dict[str, List[int]]] = {}
    for field in fields:
        field_type = field["type"]
        rates_by_type[field_type] = field["encounter_rates"]
        if "groups" in field:
            groups_by_type[field_type] = field["groups"]
    return rates_by_type, groups_by_type


def render_grass_section(encounter: dict, slot_rates: Dict[str, List[int]]) -> List[str]:
    land_data = encounter.get("land_mons")
    if not land_data:
        return ["Grass: none"]

    rates = slot_rates.get("land_mons", [])
    rows = []
    for idx, weight, pct in slot_percentages(rates):
        if idx >= len(land_data["mons"]):
            break
        mon = land_data["mons"][idx]
        rows.append(
            [
                str(idx + 1),
                f"{percent_string(pct)} ({weight})",
                level_range(mon),
                prettify_species(mon["species"]),
            ]
        )

    lines = [f"Grass (encounter rate {land_data['encounter_rate']}):"]
    table = markdown_table(["Slot", "Rate", "Lvls", "Species"], rows)
    return lines + (table if table else ["(no slots defined)"])


def render_fishing_section(encounter: dict, slot_rates: Dict[str, List[int]], slot_groups: Dict[str, Dict[str, List[int]]]) -> List[str]:
    fishing_data = encounter.get("fishing_mons")
    if not fishing_data:
        return ["Fishing: none"]

    rates = slot_rates.get("fishing_mons", [])
    rod_groups = slot_groups.get("fishing_mons", {})
    lines: List[str] = [f"Fishing (encounter rate {fishing_data['encounter_rate']}):"]

    if not rod_groups:
        rows = []
        for idx, weight, pct in slot_percentages(rates):
            if idx >= len(fishing_data["mons"]):
                break
            mon = fishing_data["mons"][idx]
            rows.append(
                [
                    str(idx + 1),
                    f"{percent_string(pct)} ({weight})",
                    level_range(mon),
                    prettify_species(mon["species"]),
                ]
            )
        table = markdown_table(["Slot", "Rate", "Lvls", "Species"], rows)
        return lines + (table if table else ["(no fishing slots defined)"])

    for rod, indices in rod_groups.items():
        rows = []
        for idx, weight, pct in slot_percentages(rates, indices):
            if idx >= len(fishing_data["mons"]):
                break
            mon = fishing_data["mons"][idx]
            rows.append(
                [
                    str(idx + 1),
                    f"{percent_string(pct)} ({weight})",
                    level_range(mon),
                    prettify_species(mon["species"]),
                ]
            )
        lines.append(f"{rod.replace('_', ' ').title()}:")
        table = markdown_table(["Slot", "Rate", "Lvls", "Species"], rows)
        lines.extend(table if table else ["(no slots for this rod)"])
        lines.append("")

    if lines and lines[-1] == "":
        lines.pop()
    return lines


def should_render(encounter: dict, filters: Sequence[str] | None) -> bool:
    if not filters:
        return True
    haystack = " ".join(
        [
            encounter.get("map", ""),
            encounter.get("base_label", ""),
            normalize_label(encounter.get("base_label", "")),
        ]
    ).lower()
    return any(fragment.lower() in haystack for fragment in filters)


def render_map(encounter: dict, slot_rates: Dict[str, List[int]], slot_groups: Dict[str, Dict[str, List[int]]]) -> str:
    lines = [
        f"## {normalize_label(encounter['base_label'])} ({encounter['map']})",
        f"Base label: {encounter['base_label']}",
        "",
    ]
    lines.extend(render_grass_section(encounter, slot_rates))
    lines.append("")
    lines.extend(render_fishing_section(encounter, slot_rates, slot_groups))
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description="Format wild grass/fishing encounters into Markdown tables.")
    parser.add_argument(
        "--data",
        type=Path,
        default=DEFAULT_DATA_PATH,
        help="Path to wild_encounters.json (default: %(default)s)",
    )
    parser.add_argument(
        "--map",
        action="append",
        dest="maps",
        help="Optional substring filter for map/base labels (can be repeated).",
    )
    args = parser.parse_args()

    data = load_encounters(args.data)
    map_group = find_map_group(data)
    slot_rates, slot_groups = build_slot_metadata(map_group["fields"])

    outputs = []
    for encounter in map_group["encounters"]:
        if not should_render(encounter, args.maps):
            continue
        outputs.append(render_map(encounter, slot_rates, slot_groups))

    if not outputs:
        raise SystemExit("No encounters matched the given filters.")

    print("\n\n".join(outputs))


if __name__ == "__main__":
    main()
