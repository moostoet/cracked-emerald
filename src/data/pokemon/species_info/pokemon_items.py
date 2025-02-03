import re
import os

def item_constant_to_name(constant):
    """Converts an ITEM_* constant to a formatted item name."""
    if not constant:
        return "None"
    name_part = constant.replace("ITEM_", "", 1)
    words = name_part.split('_')
    capitalized = [word.capitalize() for word in words]
    return ' '.join(capitalized)

def parse_structs(content):
    """Extracts all Pokemon structs from the given content."""
    structs = []
    start = 0
    while True:
        species_start = content.find('[SPECIES_', start)
        if species_start == -1:
            break
        equals_pos = content.find('=', species_start)
        if equals_pos == -1:
            break
        brace_pos = content.find('{', equals_pos)
        if brace_pos == -1:
            break
        balance = 1
        end_pos = brace_pos + 1
        while end_pos < len(content) and balance > 0:
            if content[end_pos] == '{':
                balance += 1
            elif content[end_pos] == '}':
                balance -= 1
            end_pos += 1
        if balance != 0:
            break
        struct_content = content[brace_pos + 1: end_pos - 1]
        structs.append(struct_content)
        start = end_pos
    return structs

def process_struct(struct_content):
    """Processes a single struct to extract species name, common item, and rare item."""
    species_name = None
    item_common = None
    item_rare = None
    lines = struct_content.split('\n')
    for line in lines:
        line = line.strip()
        if not line:
            continue
        # Check for species name
        if '.speciesName' in line:
            clean_line = line.split('//')[0].strip()
            match = re.search(r'\.speciesName\s*=\s*_\s*\(\s*"([^"]+)"\s*\)', clean_line)
            if match:
                species_name = match.group(1)
        # Check for itemCommon
        if line.startswith('.itemCommon'):
            parts = line.split('=', 1)
            if len(parts) > 1:
                value_part = parts[1].split('//')[0].strip().rstrip(',')
                item_common = value_part.strip()
        # Check for itemRare
        if line.startswith('.itemRare'):
            parts = line.split('=', 1)
            if len(parts) > 1:
                value_part = parts[1].split('//')[0].strip().rstrip(',')
                item_rare = value_part.strip()
    return species_name, item_common, item_rare

def main():
    output = []
    for gen in range(1, 10):
        filename = f'gen_{gen}_families.h'
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                content = f.read()
        except FileNotFoundError:
            print(f"File {filename} not found. Skipping.", file=os.sys.stderr)
            continue
        structs = parse_structs(content)
        for struct in structs:
            species_name, item_common, item_rare = process_struct(struct)
            if species_name is None:
                continue  # Skip entries without a valid species name
            common_name = item_constant_to_name(item_common) if item_common else "None"
            rare_name = item_constant_to_name(item_rare) if item_rare else "None"
            output.append((species_name, common_name, rare_name))
    # Write the results to a .txt file
    with open('pokemon_items.txt', 'w', encoding='utf-8') as f:
        for name, common, rare in output:
            f.write(f"{name}\n")
            f.write(f"Common Item - {common}\n")
            f.write(f"Rare Item - {rare}\n")
            f.write("\n")  # Empty line for separation

if __name__ == "__main__":
    main()