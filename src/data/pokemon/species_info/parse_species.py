#!/usr/bin/env python3
import re
import json
import os
import ast
import operator as op

#------------------------------------------------------------------------------
# Mapping dictionaries (adjust/extend as needed)
#------------------------------------------------------------------------------
TYPE_MAPPING = {
    "TYPE_NORMAL":    "Normal",
    "TYPE_FIRE":      "Fire",
    "TYPE_WATER":     "Water",
    "TYPE_GRASS":     "Grass",
    "TYPE_ELECTRIC":  "Electric",
    "TYPE_ICE":       "Ice",
    "TYPE_FIGHTING":  "Fighting",
    "TYPE_POISON":    "Poison",
    "TYPE_GROUND":    "Ground",
    "TYPE_FLYING":    "Flying",
    "TYPE_PSYCHIC":   "Psychic",
    "TYPE_BUG":       "Bug",
    "TYPE_ROCK":      "Rock",
    "TYPE_GHOST":     "Ghost",
    "TYPE_STEEL":     "Steel",
    "TYPE_DRAGON":    "Dragon",
    "TYPE_DARK":      "Dark",
    "TYPE_FAIRY":     "Fairy"
}
EGG_GROUP_MAPPING = {
    "EGG_GROUP_MONSTER":      "Monster",
    "EGG_GROUP_GRASS":        "Grass",
    "EGG_GROUP_WATER_1":      "Water 1",
    "EGG_GROUP_WATER_2":      "Water 2",
    "EGG_GROUP_WATER_3":      "Water 3",
    "EGG_GROUP_BUG":          "Bug",
    "EGG_GROUP_DRAGON":       "Dragon",
    "EGG_GROUP_FLYING":       "Flying",
    "EGG_GROUP_FIELD":        "Field",
    "EGG_GROUP_HUMAN_LIKE":   "Human-Like",
    "EGG_GROUP_FAIRY":        "Fairy",
    "EGG_GROUP_MINERAL":      "Mineral",
    "EGG_GROUP_AMORPHOUS":    "Amorphous",
    "EGG_GROUP_DITTO":        "Ditto",
    "EGG_GROUP_NO_EGGS":      "No Eggs Discovered"
}

def convert_ability(tok):
    """Convert a string like 'ABILITY_OVERGROW' into 'Overgrow' (also handles ABILITY_NONE)."""
    tok = tok.strip()
    if tok.startswith("ABILITY_"):
        ab = tok[len("ABILITY_"):]
        return ab.replace("_", " ").title()
    return tok

# Define allowed operators
allowed_operators = {
    ast.Add: op.add,
    ast.Sub: op.sub,
    ast.Mult: op.mul,
    ast.Div: op.truediv,
    ast.Mod: op.mod,
    ast.Pow: op.pow,
}

def safe_eval(expr):
    """
    Safely evaluate a simple arithmetic expression (e.g., '2+3*4').
    Only non-negative numbers and the allowed operators are supported.
    """
    tree = ast.parse(expr, mode='eval')
    def eval_node(node):
        if isinstance(node, ast.Expression):
            return eval_node(node.body)
        elif isinstance(node, ast.Constant):  # Python 3.8+
            if isinstance(node.value, (int, float)):
                return node.value
            else:
                raise ValueError("Unsupported constant type: {}".format(type(node.value)))
        elif isinstance(node, ast.BinOp):
            left_val = eval_node(node.left)
            right_val = eval_node(node.right)
            operator = allowed_operators.get(type(node.op))
            if operator is None:
                raise ValueError("Operator not allowed: {}".format(type(node.op)))
            return operator(left_val, right_val)
        elif isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.UAdd, ast.USub)):
            operand = eval_node(node.operand)
            return operand if isinstance(node.op, ast.UAdd) else -operand
        else:
            raise ValueError("Unsupported expression: {}".format(node))
    return eval_node(tree)

