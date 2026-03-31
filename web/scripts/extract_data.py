#!/usr/bin/env python3
"""
Extract game data from Pokemon ROM hack C source files and output JSON
for the Pokedex web app.

Usage: python3 extract_data.py [repo_root]
"""

import json
import os
import re
import sys
from collections import defaultdict


def find_repo_root(hint=None):
    """Find the repository root directory."""
    if hint and os.path.isdir(hint):
        return os.path.abspath(hint)
    # Try to find it relative to this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # web/scripts/ -> repo root is ../../
    candidate = os.path.normpath(os.path.join(script_dir, '..', '..'))
    if os.path.isfile(os.path.join(candidate, 'include', 'constants', 'species.h')):
        return candidate
    print("ERROR: Cannot find repo root. Pass it as an argument.")
    sys.exit(1)


# ---------------------------------------------------------------------------
# Utility helpers
# ---------------------------------------------------------------------------

def extract_string(text):
    """Extract string content from _("...") or COMPOUND_STRING("...") macros,
    handling multi-line concatenation."""
    # Remove COMPOUND_STRING( wrapper or _( wrapper
    text = text.strip()
    # Handle COMPOUND_STRING("line1\n" "line2")
    strings = re.findall(r'"((?:[^"\\]|\\.)*)"', text)
    result = ''.join(strings)
    # Clean up escape sequences for display
    result = result.replace('\\n', ' ').replace('\\l', ' ').replace('\\p', ' ')
    # Collapse multiple spaces
    result = re.sub(r'  +', ' ', result).strip()
    return result


def format_constant(prefix, name):
    """Convert a C constant like GROWTH_MEDIUM_SLOW to 'Medium Slow'."""
    if name.startswith(prefix):
        name = name[len(prefix):]
    return name.replace('_', ' ').title()


def format_item_name(item_const):
    """Convert ITEM_FIRE_STONE to 'Fire Stone'."""
    return format_constant('ITEM_', item_const)


def take_first_ternary(value_str):
    """For conditional expressions like 'B_X >= GEN_2 ? TYPE_FIGHTING : TYPE_NORMAL',
    take the first value (modern default)."""
    value_str = value_str.strip()
    m = re.match(r'.+\?\s*(.+?)\s*:\s*(.+)', value_str)
    if m:
        return m.group(1).strip()
    return value_str


def resolve_ternary_int(value_str):
    """Resolve a ternary to an int, taking the modern value."""
    val = take_first_ternary(value_str)
    try:
        return int(val)
    except ValueError:
        return 0


# ---------------------------------------------------------------------------
# Parse enum constants from header files
# ---------------------------------------------------------------------------

def parse_enum_constants(filepath, prefix):
    """Parse an enum from a C header file, returning {NAME: numeric_id}.
    Skips aliases (entries referencing other constants)."""
    constants = {}
    aliases = {}
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    # Find enum block - handle multi-line enum declarations
    # The enum may be: "enum ... {" on one line, or "enum ...\n{" on two lines
    in_enum = False
    waiting_for_brace = False
    current_val = 0
    for line in content.split('\n'):
        line = line.strip()
        if waiting_for_brace:
            if line == '{':
                in_enum = True
                waiting_for_brace = False
                continue
            else:
                waiting_for_brace = False
        if not in_enum and line.startswith('enum '):
            if '{' in line:
                in_enum = True
                continue
            else:
                # Brace may be on the next line
                waiting_for_brace = True
                continue
        if in_enum and (line == '};' or line.startswith('};')):
            break
        if not in_enum:
            continue

        # Skip comments and empty lines
        if not line or line.startswith('//') or line.startswith('/*') or line.startswith('*'):
            continue

        # Match: SPECIES_BULBASAUR = 1,
        m = re.match(r'(' + prefix + r'\w+)\s*=\s*(.+?)\s*,?\s*(?://.*)?$', line)
        if m:
            name = m.group(1)
            val_str = m.group(2).strip()
            # Check if this is an alias (references another constant with same prefix)
            if re.match(r'' + prefix + r'\w+', val_str) or re.match(r'MOVE_\w+', val_str):
                aliases[name] = val_str
                continue
            try:
                val = int(val_str)
                constants[name] = val
                current_val = val + 1
            except ValueError:
                # Complex expression, skip
                continue
        elif re.match(r'(' + prefix + r'\w+)\s*,', line):
            # Auto-incrementing enum entry
            name = re.match(r'(' + prefix + r'\w+)', line).group(1)
            constants[name] = current_val
            current_val += 1

    return constants


def parse_pokedex_constants(filepath):
    """Parse NATIONAL_DEX_XXX constants."""
    constants = {}
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    in_enum = False
    current_val = 0
    for line in content.split('\n'):
        line = line.strip()
        if 'enum' in line and 'NationalDexOrder' in line:
            in_enum = True
            continue
        if in_enum and line.startswith('};'):
            break
        if not in_enum:
            continue
        if not line or line.startswith('//'):
            continue

        m = re.match(r'(NATIONAL_DEX_\w+)\s*(?:=\s*(\d+))?\s*,?\s*(?://.*)?$', line)
        if m:
            name = m.group(1)
            if m.group(2):
                current_val = int(m.group(2))
            constants[name] = current_val
            current_val += 1

    return constants


# ---------------------------------------------------------------------------
# Parse item names from items.h
# ---------------------------------------------------------------------------

def parse_item_names(filepath):
    """Parse ITEM_XXX -> name mapping from items data."""
    items = {}
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    # Find [ITEM_XXX] = { .name = ITEM_NAME("Xxx"), ... }
    pattern = re.compile(
        r'\[(ITEM_\w+)\]\s*=\s*\{[^}]*?\.name\s*=\s*ITEM_NAME\(\s*"([^"]+)"\s*\)',
        re.DOTALL
    )
    for m in pattern.finditer(content):
        items[m.group(1)] = m.group(2)

    return items


# ---------------------------------------------------------------------------
# Species Parser
# ---------------------------------------------------------------------------

def parse_shared_dex_text(repo_root):
    """Parse shared_dex_text.h to build a map of variable name -> description text."""
    filepath = os.path.join(repo_root, 'src', 'data', 'pokemon', 'species_info', 'shared_dex_text.h')
    dex_texts = {}
    if not os.path.isfile(filepath):
        return dex_texts

    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    # Match: const u8 gVarName[] = _( "text..." );
    # or COMPOUND_STRING(...)
    pattern = re.compile(
        r'const\s+u8\s+(\w+)\[\]\s*=\s*(_\(|COMPOUND_STRING\()(.*?)\);',
        re.DOTALL
    )
    for m in pattern.finditer(content):
        var_name = m.group(1)
        text_block = m.group(2) + m.group(3) + ')'
        text = extract_string(text_block)
        if text:
            dex_texts[var_name] = text

    return dex_texts


