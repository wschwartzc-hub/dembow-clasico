#!/usr/bin/env python3
"""Genera los textos del instalador (bienvenida, licencia, léeme, final), la licencia en texto
plano y el distribution.xml, a partir de product.env, legal/EULA.md y las licencias de terceros.

Uso:
  python3 installer/make_resources.py --juce RUTA_A_JUCE --out CARPETA [--version 1.0.0]
"""
import argparse, datetime, html, os, re, shutil, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def read_env(path):
    env = {}
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = re.match(r'^([A-Z_]+)="?(.*?)"?\s*$', line.strip())
            if m and not line.lstrip().startswith("#"):
                env[m.group(1)] = m.group(2)
    return env


def project_version():
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r"project\(\s*DembowClasico\s+VERSION\s+([0-9.]+)", f.read())
    return m.group(1) if m else "1.0.0"


def fill(text, v):
    return re.sub(r"\{\{([A-Z_]+)\}\}", lambda m: v.get(m.group(1), ""), text)


# --- licencias de terceros ------------------------------------------------------------------
def section(text, start, stop_regex):
    i = text.find(start)
    if i < 0:
        return text
    rest = text[i:]
    m = re.search(stop_regex, rest[len(start):])
    return rest if not m else rest[: len(start) + m.start()]


def third_party(juce):
    def rd(*p):
        path = os.path.join(*p)
        with open(path, encoding="utf-8", errors="replace") as f:
            return f.read().strip()

    m = os.path.join(juce, "modules")
    items = [
        ("JUCE",
         "Este producto usa el framework JUCE (© Raw Material Software Limited), licenciado bajo el "
         "JUCE 8 End User Licence Agreement (https://juce.com/legal/juce-8-licence/)."),
        ("VST 3 SDK (Steinberg Media Technologies GmbH) — Licencia MIT",
         rd(m, "juce_audio_processors_headless", "format_types", "VST3_SDK", "LICENSE.txt")),
        ("AudioUnitSDK (Apple Inc.) — Apache License 2.0",
         rd(m, "juce_audio_plugin_client", "AU", "AudioUnitSDK", "LICENSE.txt")),
        ("HarfBuzz — Licencia MIT (antigua)",
         rd(m, "juce_graphics", "fonts", "harfbuzz", "COPYING")),
        ("SheenBidi — Apache License 2.0",
         rd(m, "juce_graphics", "unicode", "sheenbidi", "LICENSE")),
        ("libpng — PNG Reference Library License",
         rd(m, "juce_graphics", "image_formats", "pnglib", "LICENSE")),
        ("Independent JPEG Group (libjpeg)",
         "Este software se basa en parte en el trabajo del Independent JPEG Group.\n\n"
         + section(rd(m, "juce_graphics", "image_formats", "jpglib", "README"), "LEGAL ISSUES", r"\n[A-Z][A-Z ]+\n=+\n")),
        ("zlib",
         section(rd(m, "juce_core", "zip", "zlib", "README"), "Copyright notice", r"\Z")),
        ("Tipografía Barlow — SIL Open Font License 1.1",
         rd(ROOT, "Resources", "fonts", "OFL.txt")),
    ]
    return items


# --- markdown mínimo → HTML -------------------------------------------------------------------
def md_to_html(md):
    out, in_list = [], False
    for block in md.strip().split("\n\n"):
        lines = block.strip().split("\n")
        if all(l.startswith("- ") for l in lines):
            out.append("<ul>" + "".join(f"<li>{html.escape(l[2:])}</li>" for l in lines) + "</ul>")
        elif lines[0].startswith("# "):
            out.append(f"<h1>{html.escape(lines[0][2:])}</h1>")
        elif lines[0].startswith("## "):
            out.append(f"<h2>{html.escape(lines[0][3:])}</h2>")
        else:
            out.append("<p>" + "<br>".join(html.escape(l) for l in lines) + "</p>")
    return "\n".join(out)