#------------------------------------------------------------------------------
# Helper to get exception family key from a species identifier.
# For families whose alternative forms should be merged, we return a common key.
#------------------------------------------------------------------------------
def get_exception_family(identifier):
    if identifier.startswith("SPECIES_FLABEBE"):
        return "SPECIES_FLABEBE"
    if identifier.startswith("SPECIES_VIVILLON"):
        return "SPECIES_VIVILLON"
    if identifier.startswith("SPECIES_SCATTERBUG"):
        return "SPECIES_SCATTERBUG"
    if identifier.startswith("SPECIES_SPEWPA"):
        return "SPECIES_SPEWPA"
    if identifier.startswith("SPECIES_FLORGES"):
        return "SPECIES_FLORGES"
    if identifier.startswith("SPECIES_UNOWN"):
        return "SPECIES_UNOWN"
    if identifier.startswith("SPECIES_GENESECT"):
        return "SPECIES_GENESECT"
    if identifier.startswith("SPECIES_FURFROU"):
        return "SPECIES_FURFROU"
    if identifier.startswith("SPECIES_ZYGARDE_50_POWER_CONSTRUCT") or \
       identifier.startswith("SPECIES_ZYGARDE_50"):
        return "SPECIES_ZYGARDE_50"
    if identifier.startswith("SPECIES_ZYGARDE_10_POWER_CONSTRUCT") or \
       identifier.startswith("SPECIES_ZYGARDE_10"):
        return "SPECIES_ZYGARDE_10"
    if identifier.startswith("SPECIES_GUMSHOOS"):
        return "SPECIES_GUMSHOOS"
    if identifier.startswith("SPECIES_VIKAVOLT"):
        return "SPECIES_VIKAVOLT"
    if identifier.startswith("SPECIES_RIBOMBEE"):
        return "SPECIES_RIBOMBEE"
    if identifier.startswith("SPECIES_ROCKRUFF"):
        return "SPECIES_ROCKRUFF"
    if identifier.startswith("SPECIES_ARAQUANID"):
        return "SPECIES_ARAQUANID"
    if identifier.startswith("SPECIES_LURANTIS"):
        return "SPECIES_LURANTIS"
    if identifier.startswith("SPECIES_SALAZZLE"):
        return "SPECIES_SALAZZLE"
    if identifier.startswith("SPECIES_TOGEDEMARU"):
        return "SPECIES_TOGEDEMARU"
    if identifier.startswith("SPECIES_MIMIKYU"):
        return "SPECIES_MIMIKYU"
    if identifier.startswith("SPECIES_KOMMO_O"):
        return "SPECIES_KOMMO_O"
    if identifier.startswith("SPECIES_ALCREMIE"):
        return "SPECIES_ALCREMIE"
    if identifier.startswith("SPECIES_OGERPON_TEAL_TERA") or \
       identifier.startswith("SPECIES_OGERPON_TEAL") or \
       identifier.startswith("SPECIES_OGERPON_WELLSPRING_TERA") or \
       identifier.startswith("SPECIES_OGERPON_HEARTHFLAME_TERA") or \
       identifier.startswith("SPECIES_OGERPON_CORNERSTONE_TERA"):
        return "SPECIES_OGERPON"
    if identifier.startswith("SPECIES_CRAMORANT"):
        return "SPECIES_CRAMORANT"
    if identifier.startswith("SPECIES_XERNEAS"):
        return "SPECIES_XERNEAS"
    return None

