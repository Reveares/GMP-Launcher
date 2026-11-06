#!/bin/bash
# Runs gmpinjector.exe inside the Proton prefix of Steam's Gothic II. GMP_PROTON=<Proton directory>
# overrides the Proton build Steam has picked for the game.
set -euo pipefail

app_dir=$(cd "$(dirname "$0")" && pwd)

resolved=$(python3 - <<'PY'
import os, re, sys

APP_ID = "39510"  # Gothic II: Gold Edition

def fail(message):
    print(message, file=sys.stderr)
    sys.exit(1)

def parse_vdf(path):
    """Steam's text KeyValues as nested dicts, keys lowercased: Steam matches them case-insensitively."""
    try:
        text = open(path, encoding="utf-8", errors="replace").read()
    except OSError:
        return {}
    root, stack, key = {}, [], None
    node = root
    for match in re.finditer(r'"((?:[^"\\]|\\.)*)"|([{}])|//[^\n]*', text):
        string, brace = match.group(1), match.group(2)
        if brace == "{":
            child = {}
            if key is not None:
                node[key.lower()] = child
            stack.append(node)
            node, key = child, None
        elif brace == "}":
            if not stack:
                break
            node, key = stack.pop(), None
        elif string is None:
            continue
        elif key is None:
            key = string
        else:
            node[key.lower()] = string.replace("\\\\", "\\")
            key = None
    return root

def dig(node, *keys):
    for key in keys:
        if not isinstance(node, dict):
            return None
        node = node.get(key.lower())
    return node

home = os.path.expanduser("~")
candidates = [
    f"{home}/.steam/steam",
    f"{home}/.local/share/Steam",
    f"{home}/.steam/debian-installation",
    f"{home}/.var/app/com.valvesoftware.Steam/.local/share/Steam",
    f"{home}/snap/steam/common/.local/share/Steam",
]
roots = list(dict.fromkeys(os.path.realpath(c) for c in candidates if os.path.isdir(f"{c}/steamapps")))
if not roots:
    fail("Steam was not found.")

def libraries(root):
    folders = dig(parse_vdf(f"{root}/steamapps/libraryfolders.vdf"), "libraryfolders") or {}
    paths = [root] + [f["path"] for f in folders.values() if isinstance(f, dict) and "path" in f]
    return list(dict.fromkeys(os.path.realpath(p) for p in paths))

game = next(((root, library) for root in roots for library in libraries(root)
             if os.path.isfile(f"{library}/steamapps/appmanifest_{APP_ID}.acf")), None)
if not game:
    fail("Gothic II: Gold Edition is not installed in Steam.")
steam_root, library = game
# The prefix lives in the library that holds the game, not in the Steam root.
compat_data = f"{library}/steamapps/compatdata/{APP_ID}"
if not os.path.isdir(f"{compat_data}/pfx"):
    fail("Gothic II has no Proton prefix yet. Start it once from Steam, then try again.")

def proton_builds():
    """(name, directory): a custom build by the internal name CompatToolMapping stores, Valve's by folder."""
    builds = []
    tools_dir = f"{steam_root}/compatibilitytools.d"
    for entry in sorted(os.listdir(tools_dir)) if os.path.isdir(tools_dir) else []:
        manifest_dir = f"{tools_dir}/{entry}"
        tools = dig(parse_vdf(f"{manifest_dir}/compatibilitytool.vdf"), "compatibilitytools", "compat_tools") or {}
        for name, tool in tools.items():
            if isinstance(tool, dict):
                builds.append((name, os.path.normpath(f"{manifest_dir}/{tool.get('install_path', '.')}")))
    for lib in libraries(steam_root):
        common = f"{lib}/steamapps/common"
        for entry in sorted(os.listdir(common)) if os.path.isdir(common) else []:
            if entry.startswith("Proton"):
                builds.append((entry.lower(), f"{common}/{entry}"))
    return [(name, path) for name, path in builds if os.path.isfile(f"{path}/proton")]

def mapped_build(name):
    # Valve's builds are mapped by an internal name ("proton_9") that only their folder names follow.
    if name == "proton_experimental":
        folder = "proton - experimental"
    elif name == "proton_hotfix":
        folder = "proton hotfix"
    elif name.startswith("proton_"):
        folder = f"proton {name[len('proton_'):]}.0"
    else:
        folder = None
    return next((path for build, path in proton_builds()
                 if build == name or (folder and build.startswith(folder))), None)

def last_used_build():
    # config_info names files inside the build that last ran the prefix.
    try:
        lines = open(f"{compat_data}/config_info", encoding="utf-8", errors="replace").read().splitlines()
    except OSError:
        return None
    roots = (line.rsplit("/files/", 1)[0] for line in lines if "/files/" in line)
    return next((root for root in roots if os.path.isfile(f"{root}/proton")), None)

proton = os.environ.get("GMP_PROTON")
if not proton:
    mapping = dig(parse_vdf(f"{steam_root}/config/config.vdf"),
                  "InstallConfigStore", "Software", "Valve", "Steam", "CompatToolMapping") or {}
    # A mapped build that is not installed falls through to the one that last ran the prefix, not to
    # the default: Steam would not run the default either.
    name = next((dig(mapping, key, "name") for key in (APP_ID, "0") if dig(mapping, key, "name")), None)
    proton = (mapped_build(name.lower()) if name else None) or last_used_build()
if not proton or not os.path.isfile(f"{proton}/proton"):
    fail(f"No Proton build was found{f' at {proton}' if proton else ''}. "
         "Start Gothic II once from Steam, or set GMP_PROTON to a Proton directory.")

print(steam_root, compat_data, proton, sep="\n")
PY
)
{ read -r steam_root; read -r compat_data; read -r proton; } <<<"$resolved"

# While a Steam launch of the game holds the prefix (its wineserver runs in Steam's runtime container),
# injecting into a Gothic started from here fails with ERROR_ACCESS_DENIED.
if pgrep -f "SteamLaunch AppId=39510 " >/dev/null; then
    echo "Gothic II is running from Steam. Close it, including its starter window, then try again." >&2
    exit 1
fi

# GE-Proton's protonfixes for 39510 set the dsound, ddraw and dinput overrides and the latency, but
# only under Steam's own verb, waitforexitandrun. Music needs directmusic in the prefix, which those
# fixes install and Valve's Proton does not; its native dsound breaks MSS ("Broken waveOut driver").
# A native ddraw.dll is GD3D11, the renderer that keeps Alt+Tab from crashing the game. It draws
# through d3d11: Proton puts DXVK into the prefix but enables it only in the setup runinprefix skips,
# and wined3d leaves GD3D11's screen black.
export WINEDLLOVERRIDES="*dsound=b;ddraw=n,b;dinput=n,b;d3d11=n,b;dxgi=n,b;d3d10core=n,b;d3d9=n,b${WINEDLLOVERRIDES:+;$WINEDLLOVERRIDES}"
export PULSE_LATENCY_MSEC="${PULSE_LATENCY_MSEC:-90}"

# runinprefix runs wine directly, so the injector's output reaches the launcher; `run` would start it
# in a console of its own. Unlike `run`, it does not set the prefix up, hence the check above.
STEAM_COMPAT_DATA_PATH="$compat_data" \
STEAM_COMPAT_CLIENT_INSTALL_PATH="$steam_root" \
SteamAppId=39510 \
exec "$proton/proton" runinprefix "$app_dir/gmpinjector.exe" "$@"