def parse_species_info(repo_root, species_ids, natdex_ids, move_names, ability_names, item_names_map):
    """Parse all species info from gen_*_families.h files."""
    species_dir = os.path.join(repo_root, 'src', 'data', 'pokemon', 'species_info')
    pokemon_list = []
    learnset_map = defaultdict(list)  # variable name -> [(learnset_type, species_id), ...]

    # Parse shared dex text for resolving variable references
    shared_dex_texts = parse_shared_dex_text(repo_root)
    print(f"  Loaded {len(shared_dex_texts)} shared dex texts")

    for gen_num in range(1, 10):
        filepath = os.path.join(species_dir, f'gen_{gen_num}_families.h')
        if not os.path.isfile(filepath):
            print(f"  Warning: {filepath} not found, skipping")
            continue

        with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()

        print(f"  Parsing gen_{gen_num}_families.h...")
        entries = parse_species_entries(content, gen_num, species_ids, natdex_ids,
                                         move_names, ability_names, item_names_map,
                                         learnset_map, shared_dex_texts)
        pokemon_list.extend(entries)

    return pokemon_list, learnset_map


def parse_species_macros(content):
    """Find all #define macros that look like species info macros (contain .speciesName
    or reference other _MISC_INFO/_SPECIES_INFO macros that do).
    Returns dict of macro_name -> (param_names_list, body_text)."""
    # First pass: collect ALL macros with _SPECIES_INFO or _MISC_INFO in the name
    all_macros = {}
    # Match macros WITH parameters
    define_pattern = re.compile(
        r'#define\s+(\w+_(?:SPECIES_INFO|MISC_INFO))\s*\(([^)]*)\)\s*\\?\s*\n((?:.*\\\s*\n)*.*\n?)'
    )
    for m in define_pattern.finditer(content):
        macro_name = m.group(1)
        params_str = m.group(2).strip()
        body_raw = m.group(3)

        param_names = [p.strip() for p in params_str.split(',') if p.strip()]

        body_lines = []
        for line in body_raw.split('\n'):
            cleaned = line.rstrip()
            if cleaned.endswith('\\'):
                cleaned = cleaned[:-1].rstrip()
            body_lines.append(cleaned)
        body = '\n'.join(body_lines)

        all_macros[macro_name] = (param_names, body)

    # Match macros WITHOUT parameters (e.g., #define MOTHIM_SPECIES_INFO \)
    define_noparams_pattern = re.compile(
        r'#define\s+(\w+_(?:SPECIES_INFO|MISC_INFO))\s+\\\s*\n((?:.*\\\s*\n)*.*\n?)'
    )
    for m in define_noparams_pattern.finditer(content):
        macro_name = m.group(1)
        if macro_name in all_macros:
            continue  # Already matched with params
        body_raw = m.group(2)

        body_lines = []
        for line in body_raw.split('\n'):
            cleaned = line.rstrip()
            if cleaned.endswith('\\'):
                cleaned = cleaned[:-1].rstrip()
            body_lines.append(cleaned)
        body = '\n'.join(body_lines)

        all_macros[macro_name] = ([], body)  # Empty param list

    # Second pass: determine which macros ultimately contain .speciesName
    # (either directly or via nested macro calls)
    def has_species_name(macro_name, visited=None):
        if visited is None:
            visited = set()
        if macro_name in visited:
            return False
        visited.add(macro_name)
        if macro_name not in all_macros:
            return False
        _, body = all_macros[macro_name]
        if '.speciesName' in body:
            return True
        # Check if body calls another macro that has .speciesName
        for other_name in all_macros:
            if other_name != macro_name and re.search(r'\b' + re.escape(other_name) + r'\s*\(', body):
                if has_species_name(other_name, visited):
                    return True
        return False

    macros = {}
    for macro_name in all_macros:
        if has_species_name(macro_name):
            macros[macro_name] = all_macros[macro_name]

    # Also include all utility macros (non-species-level) for nested expansion
    macros['_all'] = all_macros
    return macros


def _expand_macro_body(body, param_names, macro_args):
    """Substitute macro parameters into a body text."""
    expanded = body
    # Sort params by length descending to avoid partial replacements
    # e.g., replace 'typeName' before 'type'
    substitutions = sorted(zip(param_names, macro_args), key=lambda x: -len(x[0]))
    for param, arg in substitutions:
        # Handle ## token pasting (e.g., Arceus ##typeName -> ArceusNormal)
        expanded = expanded.replace('##' + param, arg)
        expanded = expanded.replace('## ' + param, arg)
        # Replace standalone word occurrences
        expanded = re.sub(r'\b' + re.escape(param) + r'\b', arg, expanded)
    return expanded


def expand_species_macro(macro_name, macro_args, macros_dict):
    """Expand a macro invocation by substituting args into the body.
    Also expands nested macro calls. Returns the expanded text."""
    if macro_name not in macros_dict:
        return None

    param_names, body = macros_dict[macro_name]

    if len(macro_args) != len(param_names):
        return None

    expanded = _expand_macro_body(body, param_names, macro_args)

    # Use all macros (including utility macros) for nested expansion
    all_macros = macros_dict.get('_all', macros_dict)

    # Expand nested macro calls iteratively (may need multiple passes)
    for _pass in range(3):
        found_nested = False
        for nested_name, (nested_params, nested_body) in all_macros.items():
            if nested_name == macro_name or nested_name == '_all':
                continue
            nested_pattern = re.compile(r'\b' + re.escape(nested_name) + r'\s*\(([^)]*)\)')
            nm = nested_pattern.search(expanded)
            if nm:
                nested_args = [a.strip() for a in nm.group(1).split(',')]
                if len(nested_args) == len(nested_params):
                    nested_expanded = _expand_macro_body(nested_body, nested_params, nested_args)
                    expanded = expanded[:nm.start()] + nested_expanded + expanded[nm.end():]
                    found_nested = True
        if not found_nested:
            break

    return expanded