#------------------------------------------------------------------------------
# Helper: format the species name if its identifier includes special suffixes.
#
# Examples:
#   SPECIES_VENUSAUR_MEGA  -> Venusaur-Mega
#   SPECIES_ZIGZAGOON_GALAR -> Zigzagoon-Galar
#   SPECIES_ZOROARK_HISUI   -> Zoroark-Hisui
#------------------------------------------------------------------------------
def format_species_name(identifier, raw_name):
    if identifier == "SPECIES_ZACIAN_HERO":
        return raw_name.strip()
    suffixes = {
        "MEGA": "Mega",
        "MEGA_X": "Mega-X",
        "MEGA_Y": "Mega-Y",
        "GALAR": "Galar",
        "HISUI": "Hisui",
        "ALOLA": "Alola",
        "10_AURA_BREAK": "10%",
        "COMPLETE": "Complete",
        "BLADE": "Blade",
        "SPIKY_EARED": "Spiky-Eared",
        "COSPLAY": "Cosplay",
        "ROCK_STAR": "Rock-Star",
        "BELLE": "Belle",
        "POP_STAR": "Pop-Star",
        "PHD": "PhD",
        "LIBRE": "Libre",
        "ORIGINAL": "Original",
        "HOENN": "Hoenn",
        "SINNOH": "Sinnoh",
        "UNOVA": "Unova",
        "KALOS": "Kalos",
        "ALOLA": "Alola",
        "PARTNER": "Partner",
        "WORLD": "World",
        "BLOODMOON": "Bloodmoon",
        "STARTER": "Starter",
        "SUNNY": "Sunny",
        "RAINY": "Rainy",
        "SNOWY": "Snowy",
        "ATTACK": "Attack",
        "DEFENSE": "Defense",
        "SPEED": "Speed",
        "PRIMAL": "Primal",
        "SANDY": "Sandy",
        "TRASH": "Trash",
        "SUNSHINE": "Sunshine",
        "EAST": "East",
        "HEAT": "Heat",
        "WASH": "Wash",
        "FROST": "Frost",
        "FAN": "Fan",
        "MOW": "Mow",
        "ORIGIN": "Origin",
        "SKY": "Sky",
        "BLUE_STRIPED": "Blue-Striped",
        "WHITE_STRIPED": "White-Striped",
        "F": "F",
        "ZEN": "Zen",
        "GALAR_STANDARD": "Galar",
        "GALAR_ZEN": "Galar-Zen",
        "SUMMER": "Summer",
        "AUTUMN": "Autumn",
        "WINTER": "Winter",
        "THERIAN": "Therian",
        "BLACK": "Black",
        "WHITE": "White",
        "RESOLUTE": "Resolute",
        "PIROUETTE": "Pirouette",
        "BATTLE_BOND": "Battle-Bond",
        "ASH": "Ash",
        "PALDEA_COMBAT": "Paldea-Combat",
        "PALDEA_BLAZE": "Paldea-Blaze",
        "PALDEA_AQUA": "Paldea-Aqua",
        "FIRE": "Fire",
        "WATER": "Water",
        "GRASS": "Grass",
        "ELECTRIC": "Electric",
        "ICE": "Ice",
        "FIGHTING": "Fighting",
        "POISON": "Poison",
        "GROUND": "Ground",
        "FLYING": "Flying",
        "PSYCHIC": "Psychic",
        "BUG": "Bug",
        "ROCK": "Rock",
        "GHOST": "Ghost",
        "STEEL": "Steel",
        "DRAGON": "Dragon",
        "DARK": "Dark",
        "FAIRY": "Fairy",
        "SMALL": "Small",
        "LARGE": "Large",
        "SUPER": "Super",
        "UNBOUND": "Unbound",
        "POM_POM": "Pom-Pom",
        "PAU": "Pa'u",
        "SENSU": "Sensu",
        "DUSK": "Dusk",
        "MIDNIGHT": "Midnight",
        "SCHOOL": "School",
        "DUSK_MANE": "Dusk-Mane",
        "DAWN_WINGS": "Dawn-Wings",
        "ULTRA": "Ultra",
        "NOICE": "Noice",
        "BLUE": "Blue",
        "YELLOW": "Yellow",
        "DROOPY": "Droopy",
        "STRETCHY": "Stretchy",
        "ROAMING": "Roaming",
        "ARTISAN": "Artisan",
        "MASTERPIECE": "Masterpiece",
        "WELLSPRING": "Wellspring",
        "HEARTHFLAME": "Hearthflame",
        "CORNERSTONE": "Cornerstone",
        "TERASTAL": "Terastal",
        "STELLAR": "Stellar",
        "SINGLE_STRIKE": "Single-Strike",
        "RAPID_STRIKE": "Rapid-Strike",
        "CROWNED": "Crowned",
        "HANGRY": "Hangry",
        "ANTIQUE": "Antique",
        "LOW_KEY": "Low-Key",
        "HERO": "Hero"
    }
    if identifier.startswith("SPECIES_"):
        ident = identifier[len("SPECIES_"):]
    else:
        ident = identifier
    parts = ident.split("_")
    for i in range(1, len(parts)):
        candidate = "_".join(parts[i:])
        if candidate in suffixes:
            suffix_expanded = suffixes[candidate]
            if raw_name.strip().lower().endswith(suffix_expanded.lower()):
                return raw_name.strip()
            else:
                return f"{raw_name.strip()}-{suffix_expanded}"
    return raw_name.strip()

