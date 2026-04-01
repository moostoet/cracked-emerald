#!/usr/bin/env python3
"""
Extract game data from Pokemon ROM hack C source files and output JSON
for the Pokedex web app.

Usage: python3 extract_data.py [repo_root]
"""

import json
import os
import re
import struct
import sys
from collections import defaultdict, deque


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
    all_values = {}  # Track all enum values for alias resolution
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

        # Match any enum entry with explicit value: NAME = VALUE,
        m = re.match(r'(\w+)\s*=\s*(.+?)\s*,?\s*(?://.*)?$', line)
        if m:
            name = m.group(1)
            val_str = m.group(2).strip()
            # Try as integer
            try:
                val = int(val_str)
                all_values[name] = val
                if name.startswith(prefix):
                    constants[name] = val
                current_val = val + 1
            except ValueError:
                # Reference to another constant — save as alias
                aliases[name] = val_str
                continue
        else:
            # Auto-incrementing: NAME,
            am = re.match(r'(\w+)\s*,', line)
            if am:
                name = am.group(1)
                all_values[name] = current_val
                if name.startswith(prefix):
                    constants[name] = current_val
                current_val += 1

    # Resolve aliases using all enum values (not just prefixed ones)
    for alias_name, target_name in aliases.items():
        target_name = target_name.strip()
        resolved = all_values.get(target_name) or constants.get(target_name)
        if resolved is not None and alias_name.startswith(prefix):
            constants[alias_name] = resolved

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

    # Held items
    held_items = []
    item_common_match = re.search(r'\.itemCommon\s*=\s*(\w+)', block)
    item_rare_match = re.search(r'\.itemRare\s*=\s*(\w+)', block)
    if item_common_match:
        ic = item_common_match.group(1)
        if ic != 'ITEM_NONE':
            held_items.append(item_names_map.get(ic, format_item_name(ic)))
    if item_rare_match:
        ir = item_rare_match.group(1)
        if ir != 'ITEM_NONE':
            item_name = item_names_map.get(ir, format_item_name(ir))
            if item_name not in held_items:
                held_items.append(item_name)

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
        'heldItems': held_items,
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

    # Parse individual evolution entries, handling nested braces from CONDITIONS()
    # Each top-level { } is one evolution entry
    entries = []
    depth = 0
    current = []
    for ch in evo_content:
        if ch == '{':
            depth += 1
            if depth == 1:
                current = []
                continue
            current.append(ch)
        elif ch == '}':
            depth -= 1
            if depth == 0:
                entries.append(''.join(current))
            else:
                current.append(ch)
        else:
            if depth >= 1:
                current.append(ch)

    for entry_str in entries:
        # Split only the top-level commas (not inside CONDITIONS parentheses)
        # Extract: EVO_METHOD, param, SPECIES_XXX[, CONDITIONS(...)]
        top_parts = []
        paren_depth = 0
        brace_depth = 0
        part = []
        for ch in entry_str:
            if ch == '(':
                paren_depth += 1
                part.append(ch)
            elif ch == ')':
                paren_depth -= 1
                part.append(ch)
            elif ch == '{':
                brace_depth += 1
                part.append(ch)
            elif ch == '}':
                brace_depth -= 1
                part.append(ch)
            elif ch == ',' and paren_depth == 0 and brace_depth == 0:
                top_parts.append(''.join(part).strip())
                part = []
            else:
                part.append(ch)
        if part:
            top_parts.append(''.join(part).strip())

        if len(top_parts) < 3:
            continue

        method_const = top_parts[0].strip()
        param_raw = top_parts[1].strip()
        target_const = top_parts[2].strip()
        conditions_str = top_parts[3].strip() if len(top_parts) > 3 else ''

        # Skip non-evolution entries
        if method_const.startswith('IF_') or method_const == 'CONDITIONS_END':
            continue

        # Method name — enhanced with CONDITIONS info
        method = format_evo_method(method_const)
        if method is None:
            continue

        # Parameter — enhanced with conditions
        param = format_evo_param(method_const, param_raw, item_names_map)

        # Parse CONDITIONS to improve method/param descriptions
        if conditions_str:
            cond_desc = parse_conditions(conditions_str)
            if cond_desc:
                if method == 'Level' and param == '0':
                    # Replace generic "Level 0" with the condition description
                    method = cond_desc
                    param = ''
                else:
                    param = f"{param} ({cond_desc})" if param else cond_desc

        # Target
        target_id = species_ids.get(target_const, 0)
        evolutions.append({
            'method': method,
            'param': param,
            'targetId': target_id,
            'targetName': target_const,
        })

    # Deduplicate: if same target appears multiple times, keep the most useful entry
    # (prefer Item methods over location-based, skip region-variant duplicates)
    seen_targets = {}
    deduped = []
    for evo in evolutions:
        tid = evo['targetId']
        if tid not in seen_targets:
            seen_targets[tid] = len(deduped)
            deduped.append(evo)
        else:
            # Keep the entry with the more useful method (prefer Item over location)
            existing = deduped[seen_targets[tid]]
            if evo['method'] == 'Item' and existing['method'] != 'Item':
                deduped[seen_targets[tid]] = evo
            # Otherwise keep the first one

    return deduped


def parse_conditions(conditions_str):
    """Parse a CONDITIONS(...) string into a human-readable description."""
    # Extract conditions: {IF_XXX, VALUE} or {IF_XXX} (no value)
    conds_with_value = re.findall(r'\{(\w+)\s*,\s*([^}]+)\}', conditions_str)
    conds_no_value = re.findall(r'\{(\w+)\}', conditions_str)

    # Combine: conditions with values as (type, value), without values as (type, None)
    all_conds = [(t, v.strip()) for t, v in conds_with_value]
    # Only add no-value conditions if they weren't already captured with a value
    seen_types = {t for t, _ in all_conds}
    for t in conds_no_value:
        if t not in seen_types:
            all_conds.append((t, None))

    parts = []
    for cond_type, cond_value in all_conds:
        if cond_type == 'IF_MIN_FRIENDSHIP':
            parts.append('Friendship')
        elif cond_type == 'IF_TIME':
            time_name = cond_value.replace('TIME_', '').title() if cond_value else ''
            parts.append(time_name)
        elif cond_type == 'IF_NOT_TIME':
            time_name = cond_value.replace('TIME_', '').title() if cond_value else ''
            if time_name == 'Night':
                parts.append('Day')
            else:
                parts.append(f'Not {time_name}')
        elif cond_type == 'IF_KNOWS_MOVE_TYPE':
            type_name = cond_value.replace('TYPE_', '').title() if cond_value else ''
            parts.append(f'Knows {type_name} move')
        elif cond_type == 'IF_KNOWS_MOVE' or cond_type == 'IF_HAS_MOVE':
            move_name = format_constant('MOVE_', cond_value) if cond_value else ''
            parts.append(f'Knows {move_name}')
        elif cond_type == 'IF_IN_MAP':
            map_name = cond_value.replace('MAP_', '').replace('_', ' ').title() if cond_value else ''
            parts.append(f'At {map_name}')
        elif cond_type == 'IF_GENDER':
            gender = 'Female' if cond_value and cond_value.strip() == 'MON_FEMALE' else 'Male'
            parts.append(gender)
        elif cond_type == 'IF_HOLDS_ITEM':
            item_name = format_item_name(cond_value) if cond_value else ''
            parts.append(f'Holding {item_name}')
        elif cond_type == 'IF_ATK_GT_DEF':
            parts.append('Atk > Def')
        elif cond_type == 'IF_ATK_LT_DEF':
            parts.append('Atk < Def')
        elif cond_type == 'IF_DEF_GT_ATK':
            parts.append('Def > Atk')
        elif cond_type == 'IF_ATK_EQ_DEF':
            parts.append('Atk = Def')
        elif cond_type == 'IF_NOT_REGION':
            parts.append('')  # Skip, not useful to display
        elif cond_type == 'IF_REGION':
            parts.append('')  # Skip
        else:
            desc = format_constant('IF_', cond_type)
            parts.append(desc)

    # Filter empty parts
    parts = [p for p in parts if p]
    return ', '.join(parts) if parts else None


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
# Trainer Parser
# ---------------------------------------------------------------------------