def parse_species_entries(content, gen_num, species_ids, natdex_ids,
                          move_names, ability_names, item_names_map,
                          learnset_map, shared_dex_texts):
    """Parse individual species entries from a gen file."""
    entries = []

    # Pre-expand simple (non-function-like) #define macros in the file content.
    # Only expand macros whose values are simple (braced type lists or numeric values),
    # NOT ternary expressions (those are handled by the parsers directly).
    simple_defines = re.findall(
        r'^\s*#define\s+(\w+)\s+(\{[^}]+\})$', content, re.MULTILINE
    )
    for def_name, def_body in simple_defines:
        content = re.sub(r'\b' + re.escape(def_name) + r'\b(?!\s*\()', def_body, content)

    # Build a lookup for object-like macros with ternary expressions or other values.
    # These are used to resolve macro references in type and stat fields.
    object_macros = {}
    for m in re.finditer(r'^\s*#define\s+(\w+)\s+(.+?)$', content, re.MULTILINE):
        name, value = m.group(1), m.group(2).rstrip().rstrip('\\').strip()
        if name.startswith(('P_', 'GUARD_', 'B_', 'GEN_')) or '(' not in value:
            continue
        # Skip function-like macros (detected in parse_species_macros)
        object_macros[name] = value

    # Pre-parse macros that define species info
    macros = parse_species_macros(content)
    if macros:
        print(f"    Found species macros: {', '.join(macros.keys())}")

    # Build a set of known parameterless macro names for matching
    noparams_macros = {name for name, val in macros.items() if name != '_all' and isinstance(val, tuple) and len(val[0]) == 0}

    # Split on species entry markers
    # Pattern: [SPECIES_XXX] = { (inline struct) OR [SPECIES_XXX] = MACRO_NAME(args) OR [SPECIES_XXX] = MACRO_NAME,
    entry_pattern = re.compile(r'\[(SPECIES_\w+)\]\s*=\s*(\{|(\w+?)(?:\(|[,\s]))')
    positions = []
    for m in entry_pattern.finditer(content):
        species_const = m.group(1)
        is_macro = m.group(3) is not None  # group 3 captures a potential macro name
        macro_name = m.group(3) if is_macro else None
        # Only treat as macro if the name is in our known macros dict
        if is_macro and macro_name not in macros:
            is_macro = False
            macro_name = None
        positions.append((m.start(), species_const, is_macro, macro_name))

    for i, (pos, species_const, is_macro, macro_name) in enumerate(positions):
        if species_const == 'SPECIES_NONE' or species_const == 'SPECIES_EGG':
            continue

        species_id = species_ids.get(species_const)
        if species_id is None or species_id == 0:
            continue

        # Extract the block between this entry and the next
        end_pos = positions[i + 1][0] if i + 1 < len(positions) else len(content)
        block = content[pos:end_pos]

        if is_macro and macro_name in macros:
            m_params, _ = macros[macro_name]
            if len(m_params) == 0:
                # Parameterless macro (e.g., MOTHIM_SPECIES_INFO)
                macro_args = []
            else:
                # Extract the macro arguments from the invocation line
                # Pattern: [SPECIES_XXX] = MACRO_NAME(arg1, arg2, ...)
                invocation_match = re.match(
                    r'\[' + re.escape(species_const) + r'\]\s*=\s*' + re.escape(macro_name) + r'\((.+?)\)',
                    block.split('\n')[0]
                )
                if not invocation_match:
                    # Try multi-line match
                    invocation_match = re.match(
                        r'\[' + re.escape(species_const) + r'\]\s*=\s*' + re.escape(macro_name) + r'\((.+?)\)',
                        block, re.DOTALL
                    )
                if not invocation_match:
                    continue

                args_str = invocation_match.group(1)
                macro_args = [a.strip() for a in args_str.split(',')]

            expanded = expand_species_macro(macro_name, macro_args, macros)
            if not expanded:
                continue

            # Build a synthetic block that looks like [SPECIES_XXX] = { ... }
            block = f'[{species_const}] = {expanded}'
        else:
            # Normal inline struct: find the balanced closing brace
            brace_depth = 0
            entry_end = 0
            started = False
            for j, ch in enumerate(block):
                if ch == '{':
                    brace_depth += 1
                    started = True
                elif ch == '}':
                    brace_depth -= 1
                    if started and brace_depth == 0:
                        entry_end = j + 1
                        break
            if entry_end > 0:
                block = block[:entry_end]

            # Expand any inline macro calls within the struct body
            # (e.g., Vivillon entries use VIVILLON_MISC_INFO(...) inside { })
            all_macros = macros.get('_all', macros)
            expand_pool = {**macros, **all_macros}
            for _pass in range(3):
                found = False
                for m_name, val in expand_pool.items():
                    if m_name == '_all' or not isinstance(val, tuple):
                        continue
                    m_params, m_body = val
                    inline_pat = re.compile(r'\b' + re.escape(m_name) + r'\s*\(([^)]*)\)')
                    inline_m = inline_pat.search(block)
                    if inline_m:
                        inline_args = [a.strip() for a in inline_m.group(1).split(',')]
                        if len(inline_args) == len(m_params):
                            inline_expanded = _expand_macro_body(m_body, m_params, inline_args)
                            block = block[:inline_m.start()] + inline_expanded + block[inline_m.end():]
                            found = True
                if not found:
                    break

        # Expand object-like macro references in the block (e.g., MAGNEMITE_FAMILY_TYPE2)
        # Resolve them to their values so the field parsers can handle them.
        for obj_name, obj_value in object_macros.items():
            if obj_name in block:
                # Resolve ternary to the modern (first) value
                resolved = take_first_ternary(obj_value)
                block = re.sub(r'\b' + re.escape(obj_name) + r'\b', resolved, block)

        try:
            entry = parse_single_species(block, species_const, species_id, gen_num,
                                          species_ids, natdex_ids, move_names,
                                          ability_names, item_names_map, learnset_map,
                                          shared_dex_texts)
            if entry:
                entries.append(entry)
        except Exception as e:
            print(f"  Warning: Error parsing {species_const}: {e}")

    return entries


def parse_field(block, field_name):
    """Extract a field value from a species block."""
    # Handle both simple and complex field values
    pattern = re.compile(r'\.' + re.escape(field_name) + r'\s*=\s*(.+?)(?:,\s*$|\s*,\s*(?:\.|$))',
                         re.MULTILINE)
    m = pattern.search(block)
    if m:
        return m.group(1).strip()
    return None


def parse_field_int(block, field_name, default=0):
    """Extract an integer field, handling ternary expressions."""
    val = parse_field(block, field_name)
    if val is None:
        return default
    return resolve_ternary_int(val)


def parse_compound_string(block, field_name):
    """Extract a COMPOUND_STRING or _() field value, handling multi-line strings."""
    # Find the field start
    pattern = re.compile(r'\.' + re.escape(field_name) + r'\s*=\s*', re.MULTILINE)
    m = pattern.search(block)
    if not m:
        return None

    start = m.end()
    # Determine if it's COMPOUND_STRING or _()
    rest = block[start:]

    # Find the matching macro call
    if rest.startswith('COMPOUND_STRING('):
        # Find matching closing paren
        depth = 0
        end = 0
        for j, ch in enumerate(rest):
            if ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
                if depth == 0:
                    end = j + 1
                    break
        return extract_string(rest[:end])
    elif rest.startswith('_('):
        depth = 0
        end = 0
        for j, ch in enumerate(rest):
            if ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
                if depth == 0:
                    end = j + 1
                    break
        return extract_string(rest[:end])
    else:
        # Could be a reference to a shared string variable
        m2 = re.match(r'(\w+)', rest)
        if m2:
            return None  # Can't resolve variable references easily
    return None