#------------------------------------------------------------------------------
# --- MACRO AND CONSTANT PROCESSING HELPERS ---
#------------------------------------------------------------------------------
def extract_macros(lines):
    """
    Extract both function-style and plain macros.
    For plain macros, use a regex that accepts leading whitespace.
    In case of duplicate definitions, the first (assumed “latest generation”)
    is used.
    """
    macros = {}
    i = 0
    total = len(lines)
    while i < total:
        line = lines[i]
        if re.match(r'^\s*#define\s+', line):
            # Try function-style macro first.
            m_func = re.match(r'^\s*#define\s+([A-Z0-9_]+)\s*\(([^)]*)\)\s*(\\?.*)', line)
            if m_func:
                macro_name = m_func.group(1)
                params = [p.strip() for p in m_func.group(2).split(",") if p.strip()]
                body = m_func.group(3).rstrip(" \n\\")
                i += 1
                while i < total and lines[i].rstrip().endswith("\\"):
                    body += "\n" + lines[i].rstrip(" \n\\")
                    i += 1
                macros[macro_name] = {"params": params, "body": body}
            else:
                # Try plain macro.
                m_plain = re.match(r'^\s*#define\s+([A-Z0-9_]+)\s+(.*)', line)
                if m_plain:
                    macro_name = m_plain.group(1)
                    if macro_name not in macros:
                        body = m_plain.group(2).strip()
                        # Handle multi-line plain macros that end with '\'
                        while line.rstrip().endswith("\\") and i+1 < total:
                            i += 1
                            line = lines[i]
                            body += " " + line.strip().rstrip("\\")
                        macros[macro_name] = {"params": [], "body": body}
                i += 1
        else:
            i += 1
    return macros

def extract_constants(lines):
    """
    Extract constant definitions from lines.
    For conditional macros we always choose the modern value.
    """
    constants = {}
    pattern = re.compile(r'#define\s+([A-Z0-9_]+)\s+\(([^)]+)\)')
    for line in lines:
        m = pattern.match(line)
        if m:
            name = m.group(1)
            definition = m.group(2).strip()
            m_expr = re.search(r'P_UPDATED_STATS\s*>=\s*GEN_\d+\s*\?\s*(\d+)\s*:\s*(\d+)', definition)
            if m_expr:
                constants[name] = int(m_expr.group(1))
            else:
                try:
                    constants[name] = int(definition)
                except ValueError:
                    pass
    return constants

def expand_macro(macro_def, arg_values):
    """
    Expand a macro given its definition and a list of argument values.
    First replaces any token concatenation (##), then replaces occurrences
    of the formal parameters.
    """
    body = macro_def["body"]
    params = macro_def["params"]
    for j, param in enumerate(params):
        if j < len(arg_values):
            arg = arg_values[j]
            body = re.sub(r'\s*##\s*' + re.escape(param), arg, body)
    for j, param in enumerate(params):
        if j < len(arg_values):
            arg = arg_values[j]
            body = re.sub(r'\b' + re.escape(param) + r'\b', arg, body)
    return body

