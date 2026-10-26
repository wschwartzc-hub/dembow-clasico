#!/bin/bash
# Arma el instalador de macOS (.pkg) y el disco (.dmg) a partir de los plugins ya compilados.
#
# Uso (desde la raíz del proyecto, después de compilar en Release):
#   JUCE_DIR=build/_deps/juce-src bash installer/build_mac_installer.sh
#
# Variables opcionales:
#   ARTEFACTS_DIR           carpeta con VST3/, AU/ y Standalone/ (por defecto build/DembowClasico_artefacts/Release)
#   OUT_DIR                 dónde dejar el resultado (por defecto dist/)
#   DEVELOPER_ID_APP        "Developer ID Application: Nombre (TEAMID)"  -> firma los plugins y la app
#   DEVELOPER_ID_INSTALLER  "Developer ID Installer: Nombre (TEAMID)"    -> firma el .pkg
#   NOTARY_APPLE_ID, NOTARY_PASSWORD, NOTARY_TEAM_ID                    -> notariza con Apple
# Sin certificados se firma "ad-hoc": funciona, pero macOS pedirá permiso la primera vez.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck disable=SC1091
source "$ROOT/product.env"

VERSION="$(sed -nE 's/^project\(DembowClasico VERSION ([0-9.]+).*/\1/p' "$ROOT/CMakeLists.txt")"
ART="${ARTEFACTS_DIR:-$ROOT/build/DembowClasico_artefacts/Release}"
OUT="${OUT_DIR:-$ROOT/dist}"
JUCE_DIR="${JUCE_DIR:-$ROOT/build/_deps/juce-src}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

VST3="$ART/VST3/$PRODUCT_NAME.vst3"
AU="$ART/AU/$PRODUCT_NAME.component"
APP="$ART/Standalone/$PRODUCT_NAME.app"
SAFE_NAME="${PRODUCT_NAME// /-}"
PKG_OUT="$OUT/$SAFE_NAME-$VERSION-macOS.pkg"
DMG_OUT="$OUT/$SAFE_NAME-$VERSION-macOS.dmg"

echo "==> $PRODUCT_NAME $VERSION"
for b in "$VST3" "$AU" "$APP"; do
    [ -d "$b" ] || { echo "Falta $b (¿compilaste en Release?)"; exit 1; }
done
mkdir -p "$OUT" "$WORK/pkgs" "$WORK/res"

# --- 1. Copiar y firmar ------------------------------------------------------------------------
sign_bundle() {
    local bundle="$1" entitlements="${2:-}"
    xattr -cr "$bundle"
    if [ -n "${DEVELOPER_ID_APP:-}" ]; then
        if [ -n "$entitlements" ]; then
            codesign --force --options runtime --timestamp --entitlements "$entitlements" --sign "$DEVELOPER_ID_APP" "$bundle"
        else
            codesign --force --options runtime --timestamp --sign "$DEVELOPER_ID_APP" "$bundle"
        fi
    else
        codesign --force --deep --sign - "$bundle"
    fi
    codesign --verify --deep --strict --verbose=1 "$bundle"
}

for part in vst3 au app; do mkdir -p "$WORK/root-$part"; done
ditto "$VST3" "$WORK/root-vst3/$PRODUCT_NAME.vst3"
ditto "$AU"   "$WORK/root-au/$PRODUCT_NAME.component"
ditto "$APP"  "$WORK/root-app/$PRODUCT_NAME.app"

echo "==> Firmando (${DEVELOPER_ID_APP:-ad-hoc})"
sign_bundle "$WORK/root-vst3/$PRODUCT_NAME.vst3"
sign_bundle "$WORK/root-au/$PRODUCT_NAME.component"
sign_bundle "$WORK/root-app/$PRODUCT_NAME.app" "$ROOT/installer/entitlements-app.plist"