def parse_single_species(block, species_const, species_id, gen_num,
                          species_ids, natdex_ids, move_names,
                          ability_names, item_names_map, learnset_map,
                          shared_dex_texts=None):
    """Parse a single species entry block into a dict."""

    # Species name
    name = parse_compound_string(block, 'speciesName')
    if not name:
        return None

    # Types
    types = []
    types_match = re.search(r'\.types\s*=\s*MON_TYPES\((\w+)(?:\s*,\s*(\w+))?\)', block)
    if types_match:
        t1 = types_match.group(1)
        t2 = types_match.group(2) if types_match.group(2) else t1
        type1_name = format_constant('TYPE_', t1)
        type2_name = format_constant('TYPE_', t2)
        types.append(type1_name)
        if type2_name != type1_name:
            types.append(type2_name)
    else:
        # Try { TYPE_X, TYPE_Y } style (used by macros like ROTOM_FAMILY_TYPES)
        brace_match = re.search(r'\.types\s*=\s*\{\s*(TYPE_\w+)\s*,\s*(TYPE_\w+)\s*\}', block)
        if brace_match:
            t1 = format_constant('TYPE_', brace_match.group(1))
            t2 = format_constant('TYPE_', brace_match.group(2))
            types.append(t1)
            if t2 != t1:
                types.append(t2)
        else:
            # Try resolving a macro reference: .types = SOME_MACRO,
            macro_type_match = re.search(r'\.types\s*=\s*(\w+)', block)
            if macro_type_match:
                macro_ref = macro_type_match.group(1)
                # Look for a #define for this macro in the same block context
                # This is handled by pre-expanding macros in the block, but as fallback
                # try to find TYPE_ constants in the value
                type_consts = re.findall(r'TYPE_(\w+)', macro_ref)
                for tc in type_consts:
                    types.append(format_constant('TYPE_', 'TYPE_' + tc))

    # Base stats
    base_stats = {
        'hp': parse_field_int(block, 'baseHP'),
        'attack': parse_field_int(block, 'baseAttack'),
        'defense': parse_field_int(block, 'baseDefense'),
        'spAttack': parse_field_int(block, 'baseSpAttack'),
        'spDefense': parse_field_int(block, 'baseSpDefense'),
        'speed': parse_field_int(block, 'baseSpeed'),
    }

    # Abilities
    abilities_match = re.search(
        r'\.abilities\s*=\s*\{\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\}', block
    )
    abilities = [None, None, None]
    if abilities_match:
        for idx in range(3):
            ab_const = abilities_match.group(idx + 1)
            if ab_const == 'ABILITY_NONE':
                abilities[idx] = None
            else:
                abilities[idx] = ability_names.get(ab_const, format_constant('ABILITY_', ab_const))

    # EV yield
    ev_yield = {
        'hp': parse_field_int(block, 'evYield_HP'),
        'attack': parse_field_int(block, 'evYield_Attack'),
        'defense': parse_field_int(block, 'evYield_Defense'),
        'spAttack': parse_field_int(block, 'evYield_SpAttack'),
        'spDefense': parse_field_int(block, 'evYield_SpDefense'),
        'speed': parse_field_int(block, 'evYield_Speed'),
    }

    # Catch rate
    catch_rate = parse_field_int(block, 'catchRate')

    # Gender ratio
    gender_ratio = 0
    gr_match = re.search(r'\.genderRatio\s*=\s*(.+?)(?:,\s*$|\s*,\s*\.)', block, re.MULTILINE)
    if gr_match:
        gr_val = gr_match.group(1).strip()
        if 'PERCENT_FEMALE' in gr_val:
            pf = re.search(r'PERCENT_FEMALE\(\s*([\d.]+)\s*\)', gr_val)
            if pf:
                gender_ratio = float(pf.group(1))
        elif gr_val == 'MON_MALE':
            gender_ratio = 0
        elif gr_val == 'MON_FEMALE':
            gender_ratio = 100
        elif gr_val == 'MON_GENDERLESS':
            gender_ratio = -1
        else:
            gender_ratio = 0

    # Egg cycles
    egg_cycles = parse_field_int(block, 'eggCycles')

    # Friendship
    friendship_match = re.search(r'\.friendship\s*=\s*(.+?)(?:,\s*$|\s*,\s*\.)', block, re.MULTILINE)
    friendship = 50  # STANDARD_FRIENDSHIP default
    if friendship_match:
        fv = friendship_match.group(1).strip()
        if fv == 'STANDARD_FRIENDSHIP':
            friendship = 50
        else:
            try:
                friendship = int(fv)
            except ValueError:
                friendship = 50

    # Growth rate
    growth_rate_match = re.search(r'\.growthRate\s*=\s*(\w+)', block)
    growth_rate = "Medium Fast"
    if growth_rate_match:
        growth_rate = format_constant('GROWTH_', growth_rate_match.group(1))

    # Egg groups
    egg_groups = []
    eg_match = re.search(r'\.eggGroups\s*=\s*MON_EGG_GROUPS\((\w+)(?:\s*,\s*(\w+))?\)', block)
    if eg_match:
        egg_groups.append(format_constant('EGG_GROUP_', eg_match.group(1)))
        if eg_match.group(2):
            eg2 = format_constant('EGG_GROUP_', eg_match.group(2))
            if eg2 != egg_groups[0]:
                egg_groups.append(eg2)

    # Height, weight
    height = parse_field_int(block, 'height')
    weight = parse_field_int(block, 'weight')

    # Category
    category = parse_compound_string(block, 'categoryName')
    if not category:
        category = ""

    # Description - try inline string first, then shared dex text variable
    description = parse_compound_string(block, 'description')
    if not description:
        # Try to resolve a variable reference
        desc_var_match = re.search(r'\.description\s*=\s*(\w+)', block)
        if desc_var_match and shared_dex_texts:
            var_name = desc_var_match.group(1)
            description = shared_dex_texts.get(var_name, "")
        else:
            description = ""

    # National Dex Number
    natdex_match = re.search(r'\.natDexNum\s*=\s*(\w+)', block)
    nat_dex_num = 0
    if natdex_match:
        nat_dex_const = natdex_match.group(1)
        nat_dex_num = natdex_ids.get(nat_dex_const, 0)

    # Evolutions
    evolutions = parse_evolutions(block, species_ids, item_names_map)

    # Learnset references - map variable names to all species that use them
    lu_match = re.search(r'\.levelUpLearnset\s*=\s*(\w+)', block)
    if lu_match:
        learnset_map[lu_match.group(1)].append(('levelUp', species_id))

    te_match = re.search(r'\.teachableLearnset\s*=\s*(\w+)', block)
    if te_match:
        learnset_map[te_match.group(1)].append(('teachable', species_id))

    em_match = re.search(r'\.eggMoveLearnset\s*=\s*(\w+)', block)
    if em_match:
        learnset_map[em_match.group(1)].append(('egg', species_id))

    # Sprite ID
    sprite_id = species_const_to_sprite_id(species_const)

    entry = {
        'id': species_id,
        'name': name,
        'types': types,
        'baseStats': base_stats,
        'abilities': abilities,
        'evYield': ev_yield,
        'catchRate': catch_rate,
        'genderRatio': gender_ratio,
        'eggCycles': egg_cycles,
        'friendship': friendship,
        'growthRate': growth_rate,
        'eggGroups': egg_groups,
        'height': height,
        'weight': weight,
        'category': category,
        'description': description,
        'generation': gen_num,
        'natDexNum': nat_dex_num,
        'evolutions': evolutions,
        'levelUpMoves': [],
        'teachableMoves': [],
        'eggMoves': [],
        'spriteId': sprite_id,
        'encounters': [],
    }

    return entry