def evaluate_stat_expression(expr, constants):
    """
    Evaluate a base stat expression.
    For ternary operators choose the true_value.
    Replace tokens via constants and evaluate.
    """
    if "?" in expr:
        m = re.search(r'\?(.*?)\:(.*)', expr)
        if m:
            expr = m.group(1).strip()
    def replacer(match):
        token = match.group(0)
        if token.isdigit():
            return token
        return str(constants.get(token, token))
    expr_with_values = re.sub(r'\b[A-Z0-9_]+\b', replacer, expr)
    try:
        return safe_eval(expr_with_values)
    except Exception:
        return 0

def recursively_expand(text, macros):
    """
    Recursively scan the text for standalone macro invocations and replace them.
    """
    macro_line_pattern = r'^\s*([A-Z0-9_]+)\s*\(([^)]*)\)\s*,?\s*$'
    prev = None
    while text != prev:
        prev = text
        new_lines = []
        for line in text.splitlines():
            m_macro = re.match(macro_line_pattern, line)
            if m_macro:
                macro_name = m_macro.group(1)
                macro_args_str = m_macro.group(2)
                macro_args = [a.strip() for a in macro_args_str.split(",") if a.strip()]
                if macro_name in macros:
                    replaced = expand_macro(macros[macro_name], macro_args)
                    new_lines.append(replaced)
                else:
                    new_lines.append(line)
            else:
                new_lines.append(line)
        text = "\n".join(new_lines)
    return text

#------------------------------------------------------------------------------
# Parse a block of text (inside curly braces) to extract fields.
#------------------------------------------------------------------------------
def parse_pokemon_block(block_text, constants, macros):
    data = {}
    m = re.search(r'\.speciesName\s*=\s*_\("([^"]+)"\)', block_text)
    data["name"] = m.group(1) if m else ""
    m = re.search(r'\.categoryName\s*=\s*_\("([^"]+)"\)', block_text)
    data["category"] = m.group(1) if m else ""
    new_stat_fields = [
        (r'\.baseHP\s*=\s*([^,\n]+)', "HP"),
        (r'\.baseAttack\s*=\s*([^,\n]+)', "Attack"),
        (r'\.baseDefense\s*=\s*([^,\n]+)', "Defense"),
        (r'\.baseSpAttack\s*=\s*([^,\n]+)', "Sp. Attack"),
        (r'\.baseSpDefense\s*=\s*([^,\n]+)', "Sp. Defense"),
        (r'\.baseSpeed\s*=\s*([^,\n]+)', "Speed")
    ]
    stats = {}
    for stat_regex, key in new_stat_fields:
        m = re.search(stat_regex, block_text)
        if m:
            expr = m.group(1).strip()
            stats[key] = evaluate_stat_expression(expr, constants)
        else:
            stats[key] = 0
    data["baseStats"] = stats
    # --- Types extraction ---
    types_tokens = None
    m = re.search(r'\.types\s*=\s*MON_TYPES\s*\(\s*([^)]+)\)', block_text)
    if m:
        types_tokens = m.group(1)
    else:
        m = re.search(r'\.types\s*=\s*(\{[^}]+\}|\w+)\s*,', block_text)
        if m:
            token_str = m.group(1).strip()
            if token_str.startswith("{") and token_str.endswith("}"):
                types_tokens = token_str[1:-1]
            else:
                if token_str in macros:
                    expanded = expand_macro(macros[token_str], [])
                    expanded = recursively_expand(expanded, macros).strip()
                    if expanded.startswith("{") and expanded.endswith("}"):
                        types_tokens = expanded[1:-1]
                    else:
                        types_tokens = expanded
                else:
                    types_tokens = ""
    if types_tokens:
        tokens = [t.strip() for t in types_tokens.split(",") if t.strip()]
        data["types"] = tokens
    else:
        data["types"] = []
    m = re.search(r'\.eggGroups\s*=\s*MON_EGG_GROUPS\s*\(\s*([^)]+)\)', block_text)
    if m:
        tokens = [t.strip() for t in m.group(1).split(",") if t.strip()]
        data["eggGroups"] = tokens
    else:
        data["eggGroups"] = []
    m = re.search(r'\.abilities\s*=\s*\{([^}]+)\}', block_text, re.DOTALL)
    if m:
        toks = [t.strip() for t in m.group(1).split(",") if t.strip()]
        data["abilities"] = toks
    else:
        data["abilities"] = []
    m = re.search(r'\.height\s*=\s*(\d+)', block_text)
    data["height"] = int(m.group(1)) if m else 0
    m = re.search(r'\.weight\s*=\s*(\d+)', block_text)
    data["weight"] = int(m.group(1)) if m else 0
    m = re.search(r'\.evolutions\s*=\s*EVOLUTION\s*\(\s*\{([^}]+)\}\s*\)', block_text)
    if m:
        parts = [p.strip() for p in m.group(1).split(",")]
        if len(parts) >= 3:
            evolve = {"method": parts[0], "level": parts[1], "target": parts[2]}
            data["evolution"] = evolve
    else:
        data["evolution"] = None
    return data