_BULBA = 'https://archives.bulbagarden.net/media/upload'
TRAINER_SPRITE_MAP = {
    # Pic field value -> Bulbagarden Gen III sprite URL
    'Hiker': f'{_BULBA}/b/b7/Spr_RS_Hiker.png',
    'Youngster': f'{_BULBA}/c/c5/Spr_RS_Youngster.png',
    'Lass': f'{_BULBA}/1/16/Spr_RS_Lass.png',
    'Beauty': f'{_BULBA}/c/cb/Spr_RS_Beauty.png',
    'Fisherman': f'{_BULBA}/9/9a/Spr_RS_Fisherman.png',
    'Lady': f'{_BULBA}/2/2d/Spr_RS_Lady.png',
    'Guitarist': f'{_BULBA}/1/14/Spr_RS_Guitarist.png',
    'Camper': f'{_BULBA}/e/e1/Spr_RS_Camper.png',
    'Picnicker': f'{_BULBA}/7/71/Spr_RS_Picnicker.png',
    'Bug Catcher': f'{_BULBA}/7/7f/Spr_RS_Bug_Catcher.png',
    'Bug Maniac': f'{_BULBA}/4/4f/Spr_RS_Bug_Maniac.png',
    'Hex Maniac': f'{_BULBA}/3/3d/Spr_RS_Hex_Maniac.png',
    'Ninja Boy': f'{_BULBA}/b/bc/Spr_RS_Ninja_Boy.png',
    'Kindler': f'{_BULBA}/0/07/Spr_RS_Kindler.png',
    'Collector': f'{_BULBA}/2/28/Spr_RS_Collector.png',
    'Gentleman': f'{_BULBA}/f/f8/Spr_RS_Gentleman.png',
    'Sailor': f'{_BULBA}/f/f1/Spr_RS_Sailor.png',
    'Rich Boy': f'{_BULBA}/d/d7/Spr_RS_Rich_Boy.png',
    'Aroma Lady': f'{_BULBA}/2/22/Spr_RS_Aroma_Lady.png',
    'Ruin Maniac': f'{_BULBA}/9/9a/Spr_RS_Ruin_Maniac.png',
    'Pokemaniac': f'{_BULBA}/6/61/Spr_RS_Pok%C3%A9Maniac.png',
    'Bird Keeper': f'{_BULBA}/4/4f/Spr_RS_Bird_Keeper.png',
    'Black Belt': f'{_BULBA}/f/f8/Spr_RS_Black_Belt.png',
    'Dragon Tamer': f'{_BULBA}/1/12/Spr_RS_Dragon_Tamer.png',
    'Battle Girl': f'{_BULBA}/5/54/Spr_RS_Battle_Girl.png',
    'Parasol Lady': f'{_BULBA}/e/ea/Spr_RS_Parasol_Lady.png',
    'Swimmer M': f'{_BULBA}/6/6e/Spr_RS_Swimmer_M.png',
    'Swimmer F': f'{_BULBA}/7/77/Spr_RS_Swimmer_F.png',
    'Tuber M': f'{_BULBA}/e/ed/Spr_RS_Tuber_M.png',
    'Tuber F': f'{_BULBA}/d/d9/Spr_RS_Tuber_F.png',
    'Cooltrainer M': f'{_BULBA}/f/f8/Spr_RS_Cooltrainer_M.png',
    'Cooltrainer F': f'{_BULBA}/7/78/Spr_RS_Cooltrainer_F.png',
    'School Kid M': f'{_BULBA}/5/54/Spr_RS_School_Kid_M.png',
    'School Kid F': f'{_BULBA}/4/4f/Spr_RS_School_Kid_F.png',
    'Psychic M': f'{_BULBA}/b/b7/Spr_RS_Psychic_M.png',
    'Psychic F': f'{_BULBA}/6/6b/Spr_RS_Psychic_F.png',
    'Pokefan M': f'{_BULBA}/a/a3/Spr_RS_Pok%C3%A9fan_M.png',
    'Pokefan F': f'{_BULBA}/6/63/Spr_RS_Pok%C3%A9fan_F.png',
    'Expert M': f'{_BULBA}/d/d0/Spr_RS_Expert_M.png',
    'Expert F': f'{_BULBA}/6/66/Spr_RS_Expert_F.png',
    'Pokemon Breeder M': f'{_BULBA}/5/55/Spr_RS_Pok%C3%A9mon_Breeder_M.png',
    'Pokemon Breeder F': f'{_BULBA}/a/a8/Spr_RS_Pok%C3%A9mon_Breeder_F.png',
    'Pokemon Ranger M': f'{_BULBA}/b/ba/Spr_RS_Pok%C3%A9mon_Ranger_M.png',
    'Pokemon Ranger F': f'{_BULBA}/4/48/Spr_RS_Pok%C3%A9mon_Ranger_F.png',
    'Cycling Triathlete M': f'{_BULBA}/2/29/Spr_RS_Triathlete_Biker_M.png',
    'Cycling Triathlete F': f'{_BULBA}/0/0d/Spr_RS_Triathlete_Biker_F.png',
    'Running Triathlete M': f'{_BULBA}/8/8d/Spr_RS_Triathlete_Runner_M.png',
    'Running Triathlete F': f'{_BULBA}/2/21/Spr_RS_Triathlete_Runner_F.png',
    'Swimming Triathlete M': f'{_BULBA}/8/8a/Spr_RS_Triathlete_Swimmer_M.png',
    'Swimming Triathlete F': f'{_BULBA}/6/60/Spr_RS_Triathlete_Swimmer_F.png',
    'Aqua Grunt M': f'{_BULBA}/7/74/Spr_RS_Team_Aqua_Grunt_M.png',
    'Aqua Grunt F': f'{_BULBA}/5/54/Spr_RS_Team_Aqua_Grunt_F.png',
    'Aqua Admin M': f'{_BULBA}/d/d0/Spr_RS_Matt.png',
    'Aqua Admin F': f'{_BULBA}/e/e2/Spr_RS_Shelly.png',
    'Aqua Leader Archie': f'{_BULBA}/f/fe/Spr_RS_Archie.png',
    'Magma Grunt M': f'{_BULBA}/a/a0/Spr_RS_Team_Magma_Grunt_M.png',
    'Magma Grunt F': f'{_BULBA}/0/03/Spr_RS_Team_Magma_Grunt_F.png',
    'Magma Admin': f'{_BULBA}/5/51/Spr_RS_Tabitha.png',
    'Magma Leader Maxie': f'{_BULBA}/e/e3/Spr_RS_Maxie.png',
    'Leader Roxanne': f'{_BULBA}/e/ef/Spr_RS_Roxanne.png',
    'Leader Brawly': f'{_BULBA}/9/92/Spr_RS_Brawly.png',
    'Leader Wattson': f'{_BULBA}/b/b2/Spr_RS_Wattson.png',
    'Leader Flannery': f'{_BULBA}/b/be/Spr_RS_Flannery.png',
    'Leader Norman': f'{_BULBA}/7/75/Spr_RS_Norman.png',
    'Leader Winona': f'{_BULBA}/0/0e/Spr_RS_Winona.png',
    'Leader Tate And Liza': f'{_BULBA}/3/38/Spr_RS_Tate_and_Liza.png',
    'Leader Juan': f'{_BULBA}/1/16/Spr_E_Juan.png',
    'Elite Four Sidney': f'{_BULBA}/8/86/Spr_RS_Sidney.png',
    'Elite Four Phoebe': f'{_BULBA}/e/e6/Spr_RS_Phoebe.png',
    'Elite Four Glacia': f'{_BULBA}/7/71/Spr_RS_Glacia.png',
    'Elite Four Drake': f'{_BULBA}/0/04/Spr_RS_Drake.png',
    'Champion Wallace': f'{_BULBA}/c/cc/Spr_E_Wallace.png',
    'Steven': f'{_BULBA}/a/ad/Spr_RS_Steven.png',
    'Wally': f'{_BULBA}/8/87/Spr_RS_Wally.png',
    'May': f'{_BULBA}/a/a4/Spr_E_May.png',
    'Brendan': f'{_BULBA}/e/e6/Spr_E_Brendan.png',
    'RS May': f'{_BULBA}/3/38/Spr_RS_May.png',
    'RS Brendan': f'{_BULBA}/6/68/Spr_RS_Brendan.png',
    'Leaf': f'{_BULBA}/3/38/Spr_RS_May.png',
    'Red': f'{_BULBA}/a/ad/Spr_RS_Steven.png',
    'Twins': f'{_BULBA}/3/34/Spr_RS_Twins.png',
    'Old Couple': f'{_BULBA}/0/06/Spr_RS_Old_Couple.png',
    'Sis And Bro': f'{_BULBA}/3/3a/Spr_RS_Sis_and_Bro.png',
    'Sr And Jr': f'{_BULBA}/7/70/Spr_RS_Sr_and_Jr.png',
    'Young Couple': f'{_BULBA}/2/2c/Spr_RS_Young_Couple.png',
    'Interviewer': f'{_BULBA}/5/5e/Spr_RS_Interviewer.png',
    'Arena Tycoon Greta': f'{_BULBA}/f/fc/Spr_E_Greta.png',
    'Dome Ace Tucker': f'{_BULBA}/b/bd/Spr_E_Tucker.png',
    'Factory Head Noland': f'{_BULBA}/c/c2/Spr_E_Noland.png',
    'Palace Maven Spenser': f'{_BULBA}/1/12/Spr_E_Spenser.png',
    'Pike Queen Lucy': f'{_BULBA}/5/5d/Spr_E_Lucy.png',
    'Pyramid King Brandon': f'{_BULBA}/2/2c/Spr_E_Brandon.png',
    'Salon Maiden Anabel': f'{_BULBA}/d/d1/Spr_E_Anabel.png',
}


