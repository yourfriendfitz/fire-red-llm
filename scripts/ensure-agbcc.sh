#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
AGBCC_REPO="${AGBCC_REPO:-https://github.com/pret/agbcc.git}"
AGBCC_REF="${AGBCC_REF:-da598c1d918402c42c0c0d7128ba14567f3175e9}"
AGBCC_SRC_DIR="${AGBCC_SRC_DIR:-${ROOT_DIR}/third_party/agbcc}"

if ! command -v git >/dev/null 2>&1; then
  echo "error: git is required to fetch agbcc" >&2
  exit 1
fi

mkdir -p "$(dirname "${AGBCC_SRC_DIR}")"

if [ ! -d "${AGBCC_SRC_DIR}/.git" ]; then
  git clone "${AGBCC_REPO}" "${AGBCC_SRC_DIR}"
fi

git -C "${AGBCC_SRC_DIR}" fetch origin "${AGBCC_REF}"
git -C "${AGBCC_SRC_DIR}" checkout --detach "${AGBCC_REF}"

(
  cd "${AGBCC_SRC_DIR}"
  ./build.sh
  ./install.sh "${ROOT_DIR}"
)