def parse_evolutions(block, species_ids, item_names_map):
    """Parse the evolutions field from a species block."""
    evolutions = []

    # Find the .evolutions = EVOLUTION(...) block
    evo_match = re.search(r'\.evolutions\s*=\s*EVOLUTION\(', block)
    if not evo_match:
        return evolutions

    start = evo_match.end()
    # Find matching closing paren
    depth = 1
    end = start
    for j in range(start, len(block)):
        if block[j] == '(':
            depth += 1
        elif block[j] == ')':
            depth -= 1
            if depth == 0:
                end = j
                break

    evo_content = block[start:end]

    # Parse individual evolution entries: {EVO_XXX, param, SPECIES_XXX, ...}
    evo_entries = re.findall(r'\{([^}]+)\}', evo_content)
    for entry_str in evo_entries:
        parts = [p.strip() for p in entry_str.split(',')]
        if len(parts) < 3:
            continue

        method_const = parts[0].strip()
        param_raw = parts[1].strip()
        target_const = parts[2].strip()

        # Skip CONDITIONS entries
        if method_const.startswith('IF_') or method_const == 'CONDITIONS_END':
            continue

        # Method name
        method = format_evo_method(method_const)
        if method is None:
            continue

        # Parameter
        param = format_evo_param(method_const, param_raw, item_names_map)

        # Target
        target_id = species_ids.get(target_const, 0)
        # We'll fill in target name later once all species are parsed
        evolutions.append({
            'method': method,
            'param': param,
            'targetId': target_id,
            'targetName': target_const,  # Placeholder, filled in later
        })

    return evolutions


def format_evo_method(method_const):
    """Convert EVO_XXX to a human readable method name."""
    methods = {
        'EVO_LEVEL': 'Level',
        'EVO_TRADE': 'Trade',
        'EVO_ITEM': 'Item',
        'EVO_SPLIT_FROM_EVO': 'Split',
        'EVO_SCRIPT_TRIGGER': 'Script',
        'EVO_LEVEL_BATTLE_ONLY': 'Level (Battle)',
        'EVO_BATTLE_END': 'Battle End',
        'EVO_SPIN': 'Spin',
        'EVO_NONE': None,
    }
    return methods.get(method_const, format_constant('EVO_', method_const))


def format_evo_param(method_const, param_raw, item_names_map):
    """Format an evolution parameter based on the method."""
    if method_const == 'EVO_ITEM':
        return item_names_map.get(param_raw, format_item_name(param_raw))
    elif method_const == 'EVO_LEVEL' or method_const == 'EVO_LEVEL_BATTLE_ONLY':
        try:
            return str(int(param_raw))
        except ValueError:
            return param_raw
    elif method_const == 'EVO_TRADE':
        return ""
    else:
        return param_raw


def species_const_to_sprite_id(species_const):
    """Convert SPECIES_XXX to a Showdown-compatible sprite ID.
    Showdown sprite IDs strip all non-alphanumeric from base names,
    but use hyphens for form suffixes."""
    name = species_const.replace('SPECIES_', '').lower()

    # Special cases for Nidoran
    if name == 'nidoran_f':
        return 'nidoranf'
    if name == 'nidoran_m':
        return 'nidoranm'

    # Replace underscores with hyphens
    name = name.replace('_', '-')
    return name


def name_to_showdown_id(display_name):
    """Convert a Pokemon display name to Showdown's sprite ID format.
    Showdown strips all non-alphanumeric characters and lowercases.
    E.g., 'Ho-Oh' -> 'hooh', 'Mr. Mime' -> 'mrmime', 'Flabébé' -> 'flabebe'"""
    import unicodedata
    # Normalize accented characters (é -> e, etc.)
    name = unicodedata.normalize('NFKD', display_name)
    name = ''.join(c for c in name if not unicodedata.combining(c))
    # Handle gender symbols
    name = name.replace('♀', 'f').replace('♂', 'm')
    return re.sub(r'[^a-z0-9]', '', name.lower())


# ---------------------------------------------------------------------------
# Learnset Parsers
# ---------------------------------------------------------------------------

def parse_level_up_learnsets(repo_root, move_names):
    """Parse all level-up learnset files, returning {variable_name: [(level, move_name)]}."""
    learnsets = {}
    learnset_dir = os.path.join(repo_root, 'src', 'data', 'pokemon', 'level_up_learnsets')

    for gen_num in range(1, 10):
        filepath = os.path.join(learnset_dir, f'gen_{gen_num}.h')
        if not os.path.isfile(filepath):
            continue

        print(f"  Parsing level_up_learnsets/gen_{gen_num}.h...")
        with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()

        # Find each learnset array
        pattern = re.compile(
            r'static\s+const\s+struct\s+LevelUpMove\s+(\w+)\[\]\s*=\s*\{(.*?)\};',
            re.DOTALL
        )
        for m in pattern.finditer(content):
            var_name = m.group(1)
            body = m.group(2)
            moves = []
            for lm in re.finditer(r'LEVEL_UP_MOVE\(\s*(\d+)\s*,\s*(\w+)\s*\)', body):
                level = int(lm.group(1))
                move_const = lm.group(2)
                move_name = move_names.get(move_const, format_constant('MOVE_', move_const))
                moves.append({'level': level, 'move': move_name})
            learnsets[var_name] = moves

    return learnsets


def parse_egg_move_learnsets(repo_root, move_names):
    """Parse egg move learnsets, returning {variable_name: [move_name]}."""
    learnsets = {}
    filepath = os.path.join(repo_root, 'src', 'data', 'pokemon', 'egg_moves.h')
    if not os.path.isfile(filepath):
        print("  Warning: egg_moves.h not found")
        return learnsets

    print("  Parsing egg_moves.h...")
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    pattern = re.compile(
        r'static\s+const\s+u16\s+(\w+EggMoveLearnset)\[\]\s*=\s*\{(.*?)\};',
        re.DOTALL
    )
    for m in pattern.finditer(content):
        var_name = m.group(1)
        body = m.group(2)
        moves = []
        for move_match in re.finditer(r'\b(MOVE_\w+)\b', body):
            move_const = move_match.group(1)
            if move_const == 'MOVE_UNAVAILABLE':
                continue
            move_name = move_names.get(move_const, format_constant('MOVE_', move_const))
            moves.append(move_name)
        learnsets[var_name] = moves

    return learnsets


def parse_teachable_learnsets(repo_root, move_names):
    """Parse teachable move learnsets, returning {variable_name: [move_name]}."""
    learnsets = {}
    filepath = os.path.join(repo_root, 'src', 'data', 'pokemon', 'teachable_learnsets.h')
    if not os.path.isfile(filepath):
        print("  Warning: teachable_learnsets.h not found")
        return learnsets

    print("  Parsing teachable_learnsets.h...")
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    pattern = re.compile(
        r'static\s+const\s+u16\s+(\w+TeachableLearnset)\[\]\s*=\s*\{(.*?)\};',
        re.DOTALL
    )
    for m in pattern.finditer(content):
        var_name = m.group(1)
        body = m.group(2)
        moves = []
        for move_match in re.finditer(r'\b(MOVE_\w+)\b', body):
            move_const = move_match.group(1)
            if move_const == 'MOVE_UNAVAILABLE':
                continue
            move_name = move_names.get(move_const, format_constant('MOVE_', move_const))
            moves.append(move_name)
        learnsets[var_name] = moves

    return learnsets


# ---------------------------------------------------------------------------
# Move Parser
# ---------------------------------------------------------------------------