def md_to_text(md):
    t = re.sub(r"^# (.*)$", lambda m: m.group(1).upper() + "\n" + "=" * len(m.group(1)), md, flags=re.M)
    t = re.sub(r"^## (.*)$", lambda m: m.group(1) + "\n" + "-" * len(m.group(1)), t, flags=re.M)
    return t


def page(title, body, v):
    css_path = os.path.join(ROOT, "installer", "templates", "style.css")
    with open(css_path, encoding="utf-8") as f:
        css = f.read()
    return (f'<!DOCTYPE html>\n<html lang="es"><head><meta charset="utf-8">'
            f'<meta name="color-scheme" content="light dark"><title>{html.escape(title)}</title>'
            f"<style>{css}</style></head><body>\n{body}\n</body></html>\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--juce", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--version", default=None)
    a = ap.parse_args()

    env = read_env(os.path.join(ROOT, "product.env"))
    v = dict(env)
    v["VERSION"] = a.version or project_version()
    v["YEAR"] = str(datetime.date.today().year)
    contact = []
    if env.get("CONTACT_EMAIL"):
        contact.append("correo: " + env["CONTACT_EMAIL"])
    if env.get("WEBSITE"):
        contact.append("sitio web: " + env["WEBSITE"])
    v["CONTACT_SECTION"] = ("## 14. Contacto\n\nPara cualquier asunto relacionado con este contrato, escribe al Titular ("
                            + ", ".join(contact) + ").") if contact else ""

    os.makedirs(a.out, exist_ok=True)
    with open(os.path.join(ROOT, "legal", "EULA.md"), encoding="utf-8") as f:
        eula_md = fill(f.read(), v).strip()

    tp = third_party(a.juce)

    # licencia (HTML para el instalador)
    tp_html = "".join(f"<h3>{html.escape(name)}</h3><pre>{html.escape(text)}</pre>" for name, text in tp)
    lic_body = (md_to_html(eula_md)
                + "<hr><h2>Avisos de terceros</h2><p>El Software incluye los siguientes componentes de terceros, "
                  "distribuidos bajo sus propias licencias:</p>" + tp_html)
    with open(os.path.join(a.out, "license.html"), "w", encoding="utf-8") as f:
        f.write(page("Licencia", lic_body, v))

    # licencia (texto plano para el DMG / zip)
    txt = md_to_text(eula_md) + "\n\n\nAVISOS DE TERCEROS\n==================\n"
    for name, text in tp:
        txt += f"\n\n{name}\n{'-' * len(name)}\n\n{text}\n"
    with open(os.path.join(a.out, "Licencias.txt"), "w", encoding="utf-8") as f:
        f.write(txt)

    # bienvenida, léeme, final y distribution.xml
    tdir = os.path.join(ROOT, "installer", "templates")
    for name, title in [("welcome", "Bienvenida"), ("readme", "Léeme"), ("conclusion", "Listo")]:
        with open(os.path.join(tdir, name + ".html"), encoding="utf-8") as f:
            body = fill(f.read(), v)
        with open(os.path.join(a.out, name + ".html"), "w", encoding="utf-8") as f:
            f.write(page(title, body, v))

    for img in ("background.png", "background-dark.png"):
        shutil.copy(os.path.join(ROOT, "installer", "assets", img), os.path.join(a.out, img))

    with open(os.path.join(tdir, "distribution.xml"), encoding="utf-8") as f:
        dist = fill(f.read(), {k: html.escape(val) for k, val in v.items()})
    with open(os.path.join(a.out, "distribution.xml"), "w", encoding="utf-8") as f:
        f.write(dist)

    with open(os.path.join(tdir, "uninstall.command"), encoding="utf-8") as f:
        un = fill(f.read(), v)
    with open(os.path.join(a.out, "uninstall.command"), "w", encoding="utf-8") as f:
        f.write(un)

    print("recursos listos en", a.out, "· versión", v["VERSION"])


if __name__ == "__main__":
    sys.exit(main())
