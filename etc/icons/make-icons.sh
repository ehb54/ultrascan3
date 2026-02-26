#!/usr/bin/env bash
set -euo pipefail

IN="${1:-us3-icon-1024x1024.png}"

OUTDIR="dist/icons"
ICONSET="dist/us3.iconset"
mkdir -p "$OUTDIR" "$ICONSET"

if [[ ! -f "$IN" ]]; then
  echo "ERROR: Input not found: $IN"
  exit 1
fi

# Sanity check: master must have transparency
if [[ "$(magick "$IN" -format "%[opaque]" info: 2>/dev/null || echo true)" == "true" ]]; then
  echo "ERROR: Master appears fully opaque (no transparency). Use the transparent master PNG."
  exit 2
fi

make_png () {
  local size="$1"
  local unsharp="$2"
  local out="$3"

  magick "$IN" \
    -colorspace sRGB \
    -alpha set \
    -filter Lanczos -define filter:lobes=3 \
    -resize "${size}x${size}" \
    -unsharp "$unsharp" \
    -strip \
    "$out"
}

# Copy canonical 1024
magick "$IN" -colorspace sRGB -alpha set -strip "$OUTDIR/us3-icon-1024x1024.png"

# Tuned sizes
make_png 512 "0x0.60+0.90+0.008" "$OUTDIR/us3-icon-512x512.png"
make_png 256 "0x0.70+1.00+0.010" "$OUTDIR/us3-icon-256x256.png"
make_png 128 "0x0.80+1.05+0.012" "$OUTDIR/us3-icon-128x128.png"
make_png 64  "0x0.90+1.10+0.015" "$OUTDIR/us3-icon-64x64.png"
make_png 48  "0x0.95+1.15+0.018" "$OUTDIR/us3-icon-48x48.png"
make_png 32  "0x1.00+1.20+0.022" "$OUTDIR/us3-icon-32x32.png"
make_png 24  "0x1.05+1.25+0.028" "$OUTDIR/us3-icon-24x24.png"
make_png 16  "0x1.10+1.30+0.035" "$OUTDIR/us3-icon-16x16.png"

# ------------------------------------------------------------------
# macOS iconset
# ------------------------------------------------------------------
cp "$OUTDIR/us3-icon-16x16.png"       "$ICONSET/icon_16x16.png"
cp "$OUTDIR/us3-icon-32x32.png"       "$ICONSET/icon_32x32.png"
cp "$OUTDIR/us3-icon-128x128.png"     "$ICONSET/icon_128x128.png"
cp "$OUTDIR/us3-icon-256x256.png"     "$ICONSET/icon_256x256.png"
cp "$OUTDIR/us3-icon-512x512.png"     "$ICONSET/icon_512x512.png"

cp "$OUTDIR/us3-icon-32x32.png"       "$ICONSET/icon_16x16@2x.png"
cp "$OUTDIR/us3-icon-64x64.png"       "$ICONSET/icon_32x32@2x.png"
cp "$OUTDIR/us3-icon-256x256.png"     "$ICONSET/icon_128x128@2x.png"
cp "$OUTDIR/us3-icon-512x512.png"     "$ICONSET/icon_256x256@2x.png"
cp "$OUTDIR/us3-icon-1024x1024.png"   "$ICONSET/icon_512x512@2x.png"

if command -v iconutil >/dev/null 2>&1; then
  iconutil -c icns "$ICONSET" -o dist/us3-icon.icns
  echo "Built dist/us3-icon.icns"
else
  echo "NOTE: iconutil not found (macOS only). iconset created at: $ICONSET"
fi

# ------------------------------------------------------------------
# Windows .ico
# ------------------------------------------------------------------
magick \
  "$OUTDIR/us3-icon-256x256.png" \
  "$OUTDIR/us3-icon-128x128.png" \
  "$OUTDIR/us3-icon-64x64.png" \
  "$OUTDIR/us3-icon-48x48.png" \
  "$OUTDIR/us3-icon-32x32.png" \
  "$OUTDIR/us3-icon-16x16.png" \
  dist/us3-icon.ico

# Optional legacy single-size ICOs (only if you need them)
magick "$OUTDIR/us3-icon-48x48.png"   dist/us3-icon-48x48.ico
magick "$OUTDIR/us3-icon-128x128.png" dist/us3-icon-128x128.ico

echo "Done."