def trainer_pic_to_sprite(pic_name):
    """Convert trainer Pic field to Bulbagarden Gen III sprite URL."""
    pic_clean = pic_name.strip()
    return TRAINER_SPRITE_MAP.get(pic_clean, '')


def parse_trainers_party(repo_root, species_ids):
    """Parse trainers.party Showdown-format file into trainer dicts."""
    filepath = os.path.join(repo_root, 'src', 'data', 'trainers.party')
    print("Parsing trainers.party...")

    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()

    # Split into trainer blocks by === TRAINER_XXX ===
    trainer_blocks = re.split(r'^=== (TRAINER_\w+) ===$', content, flags=re.MULTILINE)
    # Result: ['preamble', 'TRAINER_ID1', 'block1', 'TRAINER_ID2', 'block2', ...]

    trainers = {}

    for i in range(1, len(trainer_blocks), 2):
        trainer_id = trainer_blocks[i]
        block = trainer_blocks[i + 1]

        if trainer_id == 'TRAINER_NONE':
            continue

        # Parse trainer fields
        name = ''
        trainer_class = ''
        pic = ''
        is_double = False

        for line in block.split('\n'):
            line = line.strip()
            if line.startswith('Name:'):
                name = line[5:].strip()
            elif line.startswith('Class:'):
                trainer_class = line[6:].strip()
            elif line.startswith('Pic:'):
                pic = line[4:].strip()
            elif line.startswith('Double Battle:'):
                is_double = line[14:].strip().lower() == 'yes'

        # Parse Pokemon
        party = []
        # Split block into chunks separated by blank lines
        # Trainer fields come before the first blank-line-separated Pokemon block
        pokemon_section = False
        current_mon = None

        for line in block.split('\n'):
            stripped = line.strip()

            # Skip empty lines — they separate Pokemon
            if not stripped:
                if current_mon:
                    party.append(current_mon)
                    current_mon = None
                pokemon_section = True  # After first blank line past header
                continue

            # Skip trainer header fields
            if ':' in stripped and not stripped.startswith('-') and current_mon is None:
                field = stripped.split(':')[0].strip()
                if field in ('Name', 'Class', 'Pic', 'Gender', 'Music',
                             'Double Battle', 'AI', 'Mugshot', 'Items',
                             'Starting Status', 'Battle Type'):
                    continue

            # Move line
            if stripped.startswith('- '):
                if current_mon:
                    current_mon['moves'].append(stripped[2:].strip())
                continue

            # Pokemon stat lines
            if current_mon:
                if stripped.startswith('Level:'):
                    try:
                        current_mon['level'] = int(stripped[6:].strip())
                    except ValueError:
                        pass
                elif stripped.startswith('Ability:'):
                    current_mon['ability'] = stripped[8:].strip()
                elif stripped.endswith('Nature'):
                    current_mon['nature'] = stripped.replace('Nature', '').strip()
                elif stripped.startswith('IVs:'):
                    pass  # Skip IVs for display
                elif stripped.startswith('EVs:'):
                    pass  # Skip EVs for display
                elif stripped.startswith('Shiny:'):
                    current_mon['shiny'] = stripped[6:].strip().lower() == 'yes'
                elif stripped.startswith('Tera Type:'):
                    current_mon['teraType'] = stripped[10:].strip()
                continue

            # Species line: "Nickname (Species) (G) @ Item" or just "Species @ Item" or "Species"
            if pokemon_section or (not any(stripped.startswith(f) for f in
                    ('Name:', 'Class:', 'Pic:', 'Gender:', 'Music:',
                     'Double Battle:', 'AI:', 'Mugshot:', 'Items:',
                     'Starting Status:', 'Battle Type:'))):
                # Try to parse as a species line
                species_line = stripped
                item = ''
                species_name = ''

                # Extract held item
                if ' @ ' in species_line:
                    species_line, item = species_line.rsplit(' @ ', 1)
                    item = item.strip()

                # Remove gender indicator
                species_line = re.sub(r'\s*\([MF]\)\s*', ' ', species_line).strip()

                # Check for "Nickname (Species)" pattern
                nick_match = re.match(r'^(.+?)\s*\(([^)]+)\)\s*$', species_line)
                if nick_match:
                    species_name = nick_match.group(2).strip()
                else:
                    species_name = species_line.strip()

                if species_name and not species_name.startswith(('Name:', 'Class:', 'Pic:')):
                    # Resolve species name to ID
                    # Try as constant first (SPECIES_XXX), then as display name
                    sp_const = species_name if species_name.startswith('SPECIES_') else None
                    sp_id = 0
                    if sp_const:
                        sp_id = species_ids.get(sp_const, 0)
                        species_name = format_constant('SPECIES_', sp_const)
                    else:
                        # Find by name match
                        target = species_name.lower().replace(' ', '').replace('-', '').replace("'", '').replace('\u2019', '')
                        for const, sid in species_ids.items():
                            formatted = format_constant('SPECIES_', const).lower().replace(' ', '').replace('-', '').replace("'", '').replace('\u2019', '')
                            if formatted == target:
                                sp_id = sid
                                species_name = format_constant('SPECIES_', const)
                                break
                        if sp_id == 0:
                            # Use the name as-is
                            species_name = species_name.title()

                    current_mon = {
                        'species': species_name,
                        'speciesId': sp_id,
                        'level': 100,
                        'item': item,
                        'ability': '',
                        'nature': '',
                        'moves': [],
                        'shiny': False,
                        'teraType': '',
                    }

        # Don't forget last Pokemon
        if current_mon:
            party.append(current_mon)

        if not name and not party:
            continue

        trainers[trainer_id] = {
            'id': trainer_id,
            'name': name.title() if name.isupper() else name,
            'trainerClass': trainer_class,
            'isDouble': is_double,
            'sprite': trainer_pic_to_sprite(pic),
            'party': party,
        }

    print(f"  Parsed {len(trainers)} trainers")
    return trainers