def parse_moves_info(repo_root, move_ids):
    """Parse moves_info.h and return list of move dicts."""
    filepath = os.path.join(repo_root, 'src', 'data', 'moves_info.h')
    print("Parsing moves_info.h...")

    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    moves = []
    move_names_map = {}  # MOVE_XXX -> display name

    # Find each move entry: [MOVE_XXX] = {
    entry_pattern = re.compile(r'\[(MOVE_\w+)\]\s*=\s*\{')
    positions = [(m.start(), m.group(1)) for m in entry_pattern.finditer(content)]

    for i, (pos, move_const) in enumerate(positions):
        if move_const == 'MOVE_NONE':
            continue

        move_id = move_ids.get(move_const)
        if move_id is None or move_id == 0:
            continue

        end_pos = positions[i + 1][0] if i + 1 < len(positions) else len(content)
        block = content[pos:end_pos]

        # Find closing brace
        brace_depth = 0
        entry_end = 0
        started = False
        for j, ch in enumerate(block):
            if ch == '{':
                brace_depth += 1
                started = True
            elif ch == '}':
                brace_depth -= 1
                if started and brace_depth == 0:
                    entry_end = j + 1
                    break
        if entry_end > 0:
            block = block[:entry_end]

        try:
            name = parse_compound_string(block, 'name')
            if not name:
                continue

            description = parse_compound_string(block, 'description')
            if not description:
                description = ""

            # Power
            power_val = parse_field(block, 'power')
            power = resolve_ternary_int(power_val) if power_val else 0

            # Type
            type_val = parse_field(block, 'type')
            if type_val:
                type_val = take_first_ternary(type_val)
                type_name = format_constant('TYPE_', type_val)
            else:
                type_name = "Normal"

            # Accuracy
            accuracy_val = parse_field(block, 'accuracy')
            accuracy = resolve_ternary_int(accuracy_val) if accuracy_val else 0

            # PP
            pp_val = parse_field(block, 'pp')
            pp = resolve_ternary_int(pp_val) if pp_val else 0

            # Priority
            priority_val = parse_field(block, 'priority')
            priority = resolve_ternary_int(priority_val) if priority_val else 0

            # Category
            cat_val = parse_field(block, 'category')
            if cat_val:
                cat_val = take_first_ternary(cat_val)
                if 'PHYSICAL' in cat_val:
                    category = "Physical"
                elif 'SPECIAL' in cat_val:
                    category = "Special"
                elif 'STATUS' in cat_val:
                    category = "Status"
                else:
                    category = "Physical"
            else:
                category = "Physical"

            # Makes contact
            makes_contact = False
            mc_val = parse_field(block, 'makesContact')
            if mc_val and 'TRUE' in mc_val.upper():
                makes_contact = True

            move_entry = {
                'id': move_id,
                'name': name,
                'type': type_name,
                'category': category,
                'power': power,
                'accuracy': accuracy,
                'pp': pp,
                'priority': priority,
                'description': description,
                'makesContact': makes_contact,
            }
            moves.append(move_entry)
            move_names_map[move_const] = name

        except Exception as e:
            print(f"  Warning: Error parsing move {move_const}: {e}")

    print(f"  Parsed {len(moves)} moves")
    return moves, move_names_map


# ---------------------------------------------------------------------------
# Ability Parser
# ---------------------------------------------------------------------------

def parse_abilities_info(repo_root, ability_ids):
    """Parse abilities.h and return list of ability dicts."""
    filepath = os.path.join(repo_root, 'src', 'data', 'abilities.h')
    print("Parsing abilities.h...")

    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    abilities = []
    ability_names_map = {}  # ABILITY_XXX -> display name

    entry_pattern = re.compile(r'\[(ABILITY_\w+)\]\s*=\s*\{')
    positions = [(m.start(), m.group(1)) for m in entry_pattern.finditer(content)]

    for i, (pos, ability_const) in enumerate(positions):
        if ability_const == 'ABILITY_NONE':
            continue

        ability_id = ability_ids.get(ability_const)
        if ability_id is None or ability_id == 0:
            continue

        end_pos = positions[i + 1][0] if i + 1 < len(positions) else len(content)
        block = content[pos:end_pos]

        brace_depth = 0
        entry_end = 0
        started = False
        for j, ch in enumerate(block):
            if ch == '{':
                brace_depth += 1
                started = True
            elif ch == '}':
                brace_depth -= 1
                if started and brace_depth == 0:
                    entry_end = j + 1
                    break
        if entry_end > 0:
            block = block[:entry_end]

        try:
            name = parse_compound_string(block, 'name')
            if not name:
                name = extract_string(block)
                if not name:
                    continue

            description = parse_compound_string(block, 'description')
            if not description:
                description = ""

            abilities.append({
                'id': ability_id,
                'name': name,
                'description': description,
            })
            ability_names_map[ability_const] = name

        except Exception as e:
            print(f"  Warning: Error parsing ability {ability_const}: {e}")

    print(f"  Parsed {len(abilities)} abilities")
    return abilities, ability_names_map


# ---------------------------------------------------------------------------
# Type Parser
# ---------------------------------------------------------------------------

def parse_types_info(repo_root):
    """Parse types_info.h for type names and effectiveness matrix."""
    filepath = os.path.join(repo_root, 'src', 'data', 'types_info.h')
    print("Parsing types_info.h...")

    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    # Type ordering from pokemon.h
    type_order = [
        'TYPE_NONE', 'TYPE_NORMAL', 'TYPE_FIGHTING', 'TYPE_FLYING',
        'TYPE_POISON', 'TYPE_GROUND', 'TYPE_ROCK', 'TYPE_BUG',
        'TYPE_GHOST', 'TYPE_STEEL', 'TYPE_MYSTERY', 'TYPE_FIRE',
        'TYPE_WATER', 'TYPE_GRASS', 'TYPE_ELECTRIC', 'TYPE_PSYCHIC',
        'TYPE_ICE', 'TYPE_DRAGON', 'TYPE_DARK', 'TYPE_FAIRY',
        'TYPE_STELLAR'
    ]

    # Types to exclude
    excluded = {'TYPE_NONE', 'TYPE_MYSTERY', 'TYPE_STELLAR'}

    # Parse type names from gTypesInfo
    type_names = {}
    for type_const in type_order:
        pattern = re.compile(
            r'\[' + re.escape(type_const) + r'\]\s*=\s*\{[^}]*?\.name\s*=\s*_\("([^"]+)"\)',
            re.DOTALL
        )
        m = pattern.search(content)
        if m:
            type_names[type_const] = m.group(1)
        else:
            type_names[type_const] = format_constant('TYPE_', type_const)

    # Parse effectiveness matrix
    # Resolve conditional macros to their modern (first/true) values
    macro_values = {
        'STL_RS': 1.0,    # Ghost/Dark -> Steel: X(1.0) for GEN_6+
        'PSN_RS': 0.5,    # Bug -> Poison: X(0.5) for GEN_2+
        'BUG_RS': 1.0,    # Poison -> Bug: X(1.0) for GEN_2+
        'PSY_RS': 2.0,    # Ghost -> Psychic: X(2.0) for GEN_2+
        'FIR_RS': 0.5,    # Ice -> Fire: X(0.5) for GEN_2+
    }

    # Parse each row of the matrix
    effectiveness = {}
    for type_const in type_order:
        row_pattern = re.compile(
            r'\[' + re.escape(type_const) + r'\]\s*=\s*\{([^}]+)\}',
        )
        # Search only in the effectiveness table section
        table_match = re.search(r'gTypeEffectivenessTable.*?\{(.*?)#undef', content, re.DOTALL)
        if table_match:
            table_content = table_match.group(1)
            m = row_pattern.search(table_content)
            if m:
                row_str = m.group(1)
                values = parse_effectiveness_row(row_str, macro_values)
                effectiveness[type_const] = values

    # Build output: only include non-excluded types
    filtered_types = [t for t in type_order if t not in excluded]
    filtered_indices = [type_order.index(t) for t in filtered_types]

    type_names_list = [type_names.get(t, format_constant('TYPE_', t)) for t in filtered_types]
    effectiveness_matrix = []
    for t in filtered_types:
        row = effectiveness.get(t, [1.0] * len(type_order))
        filtered_row = [row[i] if i < len(row) else 1.0 for i in filtered_indices]
        effectiveness_matrix.append(filtered_row)

    print(f"  Parsed {len(type_names_list)} types")
    return {
        'types': type_names_list,
        'effectiveness': effectiveness_matrix,
    }


