#!/usr/bin/env python3
"""
Copy Pokemon front sprites from the repo to web/public/sprites/ with normalized names.

Usage: python3 copy_sprites.py [repo_root]
"""

import os
import shutil
import sys


def find_repo_root(hint=None):
    """Find the repository root directory."""
    if hint and os.path.isdir(hint):
        return os.path.abspath(hint)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    candidate = os.path.normpath(os.path.join(script_dir, '..', '..'))
    if os.path.isdir(os.path.join(candidate, 'graphics', 'pokemon')):
        return candidate
    print("ERROR: Cannot find repo root. Pass it as an argument.")
    sys.exit(1)


def normalize_sprite_name(rel_parts):
    """Convert directory path parts to a Showdown-compatible sprite ID.
    Showdown strips all non-alphanumeric from base names, uses hyphens for forms.

    Examples:
        ['bulbasaur'] -> 'bulbasaur'
        ['charizard', 'mega_x'] -> 'charizard-megax'
        ['vulpix', 'alola'] -> 'vulpix-alola'
        ['mr_mime'] -> 'mrmime'
        ['ho_oh'] -> 'hooh'
        ['porygon_z'] -> 'porygonz'
        ['mr_mime', 'galar'] -> 'mrmime-galar'
    """
    if not rel_parts:
        return None

    base = rel_parts[0]

    # Base name: strip all non-alphanumeric (Showdown convention)
    name = base.replace('_', '').lower()

    # Add form suffix if present
    if len(rel_parts) > 1:
        form = rel_parts[1]
        # Form suffix: strip underscores, keep as hyphen-separated
        form_normalized = form.replace('_', '').lower()
        name = f"{name}-{form_normalized}"

    return name


def copy_sprites(repo_root):
    """Walk graphics/pokemon/ and copy front sprites to web/public/sprites/."""
    graphics_dir = os.path.join(repo_root, 'graphics', 'pokemon')
    output_dir = os.path.join(repo_root, 'web', 'public', 'sprites')
    os.makedirs(output_dir, exist_ok=True)

    copied = 0
    skipped = 0
    errors = 0

    # Walk the pokemon graphics directory
    for root, dirs, files in os.walk(graphics_dir):
        rel_path = os.path.relpath(root, graphics_dir)
        parts = rel_path.split(os.sep)

        # Skip the root graphics/pokemon/ directory itself
        if rel_path == '.':
            continue

        # Determine which sprite file to use
        sprite_file = None
        if 'anim_front.png' in files:
            sprite_file = 'anim_front.png'
        elif 'front.png' in files:
            sprite_file = 'front.png'

        if not sprite_file:
            continue

        # Skip if this is the base species dir and it's just a parent with subdirs
        # (e.g., graphics/pokemon/charizard/ has anim_front.png AND mega_x/ subdir)
        # We still want both the base and the form sprites

        # Normalize the sprite name
        sprite_id = normalize_sprite_name(parts)
        if not sprite_id:
            continue

        # Skip too-deeply-nested directories (more than species/form)
        if len(parts) > 2:
            continue

        src_path = os.path.join(root, sprite_file)
        dst_path = os.path.join(output_dir, f'{sprite_id}.png')

        try:
            shutil.copy2(src_path, dst_path)
            copied += 1
        except Exception as e:
            print(f"  Error copying {src_path}: {e}")
            errors += 1

    return copied, skipped, errors


def main():
    repo_root = find_repo_root(sys.argv[1] if len(sys.argv) > 1 else None)
    print(f"Repo root: {repo_root}")
    print(f"Source: {os.path.join(repo_root, 'graphics', 'pokemon')}")
    print(f"Output: {os.path.join(repo_root, 'web', 'public', 'sprites')}")
    print()

    print("Copying sprites...")
    copied, skipped, errors = copy_sprites(repo_root)

    print()
    print(f"Done! Copied {copied} sprites.")
    if errors:
        print(f"  {errors} errors occurred.")


if __name__ == '__main__':
    main()
