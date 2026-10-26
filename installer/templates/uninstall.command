#!/bin/bash
# Desinstalador de {{PRODUCT_NAME}}. Haz doble clic para ejecutarlo; te pedirá tu contraseña de Mac.

clear
echo ""
echo "  Desinstalar {{PRODUCT_NAME}}"
echo "  ------------------------------------------------------------"
echo "  Se borrarán:"
echo "    /Library/Audio/Plug-Ins/VST3/{{PRODUCT_NAME}}.vst3"
echo "    /Library/Audio/Plug-Ins/Components/{{PRODUCT_NAME}}.component"
echo "    /Applications/{{PRODUCT_NAME}}.app"
echo ""
echo "  Tus presets en Documentos/{{PRODUCT_NAME}} NO se borran."
echo ""
read -r -p "  ¿Continuar? (s/n) " answer
case "$answer" in
    [sS]*) ;;
    *) echo "  Cancelado."; exit 0 ;;
esac

echo ""
echo "  Escribe tu contraseña de Mac si te la pide (no se ve mientras escribes):"
sudo rm -rf "/Library/Audio/Plug-Ins/VST3/{{PRODUCT_NAME}}.vst3" \
            "/Library/Audio/Plug-Ins/Components/{{PRODUCT_NAME}}.component" \
            "/Applications/{{PRODUCT_NAME}}.app" || { echo "  No se pudo desinstalar."; exit 1; }

for part in vst3 au app; do
    sudo pkgutil --forget "{{BUNDLE_ID}}.$part" >/dev/null 2>&1 || true
done
killall -9 AudioComponentRegistrar >/dev/null 2>&1 || true

echo ""
echo "  Listo: {{PRODUCT_NAME}} se desinstaló."
echo ""