def parse_effectiveness_row(row_str, macro_values):
    """Parse a row of the effectiveness table."""
    values = []
    # Split by commas, handle each entry
    entries = row_str.split(',')
    for entry in entries:
        entry = entry.strip()
        if not entry:
            continue
        # Handle X(value)
        x_match = re.match(r'X\(\s*([\d.]+)\s*\)', entry)
        if x_match:
            values.append(float(x_match.group(1)))
        elif entry == '______':
            values.append(1.0)
        elif entry in macro_values:
            values.append(macro_values[entry])
        elif entry.startswith('//') or entry.startswith('/*'):
            continue
        else:
            # Unknown, default to 1.0
            values.append(1.0)
    return values


# ---------------------------------------------------------------------------
# Encounter Parser
# ---------------------------------------------------------------------------

def prettify_map_name(map_name):
    """Convert MAP_ROUTE101 -> 'Route 101', MAP_PETALBURG_CITY -> 'Petalburg City'."""
    name = map_name.replace('MAP_', '')

    # Insert space before numbers that follow letters
    name = re.sub(r'([A-Za-z])(\d)', r'\1 \2', name)

    # Replace underscores with spaces and title case
    name = name.replace('_', ' ').title()

    return name


def parse_encounters(repo_root, species_ids):
    """Parse wild_encounters.json and build per-species and per-location encounter lists."""
    filepath = os.path.join(repo_root, 'src', 'data', 'wild_encounters.json')
    print("Parsing wild_encounters.json...")

    with open(filepath, 'r', encoding='utf-8') as f:
        data = json.load(f)

    species_encounters = defaultdict(list)  # species_id -> [encounter]
    location_encounters = defaultdict(list)  # location_name -> [pokemon]

    method_map = {
        'land_mons': 'grass',
        'water_mons': 'surfing',
        'fishing_mons': 'fishing',
        'rock_smash_mons': 'rock smash',
        'hidden_mons': 'hidden',
    }

    for group in data.get('wild_encounter_groups', []):
        for encounter in group.get('encounters', []):
            map_name = encounter.get('map', '')
            if not map_name:
                # Skip entries with no map (e.g. Battle Frontier)
                continue
            location = prettify_map_name(map_name)

            for method_key, method_label in method_map.items():
                mons_data = encounter.get(method_key)
                if not mons_data:
                    continue

                mons = mons_data.get('mons', [])
                # Deduplicate and merge level ranges per species in this location/method
                species_level_ranges = defaultdict(lambda: [999, 0])
                for mon in mons:
                    sp = mon.get('species', '')
                    min_lv = mon.get('min_level', 0)
                    max_lv = mon.get('max_level', 0)
                    if sp:
                        slr = species_level_ranges[sp]
                        slr[0] = min(slr[0], min_lv)
                        slr[1] = max(slr[1], max_lv)

                for sp, (min_lv, max_lv) in species_level_ranges.items():
                    sp_id = species_ids.get(sp, 0)
                    sp_name = format_constant('SPECIES_', sp)

                    enc_entry = {
                        'location': location,
                        'method': method_label,
                        'minLevel': min_lv,
                        'maxLevel': max_lv,
                    }
                    species_encounters[sp_id].append(enc_entry)

                    loc_entry = {
                        'species': sp_name,
                        'speciesId': sp_id,
                        'method': method_label,
                        'minLevel': min_lv,
                        'maxLevel': max_lv,
                    }
                    location_encounters[location].append(loc_entry)

    # Build location encounters list
    location_list = []
    for loc_name in sorted(location_encounters.keys()):
        location_list.append({
            'location': loc_name,
            'pokemon': location_encounters[loc_name],
        })

    print(f"  Found encounters across {len(location_list)} locations")
    return species_encounters, location_list


# ---------------------------------------------------------------------------
# Form Deduplication
# ---------------------------------------------------------------------------

def deduplicate_forms(pokemon_list):
    """Deduplicate alternate forms: keep distinct forms (different types or abilities),
    remove cosmetic-only forms (same types and abilities as base)."""
    from collections import defaultdict

    print("Deduplicating forms...")
    count_before = len(pokemon_list)

    # Group by natDexNum
    groups = defaultdict(list)
    for pkmn in pokemon_list:
        ndex = pkmn.get('natDexNum', 0)
        if ndex > 0:
            groups[ndex].append(pkmn)

    # Set of IDs to remove (cosmetic forms)
    ids_to_remove = set()

    for ndex, group in groups.items():
        if len(group) <= 1:
            continue

        # Sort by id so the lowest id is the base form
        group.sort(key=lambda p: p['id'])
        base = group[0]
        base_types = tuple(base.get('types', []))
        base_abilities = tuple(base.get('abilities', []))
        base_stats = tuple(sorted(base.get('baseStats', {}).items()))

        for form in group[1:]:
            form_types = tuple(form.get('types', []))
            form_abilities = tuple(form.get('abilities', []))
            form_stats = tuple(sorted(form.get('baseStats', {}).items()))

            if form_types != base_types or form_abilities != base_abilities or form_stats != base_stats:
                # Distinct form - keep it but add form suffix to name
                form_suffix = _get_form_suffix(form['spriteId'], base['spriteId'])
                if form_suffix and form_suffix.lower() != base['name'].lower():
                    form['name'] = f"{base['name']}-{form_suffix}"
                # Generate Showdown sprite ID: base name (stripped) + hyphen + form suffix (stripped)
                base_showdown = name_to_showdown_id(base['name'])
                form_suffix_id = name_to_showdown_id(form_suffix) if form_suffix else ''
                form['spriteId'] = f"{base_showdown}-{form_suffix_id}" if form_suffix_id else base_showdown
            else:
                # Cosmetic form - remove it
                ids_to_remove.add(form['id'])

        # For base forms, generate sprite ID from the display name
        # This correctly handles names like Ho-Oh -> "hooh", Mr. Mime -> "mrmime"
        base['spriteId'] = name_to_showdown_id(base['name'])

    # Filter out removed forms
    filtered = [p for p in pokemon_list if p['id'] not in ids_to_remove]

    # Regenerate sprite IDs for single-entry Pokemon (not handled above)
    multi_entry_ids = set()
    for ndex, group in groups.items():
        if len(group) > 1:
            for p in group:
                multi_entry_ids.add(p['id'])
    for p in filtered:
        if p['id'] not in multi_entry_ids:
            p['spriteId'] = name_to_showdown_id(p['name'])

    count_after = len(filtered)
    print(f"  Before: {count_before}, After: {count_after} (removed {count_before - count_after} cosmetic forms)")

    return filtered