# --- 2. Paquetes de cada componente -------------------------------------------------------------
make_component() {
    local part="$1" location="$2" scripts="${3:-}"
    local plist="$WORK/$part.plist"
    pkgbuild --analyze --root "$WORK/root-$part" "$plist" >/dev/null
    # que el instalador no "reubique" el plugin si encuentra otra copia en otro lado
    plutil -replace 0.BundleIsRelocatable -bool NO "$plist"
    local args=(--root "$WORK/root-$part" --component-plist "$plist"
                --identifier "$BUNDLE_ID.$part" --version "$VERSION" --install-location "$location")
    if [ -n "$scripts" ]; then
        chmod +x "$scripts"/*
        args+=(--scripts "$scripts")
    fi
    pkgbuild "${args[@]}" "$WORK/pkgs/$part.pkg"
}

echo "==> Paquetes"
make_component vst3 "/Library/Audio/Plug-Ins/VST3"
make_component au   "/Library/Audio/Plug-Ins/Components" "$ROOT/installer/scripts/au"
make_component app  "/Applications"

# --- 3. Textos, licencia y distribution.xml -----------------------------------------------------
python3 "$ROOT/installer/make_resources.py" --juce "$JUCE_DIR" --out "$WORK/res" --version "$VERSION"
mv "$WORK/res/distribution.xml" "$WORK/distribution.xml"
mv "$WORK/res/uninstall.command" "$WORK/uninstall.command"
mv "$WORK/res/Licencias.txt" "$WORK/Licencias.txt"

# fondo nítido en pantallas Retina: un TIFF con la versión normal y la @2x
if command -v tiffutil >/dev/null 2>&1; then
    for bg in background background-dark; do
        if tiffutil -cathidpicheck "$ROOT/installer/assets/$bg.png" "$ROOT/installer/assets/$bg@2x.png" \
                    -out "$WORK/res/$bg.tiff" >/dev/null 2>&1; then
            rm -f "$WORK/res/$bg.png"
            sed -i '' "s|file=\"$bg.png\" mime-type=\"image/png\"|file=\"$bg.tiff\" mime-type=\"image/tiff\"|" "$WORK/distribution.xml"
        fi
    done
fi

# --- 4. Instalador final ------------------------------------------------------------------------
echo "==> Instalador"
pb_args=(--distribution "$WORK/distribution.xml" --resources "$WORK/res" --package-path "$WORK/pkgs")
if [ -n "${DEVELOPER_ID_INSTALLER:-}" ]; then
    pb_args+=(--sign "$DEVELOPER_ID_INSTALLER" --timestamp)
fi
rm -f "$PKG_OUT"
productbuild "${pb_args[@]}" "$PKG_OUT"

notarize() {
    local file="$1"
    if [ -n "${NOTARY_APPLE_ID:-}" ] && [ -n "${NOTARY_PASSWORD:-}" ] && [ -n "${NOTARY_TEAM_ID:-}" ]; then
        echo "==> Notarizando $(basename "$file")"
        xcrun notarytool submit "$file" --apple-id "$NOTARY_APPLE_ID" --password "$NOTARY_PASSWORD" \
                                --team-id "$NOTARY_TEAM_ID" --wait
        xcrun stapler staple "$file"
    fi
}

if [ -n "${DEVELOPER_ID_INSTALLER:-}" ]; then
    pkgutil --check-signature "$PKG_OUT"
    notarize "$PKG_OUT"
fi

# --- 5. Disco .dmg con instalador, desinstalador y licencias -------------------------------------
echo "==> Disco"
DMG_DIR="$WORK/dmg"
mkdir -p "$DMG_DIR"
cp "$PKG_OUT" "$DMG_DIR/Instalar $PRODUCT_NAME.pkg"
cp "$WORK/uninstall.command" "$DMG_DIR/Desinstalar $PRODUCT_NAME.command"
chmod +x "$DMG_DIR/Desinstalar $PRODUCT_NAME.command"
cp "$WORK/Licencias.txt" "$DMG_DIR/Licencias.txt"

rm -f "$DMG_OUT"
hdiutil create -volname "$PRODUCT_NAME $VERSION" -srcfolder "$DMG_DIR" -fs HFS+ -format UDZO -ov "$DMG_OUT" >/dev/null

if [ -n "${DEVELOPER_ID_APP:-}" ]; then
    codesign --force --timestamp --sign "$DEVELOPER_ID_APP" "$DMG_OUT"
    notarize "$DMG_OUT"
fi

cp "$WORK/Licencias.txt" "$OUT/Licencias.txt"
echo "==> Listo:"
ls -la "$OUT"
