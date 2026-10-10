#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

src="${1:-Linux_Version}"
linux_only=(deploy.sh tools/promote.sh)

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
if git diff --cached --quiet; then
	echo "main already matches $src"
else
	git commit -q -m "Sync from $src"
	echo "main <- $src"
fi

for b in dev-windows Linux_Version; do
	if git show-ref --verify --quiet "refs/heads/$b"; then
		git checkout -q "$b"
		git merge -q --no-edit main
		echo "$b <- main"
	fi
done