def _get_form_suffix(form_sprite_id, base_sprite_id):
    """Derive a display-friendly form suffix from the sprite ID.
    E.g., form_sprite_id='wormadam-sandy', base_sprite_id='wormadam-plant' -> 'Sandy'
    E.g., form_sprite_id='rotom-heat', base_sprite_id='rotom' -> 'Heat'
    """
    if '-' not in form_sprite_id:
        return None

    # Get the part after the base name
    # The base name is the part of the spriteId before the first hyphen
    # (or the entire base spriteId if it has no hyphen)
    base_root = base_sprite_id.split('-')[0]
    form_root = form_sprite_id.split('-')[0]

    if base_root == form_root and '-' in form_sprite_id:
        suffix = form_sprite_id[len(base_root) + 1:]  # everything after "basename-"
        # Title-case each part separated by hyphens
        return '-'.join(part.title() for part in suffix.split('-'))

    return None


# ---------------------------------------------------------------------------
# Main orchestration
# ---------------------------------------------------------------------------

def main():
    repo_root = find_repo_root(sys.argv[1] if len(sys.argv) > 1 else None)
    output_dir = os.path.join(repo_root, 'web', 'public', 'data')
    os.makedirs(output_dir, exist_ok=True)

    print(f"Repo root: {repo_root}")
    print(f"Output dir: {output_dir}")
    print()

    # Step 1: Parse constant IDs from headers
    print("Parsing constant IDs...")
    species_ids = parse_enum_constants(
        os.path.join(repo_root, 'include', 'constants', 'species.h'), 'SPECIES_'
    )
    print(f"  Found {len(species_ids)} species constants")

    move_ids = parse_enum_constants(
        os.path.join(repo_root, 'include', 'constants', 'moves.h'), 'MOVE_'
    )
    print(f"  Found {len(move_ids)} move constants")

    ability_ids = parse_enum_constants(
        os.path.join(repo_root, 'include', 'constants', 'abilities.h'), 'ABILITY_'
    )
    print(f"  Found {len(ability_ids)} ability constants")

    natdex_ids = parse_pokedex_constants(
        os.path.join(repo_root, 'include', 'constants', 'pokedex.h')
    )
    print(f"  Found {len(natdex_ids)} national dex constants")
    print()

    # Step 2: Parse item names for evolution params
    print("Parsing item names...")
    item_names_map = parse_item_names(os.path.join(repo_root, 'src', 'data', 'items.h'))
    print(f"  Found {len(item_names_map)} item names")
    print()

    # Step 3: Parse abilities (needed for species names)
    abilities_list, ability_names = parse_abilities_info(repo_root, ability_ids)
    print()

    # Step 4: Parse moves (needed for learnset names)
    moves_list, move_names = parse_moves_info(repo_root, move_ids)
    print()

    # Step 5: Parse species info
    print("Parsing species info...")
    pokemon_list, learnset_map = parse_species_info(
        repo_root, species_ids, natdex_ids, move_names, ability_names, item_names_map
    )
    print(f"  Parsed {len(pokemon_list)} Pokemon")
    print()

    # Step 6: Parse learnsets
    print("Parsing learnsets...")
    level_up_learnsets = parse_level_up_learnsets(repo_root, move_names)
    egg_learnsets = parse_egg_move_learnsets(repo_root, move_names)
    teachable_learnsets = parse_teachable_learnsets(repo_root, move_names)
    print(f"  Found {len(level_up_learnsets)} level-up, {len(egg_learnsets)} egg, {len(teachable_learnsets)} teachable learnsets")
    print()

    # Step 7: Attach learnsets to Pokemon
    print("Attaching learnsets to Pokemon...")
    pokemon_by_id = {p['id']: p for p in pokemon_list}

    for var_name, mappings in learnset_map.items():
        for learnset_type, species_id in mappings:
            if species_id not in pokemon_by_id:
                continue
            pkmn = pokemon_by_id[species_id]
            if learnset_type == 'levelUp' and var_name in level_up_learnsets:
                pkmn['levelUpMoves'] = level_up_learnsets[var_name]
            elif learnset_type == 'egg' and var_name in egg_learnsets:
                pkmn['eggMoves'] = egg_learnsets[var_name]
            elif learnset_type == 'teachable' and var_name in teachable_learnsets:
                pkmn['teachableMoves'] = teachable_learnsets[var_name]

    # Step 8: Resolve evolution target names
    print("Resolving evolution target names...")
    for pkmn in pokemon_list:
        for evo in pkmn.get('evolutions', []):
            target_id = evo.get('targetId', 0)
            if target_id in pokemon_by_id:
                evo['targetName'] = pokemon_by_id[target_id]['name']
            else:
                # Try to clean up the constant name
                evo['targetName'] = format_constant('SPECIES_', evo.get('targetName', ''))

    # Step 9: Parse encounters
    species_encounters, location_encounters = parse_encounters(repo_root, species_ids)

    # Attach encounters to Pokemon
    for pkmn in pokemon_list:
        pkmn['encounters'] = species_encounters.get(pkmn['id'], [])

    # Step 9.5: Deduplicate forms
    print()
    pokemon_list = deduplicate_forms(pokemon_list)

    # Step 10: Parse types
    print()
    types_data = parse_types_info(repo_root)

    # Step 11: Sort and write output
    print()
    print("Writing output files...")

    # Sort pokemon by ID
    pokemon_list.sort(key=lambda p: p['id'])

    # Remove friendship from output (not in spec)
    for pkmn in pokemon_list:
        pkmn.pop('friendship', None)

    # Write pokemon.json
    pokemon_path = os.path.join(output_dir, 'pokemon.json')
    with open(pokemon_path, 'w', encoding='utf-8') as f:
        json.dump(pokemon_list, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {pokemon_path} ({len(pokemon_list)} Pokemon)")

    # Sort moves by ID
    moves_list.sort(key=lambda m: m['id'])

    # Write moves.json
    moves_path = os.path.join(output_dir, 'moves.json')
    with open(moves_path, 'w', encoding='utf-8') as f:
        json.dump(moves_list, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {moves_path} ({len(moves_list)} moves)")

    # Sort abilities by ID
    abilities_list.sort(key=lambda a: a['id'])

    # Write abilities.json
    abilities_path = os.path.join(output_dir, 'abilities.json')
    with open(abilities_path, 'w', encoding='utf-8') as f:
        json.dump(abilities_list, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {abilities_path} ({len(abilities_list)} abilities)")

    # Write types.json
    types_path = os.path.join(output_dir, 'types.json')
    with open(types_path, 'w', encoding='utf-8') as f:
        json.dump(types_data, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {types_path}")

    # Write encounters.json
    encounters_path = os.path.join(output_dir, 'encounters.json')
    with open(encounters_path, 'w', encoding='utf-8') as f:
        json.dump(location_encounters, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {encounters_path} ({len(location_encounters)} locations)")

    print()
    print("Done!")


if __name__ == '__main__':
    main()
