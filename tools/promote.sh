#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

src="${1:-Linux_Version}"
linux_only=(tools/promote.sh)
windows_includes=(
	'src/MQEQGFX.h|#include <mq/Plugin.h>|#include "Detours.h"'
	'src/ui/Suppress.cpp|#include "Suppress.h"|#include "../Detours.h"'
)

restore_include() {
	local file="$1" anchor="$2" line="$3"
	grep -qF "$line" "$file" && return
	if ! grep -qF "$anchor" "$file"; then
		echo "$file: '$anchor' not found, cannot restore '$line'" >&2
		exit 1
	fi
	awk -v a="$anchor" -v l="$line" '{ print } !done && index($0, a) == 1 { print l; done = 1 }' "$file" > "$file.promote"
	mv "$file.promote" "$file"
	git add "$file"
}

if [[ -n "$(git status --porcelain)" ]]; then
	echo "working tree not clean" >&2
	exit 1
fi

start="$(git rev-parse --abbrev-ref HEAD)"
trap 'git checkout -q "$start"' EXIT

excludes=()
for f in "${linux_only[@]}"; do excludes+=(":(exclude)$f"); done

git checkout -q main
git checkout "$src" -- . "${excludes[@]}"
git diff --name-only --diff-filter=D main "$src" -- . "${excludes[@]}" | xargs -r git rm -q --
for f in "${linux_only[@]}"; do git rm -q --cached --ignore-unmatch "$f" >/dev/null; rm -f "$f"; done
for entry in "${windows_includes[@]}"; do
	IFS='|' read -r file anchor line <<< "$entry"
	restore_include "$file" "$anchor" "$line"
done
if git diff --cached --quiet; then
	echo "main already matches $src"
else
	git commit -q -m "Sync from $src"
	echo "main <- $src"
fi

git checkout -q "$src"
git merge -q --no-edit -s ours main
echo "$src <- main"