#------------------------------------------------------------------------------
# Process one .h file: locate Pokémon definitions and extract initializer blocks.
#------------------------------------------------------------------------------
def parse_file(filename):
    pokemon_entries = []
    try:
        with open(filename, "r", encoding="utf-8") as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {filename}: {e}")
        return pokemon_entries
    macros = extract_macros(lines)
    constants = extract_constants(lines)
    i = 0
    total = len(lines)
    while i < total:
        line = lines[i]
        m = re.match(r'\s*\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=', line)
        if m:
            species_id = m.group(1)
            if 'GMAX' in species_id or 'ETERNAMAX' in species_id:
                if "{" not in line:
                    i += 1
                    while i < total and "{" not in lines[i]:
                        i += 1
                    if i >= total:
                        break
                brace_count = 1
                i += 1
                while i < total and brace_count > 0:
                    current_line = lines[i]
                    brace_count += current_line.count("{") - current_line.count("}")
                    i += 1
                continue
            macro_entry = re.match(r'\s*\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*([A-Z0-9_]+)\s*\(([^)]*)\)', line)
            if macro_entry:
                species_id = macro_entry.group(1)
                macro_name = macro_entry.group(2)
                macro_args_str = macro_entry.group(3).strip()
                macro_args = [a.strip() for a in macro_args_str.split(",") if a.strip()]
                if macro_name in macros:
                    expanded_text = expand_macro(macros[macro_name], macro_args)
                    expanded_text = recursively_expand(expanded_text, macros)
                    entry_data = parse_pokemon_block(expanded_text, constants, macros)
                    entry_data["identifier"] = species_id
                    pokemon_entries.append(entry_data)
                    i += 1
                    continue
                else:
                    print(f"Warning: macro {macro_name} not found; skipping entry {species_id}")
                    i += 1
                    continue
            block_lines = []
            if "{" not in line:
                i += 1
                while i < total and "{" not in lines[i]:
                    i += 1
                if i >= total:
                    break
            line = lines[i]
            block_lines.append(line)
            brace_count = line.count("{") - line.count("}")
            i += 1
            while i < total and brace_count > 0:
                line = lines[i]
                brace_count += line.count("{") - line.count("}")
                block_lines.append(line)
                i += 1
            macro_line_pattern = r'^\s*([A-Z0-9_]+)\s*\(([^)]*)\)\s*,?\s*$'
            new_block_lines = []
            for bline in block_lines:
                m_macro = re.match(macro_line_pattern, bline)
                if m_macro:
                    macro_name = m_macro.group(1)
                    macro_args_str = m_macro.group(2)
                    macro_args = [a.strip() for a in macro_args_str.split(",") if a.strip()]
                    if macro_name in macros:
                        expanded = expand_macro(macros[macro_name], macro_args)
                        new_block_lines.append(expanded)
                    else:
                        new_block_lines.append(bline)
                else:
                    new_block_lines.append(bline)
            block_text = "\n".join(new_block_lines)
            block_text = recursively_expand(block_text, macros)
            entry_data = parse_pokemon_block(block_text, constants, macros)
            entry_data["identifier"] = species_id
            pokemon_entries.append(entry_data)
            continue
        i += 1
    return pokemon_entries