def parse_trainer_locations(repo_root):
    """Parse map scripts for trainerbattle_* macros to map trainer IDs to locations."""
    maps_dir = os.path.join(repo_root, 'data', 'maps')
    print("Parsing trainer locations from scripts...")

    # Build map dir -> region_map_section mapping
    rms_path = os.path.join(repo_root, 'src', 'data', 'region_map', 'region_map_sections.json')
    with open(rms_path, 'r', encoding='utf-8') as f:
        rms = json.load(f)
    section_display = {}
    for s in rms['map_sections']:
        name = s.get('name', '')
        if name:
            section_display[s['id']] = re.sub(
                r'(\d+)', lambda m: m.group(0), name.title()
            )

    trainer_re = re.compile(r'trainerbattle\w*\s+(TRAINER_\w+)')

    trainer_locations = {}  # trainer_id -> (location, map_dir_name)

    for dirpath, dirnames, filenames in os.walk(maps_dir):
        if 'scripts.inc' not in filenames:
            continue
        dirname = os.path.basename(dirpath)

        # Skip FRLG
        if 'Frlg' in dirname:
            continue

        script_path = os.path.join(dirpath, 'scripts.inc')
        with open(script_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Get display name from region_map_section
        map_json_path = os.path.join(dirpath, 'map.json')
        location = prettify_map_name('MAP_' + dirname)
        if os.path.isfile(map_json_path):
            with open(map_json_path, 'r', encoding='utf-8') as f:
                map_data = json.load(f)
            sec = map_data.get('region_map_section', '')
            if sec in section_display:
                location = section_display[sec]

        for m in trainer_re.finditer(content):
            tid = m.group(1)
            if tid not in trainer_locations:
                trainer_locations[tid] = (location, dirname)

    print(f"  Found locations for {len(trainer_locations)} trainers")
    return trainer_locations


def build_trainer_order(connections):
    """BFS from Littleroot Town through map connections to derive progression order."""

    start = 'Littleroot Town'
    order = []
    queue = deque([start])
    seen = {start}

    while queue:
        loc = queue.popleft()
        order.append(loc)
        c = connections.get(loc, {})
        for direction in ['up', 'down', 'left', 'right']:
            for neighbor in c.get(direction, []):
                if neighbor not in seen:
                    seen.add(neighbor)
                    queue.append(neighbor)
        for warp in c.get('warps', []):
            if warp not in seen:
                seen.add(warp)
                queue.append(warp)

    return {loc: idx for idx, loc in enumerate(order)}


# ---------------------------------------------------------------------------
# Double Battle Detection
# ---------------------------------------------------------------------------

# Directional blocking behavior constants
MB_IMPASSABLE_EAST  = 0x30
MB_IMPASSABLE_WEST  = 0x31
MB_IMPASSABLE_NORTH = 0x32
MB_IMPASSABLE_SOUTH = 0x33
MB_IMPASSABLE_NORTHEAST = 0x34
MB_IMPASSABLE_NORTHWEST = 0x35
MB_IMPASSABLE_SOUTHEAST = 0x36
MB_IMPASSABLE_SOUTHWEST = 0x37
MB_IMPASSABLE_SOUTH_AND_NORTH = 0xC0
MB_IMPASSABLE_WEST_AND_EAST = 0xC1

# Movement type -> set of possible facing directions
MOVEMENT_DIRECTIONS = {
    'MOVEMENT_TYPE_FACE_UP': {'up'},
    'MOVEMENT_TYPE_FACE_DOWN': {'down'},
    'MOVEMENT_TYPE_FACE_LEFT': {'left'},
    'MOVEMENT_TYPE_FACE_RIGHT': {'right'},
    'MOVEMENT_TYPE_LOOK_AROUND': {'up', 'down', 'left', 'right'},
    'MOVEMENT_TYPE_ROTATE_COUNTERCLOCKWISE': {'up', 'down', 'left', 'right'},
    'MOVEMENT_TYPE_ROTATE_CLOCKWISE': {'up', 'down', 'left', 'right'},
    'MOVEMENT_TYPE_FACE_DOWN_AND_UP': {'down', 'up'},
    'MOVEMENT_TYPE_FACE_LEFT_AND_RIGHT': {'left', 'right'},
    'MOVEMENT_TYPE_FACE_UP_AND_LEFT': {'up', 'left'},
    'MOVEMENT_TYPE_FACE_UP_AND_RIGHT': {'up', 'right'},
    'MOVEMENT_TYPE_FACE_DOWN_AND_LEFT': {'down', 'left'},
    'MOVEMENT_TYPE_FACE_DOWN_AND_RIGHT': {'down', 'right'},
    'MOVEMENT_TYPE_FACE_DOWN_UP_AND_LEFT': {'down', 'up', 'left'},
    'MOVEMENT_TYPE_FACE_DOWN_UP_AND_RIGHT': {'down', 'up', 'right'},
    'MOVEMENT_TYPE_FACE_UP_LEFT_AND_RIGHT': {'up', 'left', 'right'},
    'MOVEMENT_TYPE_FACE_DOWN_LEFT_AND_RIGHT': {'down', 'left', 'right'},
    'MOVEMENT_TYPE_WANDER_AROUND': {'up', 'down', 'left', 'right'},
    'MOVEMENT_TYPE_WANDER_AROUND_SLOWER': {'up', 'down', 'left', 'right'},
    'MOVEMENT_TYPE_WANDER_UP_AND_DOWN': {'up', 'down'},
    'MOVEMENT_TYPE_WANDER_DOWN_AND_UP': {'up', 'down'},
    'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT': {'left', 'right'},
    'MOVEMENT_TYPE_WANDER_RIGHT_AND_LEFT': {'left', 'right'},
}

# Direction deltas: (dx, dy)
DIR_DELTA = {
    'up': (0, -1),
    'down': (0, 1),
    'left': (-1, 0),
    'right': (1, 0),
}

DIR_OPPOSITE = {'up': 'down', 'down': 'up', 'left': 'right', 'right': 'left'}


def _build_tileset_path_map(repo_root):
    """Parse headers.h and metatiles.h to build gTileset_XXX -> metatile_attributes.bin path."""
    headers_path = os.path.join(repo_root, 'src', 'data', 'tilesets', 'headers.h')
    metatiles_path = os.path.join(repo_root, 'src', 'data', 'tilesets', 'metatiles.h')

    with open(headers_path, 'r', encoding='utf-8', errors='replace') as f:
        headers = f.read()
    with open(metatiles_path, 'r', encoding='utf-8', errors='replace') as f:
        metatiles = f.read()

    # tileset name -> attributes variable name
    ts_to_attr = {}
    blocks = re.split(r'const struct Tileset\s+', headers)
    for block in blocks[1:]:
        name_m = re.match(r'(gTileset_\w+)', block)
        attr_m = re.search(r'\.metatileAttributes\s*=\s*(gMetatileAttributes_\w+)', block)
        if name_m and attr_m:
            ts_to_attr[name_m.group(1)] = attr_m.group(1)

    # attributes variable name -> file path
    attr_to_path = {}
    for m in re.finditer(r'(gMetatileAttributes_\w+)\[\]\s*=\s*INCBIN_U16\("([^"]+)"\)', metatiles):
        attr_to_path[m.group(1)] = m.group(2)

    # Final: tileset name -> file path
    result = {}
    for ts_name, attr_name in ts_to_attr.items():
        if attr_name in attr_to_path:
            result[ts_name] = attr_to_path[attr_name]

    return result


def _load_metatile_behaviors(repo_root, primary_tileset, secondary_tileset, tileset_paths):
    """Load metatile behavior arrays for primary and secondary tilesets."""
    st = struct

    primary_behaviors = []
    secondary_behaviors = []

    path = tileset_paths.get(primary_tileset)
    if path:
        fullpath = os.path.join(repo_root, path)
        if os.path.isfile(fullpath):
            with open(fullpath, 'rb') as f:
                data = f.read()
            primary_behaviors = [st.unpack_from('<H', data, i * 2)[0] & 0xFF
                                 for i in range(len(data) // 2)]

    path = tileset_paths.get(secondary_tileset)
    if path:
        fullpath = os.path.join(repo_root, path)
        if os.path.isfile(fullpath):
            with open(fullpath, 'rb') as f:
                data = f.read()
            secondary_behaviors = [st.unpack_from('<H', data, i * 2)[0] & 0xFF
                                   for i in range(len(data) // 2)]

    return primary_behaviors, secondary_behaviors


def _build_collision_grid(repo_root, layout, tileset_paths):
    """Build a collision grid for a map layout.
    Returns (width, height, grid) where grid[y][x] = {
        'passable': bool,  # basic passability
        'behavior': int,   # metatile behavior byte
        'elevation': int,
    }
    """
    st = struct

    w = layout['width']
    h = layout['height']
    blockdata_path = os.path.join(repo_root, layout['blockdata_filepath'])

    if not os.path.isfile(blockdata_path):
        return w, h, None

    with open(blockdata_path, 'rb') as f:
        map_data = f.read()

    primary_behaviors, secondary_behaviors = _load_metatile_behaviors(
        repo_root, layout['primary_tileset'], layout['secondary_tileset'], tileset_paths
    )

    grid = []
    for y in range(h):
        row = []
        for x in range(w):
            offset = (y * w + x) * 2
            if offset + 1 >= len(map_data):
                row.append({'passable': False, 'behavior': 0xFF, 'elevation': 0})
                continue

            val = st.unpack_from('<H', map_data, offset)[0]
            metatile_id = val & 0x03FF
            collision = (val >> 10) & 0x3
            elevation = (val >> 12) & 0xF

            # Look up behavior
            behavior = 0
            if metatile_id < 512:
                if metatile_id < len(primary_behaviors):
                    behavior = primary_behaviors[metatile_id]
            else:
                idx = metatile_id - 512
                if idx < len(secondary_behaviors):
                    behavior = secondary_behaviors[idx]

            passable = (collision == 0)
            row.append({'passable': passable, 'behavior': behavior, 'elevation': elevation})
        grid.append(row)

    return w, h, grid


def _is_directionally_blocked(behavior, direction):
    """Check if a metatile behavior blocks movement in a given direction."""
    blocks = {
        MB_IMPASSABLE_EAST: {'right'},
        MB_IMPASSABLE_WEST: {'left'},
        MB_IMPASSABLE_NORTH: {'up'},
        MB_IMPASSABLE_SOUTH: {'down'},
        MB_IMPASSABLE_NORTHEAST: {'up', 'right'},
        MB_IMPASSABLE_NORTHWEST: {'up', 'left'},
        MB_IMPASSABLE_SOUTHEAST: {'down', 'right'},
        MB_IMPASSABLE_SOUTHWEST: {'down', 'left'},
        MB_IMPASSABLE_SOUTH_AND_NORTH: {'down', 'up'},
        MB_IMPASSABLE_WEST_AND_EAST: {'left', 'right'},
    }
    blocked_dirs = blocks.get(behavior, set())
    return direction in blocked_dirs


def _can_move(grid, w, h, fx, fy, direction):
    """Check if movement from (fx, fy) in direction is possible, considering
    both the source tile's outgoing blocks and the destination tile's incoming blocks."""
    dx, dy = DIR_DELTA[direction]
    tx, ty = fx + dx, fy + dy

    if tx < 0 or tx >= w or ty < 0 or ty >= h:
        return False

    dest = grid[ty][tx]
    if not dest['passable']:
        return False

    # Check outgoing directional block from source
    src = grid[fy][fx]
    if _is_directionally_blocked(src['behavior'], direction):
        return False

    # Check incoming directional block on destination
    opposite = DIR_OPPOSITE[direction]
    if _is_directionally_blocked(dest['behavior'], opposite):
        return False

    return True


def _compute_sight_tiles(x, y, movement_type, trainer_type, sight_range, grid, w, h):
    """Compute all tiles a trainer can see, accounting for direction and obstacles.
    Returns dict mapping direction -> set of (tx, ty) tiles visible in that direction."""
    if trainer_type == 'TRAINER_TYPE_SEE_ALL_DIRECTIONS':
        directions = {'up', 'down', 'left', 'right'}
    else:
        directions = MOVEMENT_DIRECTIONS.get(movement_type, {'down'})

    try:
        sight_range = int(sight_range)
    except (ValueError, TypeError):
        sight_range = 0

    sight_by_dir = {}
    for d in directions:
        tiles = set()
        dx, dy = DIR_DELTA[d]
        for dist in range(1, sight_range + 1):
            tx, ty = x + dx * dist, y + dy * dist
            if tx < 0 or tx >= w or ty < 0 or ty >= h:
                break
            tile = grid[ty][tx]
            if not tile['passable']:
                break
            tiles.add((tx, ty))
        sight_by_dir[d] = tiles

    return sight_by_dir


def _get_entry_points(map_data, w, h):
    """Get map entry points from connections and warp_events."""
    entries = set()

    # Connections: edges of the map
    for conn in (map_data.get('connections') or []):
        direction = conn.get('direction', '')
        offset = conn.get('offset', 0)
        if direction == 'up':
            for x in range(w):
                entries.add((x, 0))
        elif direction == 'down':
            for x in range(w):
                entries.add((x, h - 1))
        elif direction == 'left':
            for y in range(h):
                entries.add((0, y))
        elif direction == 'right':
            for y in range(h):
                entries.add((w - 1, y))

    # Warp events
    for warp in map_data.get('warp_events', []):
        x, y = warp.get('x', 0), warp.get('y', 0)
        if 0 <= x < w and 0 <= y < h:
            entries.add((x, y))

    return entries


def _bfs_reachable(grid, w, h, starts, avoid_tiles=None):
    """BFS from start tiles, returning set of reachable tiles.
    Optionally avoids a set of tiles (sight zones)."""
    if avoid_tiles is None:
        avoid_tiles = set()

    visited = set()
    queue = deque()
    for s in starts:
        if s not in avoid_tiles and 0 <= s[0] < w and 0 <= s[1] < h:
            tile = grid[s[1]][s[0]]
            if tile['passable']:
                queue.append(s)
                visited.add(s)

    while queue:
        cx, cy = queue.popleft()
        for d in ('up', 'down', 'left', 'right'):
            dx, dy = DIR_DELTA[d]
            nx, ny = cx + dx, cy + dy
            if (nx, ny) in visited or (nx, ny) in avoid_tiles:
                continue
            if not _can_move(grid, w, h, cx, cy, d):
                continue
            visited.add((nx, ny))
            queue.append((nx, ny))

    return visited


def detect_double_battles(repo_root, tileset_paths):
    """Detect potential double battles by analyzing trainer sight line overlaps."""
    maps_dir = os.path.join(repo_root, 'data', 'maps')
    layouts_data = json.load(open(os.path.join(repo_root, 'data', 'layouts', 'layouts.json')))
    layout_lookup = {l['id']: l for l in layouts_data['layouts']}

    print("Detecting double battles...")

    # trainer_id -> {partner_id, forced}
    doubles = {}

    map_count = 0
    for dirpath, dirnames, filenames in os.walk(maps_dir):
        if 'map.json' not in filenames:
            continue
        dirname = os.path.basename(dirpath)
        if 'Frlg' in dirname:
            continue

        with open(os.path.join(dirpath, 'map.json'), 'r', encoding='utf-8') as f:
            map_data = json.load(f)

        # Get trainers on this map
        trainers_on_map = []
        for obj in map_data.get('object_events', []):
            if obj.get('trainer_type') not in ('TRAINER_TYPE_NORMAL', 'TRAINER_TYPE_SEE_ALL_DIRECTIONS'):
                continue
            script = obj.get('script', '')
            # Extract trainer ID from script name (e.g. Route102_EventScript_Calvin -> look up in trainerbattle)
            trainers_on_map.append(obj)

        if len(trainers_on_map) < 2:
            continue

        # Build collision grid
        layout_id = map_data.get('layout', '')
        layout = layout_lookup.get(layout_id)
        if not layout:
            continue

        w, h, grid = _build_collision_grid(repo_root, layout, tileset_paths)
        if grid is None:
            continue

        map_count += 1

        # Get entry points
        entries = _get_entry_points(map_data, w, h)
        if not entries:
            # Fallback: use all passable edge tiles
            for x in range(w):
                if grid[0][x]['passable']:
                    entries.add((x, 0))
                if grid[h-1][x]['passable']:
                    entries.add((x, h-1))
            for y in range(h):
                if grid[y][0]['passable']:
                    entries.add((0, y))
                if grid[y][w-1]['passable']:
                    entries.add((w-1, y))

        # Compute sight tiles for each trainer
        trainer_sights = []
        for obj in trainers_on_map:
            x, y = obj.get('x', 0), obj.get('y', 0)
            mt = obj.get('movement_type', '')
            tt = obj.get('trainer_type', '')
            sr = obj.get('trainer_sight_or_berry_tree_id', '0')
            script = obj.get('script', '')

            sight_by_dir = _compute_sight_tiles(x, y, mt, tt, sr, grid, w, h)
            all_sight = set()
            for tiles in sight_by_dir.values():
                all_sight |= tiles

            # Determine if all directions are fixed (single direction only)
            if tt == 'TRAINER_TYPE_SEE_ALL_DIRECTIONS':
                is_fixed = False
            else:
                dirs = MOVEMENT_DIRECTIONS.get(mt, {'down'})
                is_fixed = len(dirs) == 1

            trainer_sights.append({
                'obj': obj,
                'x': x, 'y': y,
                'script': script,
                'sight_by_dir': sight_by_dir,
                'all_sight': all_sight,
                'is_fixed': is_fixed,
                'directions': MOVEMENT_DIRECTIONS.get(mt, {'down'}) if tt != 'TRAINER_TYPE_SEE_ALL_DIRECTIONS' else {'up','down','left','right'},
            })

        # Collect all NPC positions on this map (trainers block movement)
        all_npc_tiles = set()
        for obj in map_data.get('object_events', []):
            ox, oy = obj.get('x', 0), obj.get('y', 0)
            if 0 <= ox < w and 0 <= oy < h:
                all_npc_tiles.add((ox, oy))

        # Find pairs with overlapping sight
        for i in range(len(trainer_sights)):
            for j in range(i + 1, len(trainer_sights)):
                t1 = trainer_sights[i]
                t2 = trainer_sights[j]

                overlap = t1['all_sight'] & t2['all_sight']
                if not overlap:
                    continue

                # We have an overlap. Now classify as forced or possible.
                # Forced: player cannot reach either trainer's adjacent tiles
                # without crossing the other's sight zone.

                # Get tiles adjacent to each trainer (where player stands to battle)
                # Exclude NPC-occupied tiles (can't stand on another NPC)
                def adjacent_passable(tx, ty):
                    adj = set()
                    for d in ('up', 'down', 'left', 'right'):
                        dx, dy = DIR_DELTA[d]
                        nx, ny = tx + dx, ty + dy
                        if 0 <= nx < w and 0 <= ny < h and grid[ny][nx]['passable']:
                            if (nx, ny) not in all_npc_tiles:
                                adj.add((nx, ny))
                    return adj

                t1_adj = adjacent_passable(t1['x'], t1['y'])
                t2_adj = adjacent_passable(t2['x'], t2['y'])

                # BFS: can we reach t1's adjacent tiles while avoiding t2's sight?
                # Also avoid NPC tiles (player can't walk through NPCs)
                avoid_for_t1 = t2['all_sight'] | all_npc_tiles
                reachable_avoiding_t2 = _bfs_reachable(grid, w, h, entries, avoid_tiles=avoid_for_t1)
                can_single_t1 = bool(t1_adj & reachable_avoiding_t2)

                # BFS: can we reach t2's adjacent tiles while avoiding t1's sight?
                avoid_for_t2 = t1['all_sight'] | all_npc_tiles
                reachable_avoiding_t1 = _bfs_reachable(grid, w, h, entries, avoid_tiles=avoid_for_t2)
                can_single_t2 = bool(t2_adj & reachable_avoiding_t1)

                forced = not can_single_t1 and not can_single_t2

                # Also classify as forced if trainers face each other on the
                # same axis with overlapping sight — this is the classic forced
                # double pattern. BFS may find obscure detour paths, but in
                # practice the player walks through the overlap.
                if not forced and overlap:
                    t1_fixed = t1['is_fixed']
                    t2_fixed = t2['is_fixed']
                    t1_dirs = t1['directions']
                    t2_dirs = t2['directions']
                    same_x = t1['x'] == t2['x']
                    same_y = t1['y'] == t2['y']
                    if same_x and t1_fixed and t2_fixed:
                        if ('down' in t1_dirs and 'up' in t2_dirs and t1['y'] < t2['y']):
                            forced = True
                        elif ('up' in t1_dirs and 'down' in t2_dirs and t1['y'] > t2['y']):
                            forced = True
                    if same_y and t1_fixed and t2_fixed:
                        if ('right' in t1_dirs and 'left' in t2_dirs and t1['x'] < t2['x']):
                            forced = True
                        elif ('left' in t1_dirs and 'right' in t2_dirs and t1['x'] > t2['x']):
                            forced = True

                # Extract trainer IDs from scripts
                script1 = t1['script']
                script2 = t2['script']

                doubles[script1] = doubles.get(script1, [])
                doubles[script1].append({'partner_script': script2, 'forced': forced})
                doubles[script2] = doubles.get(script2, [])
                doubles[script2].append({'partner_script': script1, 'forced': forced})

    print(f"  Analyzed {map_count} maps with 2+ trainers")
    print(f"  Found {sum(len(v) for v in doubles.values()) // 2} potential double battle pairs")
    return doubles


def _build_script_to_trainer_map(repo_root):
    """Parse map scripts to build a mapping from script label -> TRAINER_ID."""
    maps_dir = os.path.join(repo_root, 'data', 'maps')
    script_to_trainer = {}

    trainer_re = re.compile(r'trainerbattle\w*\s+(TRAINER_\w+)')

    for dirpath, dirnames, filenames in os.walk(maps_dir):
        if 'scripts.inc' not in filenames:
            continue
        script_path = os.path.join(dirpath, 'scripts.inc')
        with open(script_path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()

        # Find each script label and the trainer it references
        current_label = None
        for line in content.split('\n'):
            stripped = line.strip()
            # Script label: "LabelName::" or "LabelName:"
            label_m = re.match(r'^(\w+EventScript_\w+)::', stripped)
            if label_m:
                current_label = label_m.group(1)
            # Also match simple labels
            if not label_m:
                label_m = re.match(r'^(\w+)::', stripped)
                if label_m:
                    current_label = label_m.group(1)

            tm = trainer_re.search(stripped)
            if tm and current_label:
                tid = tm.group(1)
                script_to_trainer[current_label] = tid

    return script_to_trainer


def build_trainers_json(trainers, trainer_locations, connections, pokemon_by_id,
                        doubles_data=None, script_to_trainer=None):
    """Build final trainers list sorted by BFS map order, excluding locationless trainers."""
    location_order = build_trainer_order(connections)

    # Build reverse map: script_label -> trainer_id (from doubles detection)
    # and trainer_id -> script_label
    tid_to_script = {}
    if script_to_trainer:
        for script_label, tid in script_to_trainer.items():
            tid_to_script.setdefault(tid, script_label)

    trainer_list = []
    for tid, trainer in trainers.items():
        loc_info = trainer_locations.get(tid)
        if not loc_info:
            continue  # Skip trainers without a map location
        location = loc_info[0]

        # Resolve species names from pokemon_by_id where possible
        for mon in trainer['party']:
            sp_id = mon.get('speciesId', 0)
            if sp_id and sp_id in pokemon_by_id:
                mon['species'] = pokemon_by_id[sp_id]['name']

        # Look up double battle info
        double_with = None
        if doubles_data and script_to_trainer:
            script_label = tid_to_script.get(tid, '')
            if script_label in doubles_data:
                for dbl in doubles_data[script_label]:
                    partner_script = dbl['partner_script']
                    partner_tid = script_to_trainer.get(partner_script, '')
                    if partner_tid and partner_tid in trainers:
                        # Pick the strongest match (forced > possible)
                        if double_with is None or dbl['forced']:
                            double_with = {
                                'trainerId': partner_tid,
                                'forced': dbl['forced'],
                            }

        trainer_list.append({
            'id': tid,
            'name': trainer['name'],
            'trainerClass': trainer['trainerClass'],
            'isDouble': trainer['isDouble'],
            'sprite': trainer['sprite'],
            'location': location,
            'party': trainer['party'],
            'doubleWith': double_with,
            'order': location_order.get(location, 9999),
        })

    # Sort by BFS order, then by trainer ID for same-location trainers
    trainer_list.sort(key=lambda t: (t['order'], t['id']))

    # Remove order field from output
    for t in trainer_list:
        del t['order']

    return trainer_list


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


def parse_gift_pokemon(repo_root, species_ids):
    """Parse givemon/giveegg calls from map scripts to find gift Pokemon."""
    maps_dir = os.path.join(repo_root, 'data', 'maps')
    print("Parsing gift Pokemon from scripts...")

    # Build region_map_section -> display name mapping
    rms_path = os.path.join(repo_root, 'src', 'data', 'region_map', 'region_map_sections.json')
    with open(rms_path, 'r', encoding='utf-8') as f:
        rms = json.load(f)
    section_display = {}
    for s in rms['map_sections']:
        name = s.get('name', '')
        if name:
            # "ROUTE 119" -> "Route 119", "RUSTBORO CITY" -> "Rustboro City"
            section_display[s['id']] = re.sub(
                r'(\d+)', lambda m: m.group(0),
                name.title()
            )

    # Build map dir name -> region_map_section lookup
    map_to_section = {}
    for dirpath, dirnames, filenames in os.walk(maps_dir):
        if 'map.json' not in filenames:
            continue
        with open(os.path.join(dirpath, 'map.json'), 'r', encoding='utf-8') as f:
            data = json.load(f)
        dirname = os.path.basename(dirpath)
        sec = data.get('region_map_section', '')
        map_to_section[dirname] = sec

    givemon_re = re.compile(r'^\s*givemon\s+(\w+)\s*,\s*(\d+)', re.MULTILINE)
    giveegg_re = re.compile(r'^\s*giveegg\s+(\w+)', re.MULTILINE)

    gifts = []  # list of (location, species_const, level)

    for dirpath, dirnames, filenames in os.walk(maps_dir):
        if 'scripts.inc' not in filenames:
            continue
        dirname = os.path.basename(dirpath)

        # Skip FRLG maps
        if dirname.endswith('_Frlg') or '_Frlg_' in dirname or 'Frlg' in dirname:
            continue

        script_path = os.path.join(dirpath, 'scripts.inc')
        with open(script_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Resolve parent location via region_map_section
        sec = map_to_section.get(dirname, '')
        location = section_display.get(sec, '')
        if not location:
            location = prettify_map_name('MAP_' + dirname)

        for m in givemon_re.finditer(content):
            species_const = m.group(1)
            level = int(m.group(2))
            if species_const.startswith('SPECIES_'):
                gifts.append((location, species_const, level))

        for m in giveegg_re.finditer(content):
            species_const = m.group(1)
            if species_const.startswith('SPECIES_'):
                gifts.append((location, species_const, 0))  # 0 = egg

    print(f"  Found {len(gifts)} gift Pokemon across {len(set(g[0] for g in gifts))} locations")
    return gifts


def parse_encounters(repo_root, species_ids):
    """Parse wild_encounters.json and build per-species and per-location encounter lists."""
    filepath = os.path.join(repo_root, 'src', 'data', 'wild_encounters.json')
    print("Parsing wild_encounters.json...")

    with open(filepath, 'r', encoding='utf-8') as f:
        data = json.load(f)

    species_encounters = defaultdict(list)  # species_id -> [encounter]
    location_encounters = defaultdict(list)  # location_name -> [pokemon]

    # Slot percentages from src/data/wild_encounters.h
    # Each list index = slot index, value = that slot's individual percentage
    GRASS_RATES = [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1]  # 12 slots, sum=100
    WATER_RATES = [60, 30, 5, 4, 1]  # 5 slots, sum=100
    ROCK_SMASH_RATES = [60, 30, 5, 4, 1]  # 5 slots, sum=100
    # Fishing: 10 slots split by rod. Configure which rod to use here.
    # Old Rod: slots 0-1 (70, 30), Good Rod: slots 2-3 (60, 40), Super Rod: slots 4-9 (20,20,20,20,15,5)
    FISHING_ROD = 'super'  # Change to 'old', 'good', or 'super'
    FISHING_ROD_CONFIG = {
        'old':   {'slots': (0, 2),  'rates': [70, 30],             'label': 'old rod'},
        'good':  {'slots': (2, 4),  'rates': [60, 40],             'label': 'good rod'},
        'super': {'slots': (4, 10), 'rates': [20, 20, 20, 20, 15, 5], 'label': 'fishing'},
    }
    rod_cfg = FISHING_ROD_CONFIG[FISHING_ROD]

    method_configs = {
        'land_mons':      {'label': 'grass',      'rates': GRASS_RATES,      'slot_range': None},
        'water_mons':     {'label': 'surfing',     'rates': WATER_RATES,      'slot_range': None},
        'rock_smash_mons':{'label': 'rock smash',  'rates': ROCK_SMASH_RATES, 'slot_range': None},
        'fishing_mons':   {'label': rod_cfg['label'], 'rates': rod_cfg['rates'], 'slot_range': rod_cfg['slots']},
        'hidden_mons':    {'label': 'hidden',      'rates': None,             'slot_range': None},
    }

    for group in data.get('wild_encounter_groups', []):
        for encounter in group.get('encounters', []):
            map_name = encounter.get('map', '')
            if not map_name:
                continue
            location = prettify_map_name(map_name)

            for method_key, cfg in method_configs.items():
                mons_data = encounter.get(method_key)
                if not mons_data:
                    continue

                mons = mons_data.get('mons', [])
                method_label = cfg['label']
                rates = cfg['rates']
                slot_range = cfg['slot_range']

                # Filter to relevant slots if configured (fishing rod)
                if slot_range:
                    start, end = slot_range
                    mons = mons[start:end]
                elif rates:
                    mons = mons[:len(rates)]

                # Deduplicate: merge level ranges and sum percentages per species
                species_data = defaultdict(lambda: [999, 0, 0])  # [minLv, maxLv, pct]
                for i, mon in enumerate(mons):
                    sp = mon.get('species', '')
                    min_lv = mon.get('min_level', 0)
                    max_lv = mon.get('max_level', 0)
                    pct = rates[i] if rates and i < len(rates) else 0
                    if sp:
                        sd = species_data[sp]
                        sd[0] = min(sd[0], min_lv)
                        sd[1] = max(sd[1], max_lv)
                        sd[2] += pct

                for sp, (min_lv, max_lv, pct) in species_data.items():
                    sp_id = species_ids.get(sp, 0)
                    sp_name = format_constant('SPECIES_', sp)

                    enc_entry = {
                        'location': location,
                        'method': method_label,
                        'minLevel': min_lv,
                        'maxLevel': max_lv,
                        'percentage': pct,
                    }
                    species_encounters[sp_id].append(enc_entry)

                    loc_entry = {
                        'species': sp_name,
                        'speciesId': sp_id,
                        'method': method_label,
                        'minLevel': min_lv,
                        'maxLevel': max_lv,
                        'percentage': pct,
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
# Map Connections Parser
# ---------------------------------------------------------------------------

def parse_map_connections(repo_root):
    """Parse map.json files to build a connection graph between locations."""
    maps_dir = os.path.join(repo_root, 'data', 'maps')
    print("Parsing map connections...")

    connections = {}  # pretty_name -> { up: [], down: [], left: [], right: [], warps: [] }

    for dirpath, dirnames, filenames in os.walk(maps_dir):
        if 'map.json' not in filenames:
            continue
        filepath = os.path.join(dirpath, 'map.json')
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)

        map_id = data.get('id', '')
        if not map_id:
            continue

        pretty = prettify_map_name(map_id)
        entry = connections.setdefault(pretty, {
            'up': [], 'down': [], 'left': [], 'right': [], 'warps': [],
        })

        # Directional connections (overworld)
        for conn in (data.get('connections') or []):
            direction = conn.get('direction', '')
            target = prettify_map_name(conn.get('map', ''))
            if not target:
                continue
            if direction in ('up', 'down', 'left', 'right'):
                if target not in entry[direction]:
                    entry[direction].append(target)
            else:
                # dive, emerge, etc. -> treat as warps
                if target not in entry['warps']:
                    entry['warps'].append(target)

        # Warp connections (dungeons, caves, buildings)
        seen_warps = set()
        for warp in data.get('warp_events', []):
            dest = warp.get('dest_map', '')
            if not dest:
                continue
            target = prettify_map_name(dest)
            if target != pretty and target not in seen_warps:
                seen_warps.add(target)
                if target not in entry['warps']:
                    entry['warps'].append(target)

    print(f"  Found connections for {len(connections)} maps")
    return connections


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

    # Step 9a: Parse gift Pokemon and merge into encounters
    gifts = parse_gift_pokemon(repo_root, species_ids)
    # Build a lookup from location_encounters list for merging
    loc_enc_map = {le['location']: le for le in location_encounters}
    for location, species_const, level in gifts:
        sp_id = species_ids.get(species_const, 0)
        if sp_id == 0:
            continue
        # Use the actual Pokemon name if available, fall back to formatting the constant
        sp_name = pokemon_by_id[sp_id]['name'] if sp_id in pokemon_by_id else format_constant('SPECIES_', species_const)

        enc_entry = {
            'location': location,
            'method': 'gift',
            'minLevel': level,
            'maxLevel': level,
            'percentage': 0,
        }
        species_encounters[sp_id].append(enc_entry)

        loc_entry = {
            'species': sp_name,
            'speciesId': sp_id,
            'method': 'gift',
            'minLevel': level,
            'maxLevel': level,
            'percentage': 0,
        }
        if location in loc_enc_map:
            loc_enc_map[location]['pokemon'].append(loc_entry)
        else:
            new_loc = {'location': location, 'pokemon': [loc_entry]}
            location_encounters.append(new_loc)
            loc_enc_map[location] = new_loc

    # Re-sort location_encounters after adding gifts
    location_encounters.sort(key=lambda le: le['location'])

    # Attach encounters to Pokemon
    for pkmn in pokemon_list:
        pkmn['encounters'] = species_encounters.get(pkmn['id'], [])

    # Step 9.1: Parse map connections
    map_connections = parse_map_connections(repo_root)

    # Step 9.5: Deduplicate forms
    print()
    pokemon_list = deduplicate_forms(pokemon_list)

    # Step 9.6: Re-resolve evolution target names after form dedup (names may have changed)
    pokemon_by_id = {p['id']: p for p in pokemon_list}
    for pkmn in pokemon_list:
        for evo in pkmn.get('evolutions', []):
            target_id = evo.get('targetId', 0)
            if target_id in pokemon_by_id:
                evo['targetName'] = pokemon_by_id[target_id]['name']

    # Step 9.7: Parse trainers
    print()
    trainers = parse_trainers_party(repo_root, species_ids)
    trainer_locations = parse_trainer_locations(repo_root)

    # Step 9.8: Detect double battles
    print()
    tileset_paths = _build_tileset_path_map(repo_root)
    doubles_data = detect_double_battles(repo_root, tileset_paths)
    script_to_trainer = _build_script_to_trainer_map(repo_root)

    trainer_list = build_trainers_json(
        trainers, trainer_locations, map_connections, pokemon_by_id,
        doubles_data, script_to_trainer
    )
    print(f"  Built {len(trainer_list)} trainer entries")

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

    # Write connections.json
    connections_path = os.path.join(output_dir, 'connections.json')
    with open(connections_path, 'w', encoding='utf-8') as f:
        json.dump(map_connections, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {connections_path} ({len(map_connections)} maps)")

    # Write trainers.json
    trainers_path = os.path.join(output_dir, 'trainers.json')
    with open(trainers_path, 'w', encoding='utf-8') as f:
        json.dump(trainer_list, f, indent=2, ensure_ascii=False)
    print(f"  Wrote {trainers_path} ({len(trainer_list)} trainers)")

    print()
    print("Done!")


if __name__ == '__main__':
    main()
