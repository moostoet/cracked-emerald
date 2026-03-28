# Fix Protect AI blocking ally beneficial-hit setups in doubles

## Description
This updates doubles AI Protect scoring so a battler will not choose Protect when a faster ally has already committed to a beneficial move into that slot.

The new check covers direct ally-targeted coordination as well as spread-move setups that intentionally trigger the partner's ability or Weakness Policy. This fixes the reported Beat Up + Justified case from #9238 without changing unrelated Protect behavior.

This PR also adds a regression test for the Beat Up + Justified interaction.

Verified with:
`make -j32 check TESTS="AI will not use Protect if its ally is about to trigger Justified with Beat Up"`

## Media
Existing reproduction video from the issue:
https://github.com/user-attachments/assets/203adf4c-6c1c-4cf7-b986-5521793d0216

## Issue(s) that this PR fixes
Fixes #9238.

## Feature(s) this PR does NOT handle:
- This only changes Protect scoring when an ally has already committed to a beneficial hit on its partner.
- It does not broaden unrelated doubles AI coordination logic outside that Protect decision.

## Things to note in the release changelog:
- Fixed a doubles AI case where Protect could block an ally's beneficial self-hit setup, including Beat Up + Justified.
- Added a regression test for the Protect vs ally beneficial-hit interaction.

## Discord contact info
`<your Discord username>`