#------------------------------------------------------------------------------
# Main: process files, deduplicate exceptions, assign IDs, build evolutions,
# and output JSON.
#------------------------------------------------------------------------------
def main():
    all_pokemon = []
    for gen in range(1, 10):
        filename = f"gen_{gen}_families.h"
        if not os.path.exists(filename):
            print(f"File not found: {filename} (skipping)")
            continue
        print(f"Parsing {filename} ...")
        entries = parse_file(filename)
        all_pokemon.extend(entries)
    if not all_pokemon:
        print("No Pokémon entries were found. Check your file paths or formatting.")
        return
    unique_pokemon = []
    seen_exceptions = {}
    for poke in all_pokemon:
        name = poke.get("name", "").strip()
        if not name:
            continue
        fam = get_exception_family(poke.get("identifier", ""))
        if fam:
            if fam in seen_exceptions:
                continue
            seen_exceptions[fam] = True
        unique_pokemon.append(poke)
    id_mapping = {}
    next_id = 1
    for poke in unique_pokemon:
        poke["id"] = next_id
        id_mapping[poke["identifier"]] = next_id
        next_id += 1
    json_list = []
    for poke in unique_pokemon:
        poke_id = poke.get("id", 0)
        formatted_id = f"{poke_id:03}"
        raw_name = poke.get("name", "")
        species_identifier = poke.get("identifier", "")
        formatted_name = format_species_name(species_identifier, raw_name)
        types = [TYPE_MAPPING.get(t, t) for t in poke.get("types", [])]
        base = poke.get("baseStats", {})
        egggroups = [EGG_GROUP_MAPPING.get(g, g) for g in poke.get("eggGroups", [])]
        abilities = [convert_ability(a) for a in poke.get("abilities", [])]
        height_str = f"{poke.get('height', 0) / 10:.1f} m"
        weight_str = f"{poke.get('weight', 0) / 10:.1f} kg"
        evolution_obj = {}
        evo = poke.get("evolution")
        if evo is not None:
            method = evo.get("method")
            param = evo.get("level")
            target_tok = evo.get("target")
            target_id = id_mapping.get(target_tok)
            if target_id is not None:
                if method == "EVO_LEVEL":
                    evolution_obj = {"next": [[str(target_id), f"Level {param}"]]}
                else:
                    evolution_obj = {"next": [[str(target_id), method, param]]}
        image = {
            "sprite": f"https://raw.githubusercontent.com/May8th1995/sprites/master/{formatted_name}.png",
        }
        json_obj = {
            "id": formatted_id,
            "name": formatted_name,
            "type": types,
            "base": {
                "HP": base.get("HP", 0),
                "Attack": base.get("Attack", 0),
                "Defense": base.get("Defense", 0),
                "Sp. Attack": base.get("Sp. Attack", 0),
                "Sp. Defense": base.get("Sp. Defense", 0),
                "Speed": base.get("Speed", 0)
            },
            "species": poke.get("category", ""),
            "evolution": evolution_obj,
            "profile": {
                "height": height_str,
                "weight": weight_str,
                "egg": egggroups,
                "ability": abilities
            },
            "image": image
        }
        json_list.append(json_obj)
    with open("pokedex.json", "w", encoding="utf-8") as outfile:
        json.dump(json_list, outfile, indent=2)
    print(f"Done: {len(json_list)} Pokémon written to pokedex.json.")

if __name__ == "__main__":
    main()