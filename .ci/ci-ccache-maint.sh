#!/usr/bin/env bash

set -e
set -x

# Show version
ccache --version

# Flush ccache if requested in commit message
COMMIT="${CI_PULL_REQUEST_SHA:-$CI_COMMIT}"
if git log --format=%B -n 1 "$COMMIT" | grep -q -i '\[CI\s\+ccache\s\+clear\]'; then
  echo "Flushing ccache due to commit message"
  ccache -C
fi

# Dump stats, then zero stats
ccache -s -z
