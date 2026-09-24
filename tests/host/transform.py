"""Host-test transform: copies src/main.c to game.c with replacements that
let the REAL game logic run on a 6502/py65 host model.

Rules (applied in order):
- strip file-scope `static ` (host test links game functions/globals)
- REG_* hardware macros -> host model lvalues (writes vanish, reads zero)
- wait_vblank body -> gsfr++ only (no $4212 polling on host)
- read_pads hardware branch -> injectable hj_pad (tests drive edges)
- draw_text body -> gdx advance only (layout logic kept, no VRAM model)
- clear_map body -> empty (pure overhead on host)
- int main(void) -> rom_main (never called; test_main drives)
- inject extern decls for host-provided symbols
"""
import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src" / "main.c"
OUT = Path(__file__).resolve().parent / "out" / "game.c"

REGS_DUMMY = ["INIDISP", "BGMODE", "BG1SC", "BG12NBA", "BG1HOFS",
              "BG1VOFS", "VMAIN", "VMADDL", "VMADDH", "VMDATAL",
              "VMDATAH", "CGADD", "CGDATA", "TM", "NMITIMEN",
              "HVBJOY", "JOY1L", "JOY1H"]

# Offset of FARRODATA tour data from $018000 (must match FT_BASE in
# src/main.c: bank-1 FARFONT occupies $018000-$018BFF).
FAR_BASE = 0x0C00
# Highest reachable tour index: FT_SCP6 + 13 (selfdrive 6 tail). The
# image is zero-padded to cover it (hardware would return bank-1
# padding bytes there).
FAR_SIZE = FAR_BASE + 788


def far_image() -> list:
    """Flat $018000-based image of src/far_data.s tour bytes.

    Single source of truth stays in far_data.s; the host gets a
    byte-identical copy so farbyt() behaves exactly as on hardware.
    """
    vals = []
    for line in (ROOT / "src" / "far_data.s").read_text().splitlines():
        m = re.match(r"\s*\.byte\s+(.*)", line)
        if m:
            vals += [int(x) & 0xFF for x in m.group(1).split(",")]
    img = [0] * FAR_BASE + vals
    while len(img) < FAR_SIZE:
        img.append(0)
    return img


def transform(src: str) -> str:
    # farbyt(): serve the real bank-1 tour bytes on the host. Replaces
    # the asm leaf helper (src/far_tbl.s, never linked into test.bin).
    img = far_image()
    table = ("const uint8_t host_far[%d] = {%s};\n"
             "uint8_t farbyt(void) {\n"
             "    return host_far[gfar_o];\n}" %
             (len(img), ",".join("%du" % b for b in img)))
    src = src.replace("uint8_t farbyt(void);", table, 1)
    # read_pads hardware branch -> injected pad word (BEFORE REG rewrite)
    src = re.sub(
        r"    while \(REG_HVBJOY & 0x01u\) \{\n    \}\n"
        r"    gj_pad = \(uint16_t\)REG_JOY1L;\n"
        r"    gj_pad \|= \(uint16_t\)\(\(uint16_t\)REG_JOY1H << 8\);",
        "    gj_pad = hj_pad;", src, count=1)
    for r in REGS_DUMMY:
        src = re.sub(r"#define REG_" + r + r" .*",
                     "#define REG_" + r + " (host_dummy)", src)
    # wait_vblank -> tick clock only
    src = re.sub(
        r"static void wait_vblank\(void\) \{.*?\n\}",
        "void wait_vblank(void) {\n    gsfr++;\n}", src,
        flags=re.S, count=1)
    # read_pads hardware branch -> injected pad word
    src = re.sub(
        r"    while \(REG_HVBJOY & 0x01u\) \{\n    \}\n"
        r"    gj_pad = \(uint16_t\)REG_JOY1L;\n"
        r"    gj_pad \|= \(uint16_t\)\(\(uint16_t\)REG_JOY1H << 8\);",
        "    gj_pad = hj_pad;", src, count=1)
    # draw_text -> cursor advance only
    src = re.sub(
        r"static void draw_text\(void\) \{.*?\n\}",
        "void draw_text(void) {\n"
        "    gth = 0u;\n"
        "    while (gdstr[gth]) gth++;\n"
        "    gdx += (uint8_t)gth;\n"
        "}", src, flags=re.S, count=1)
    # clear_map -> empty
    src = re.sub(
        r"static void clear_map\(void\) \{.*?\n\}",
        "void clear_map(void) {\n}", src, flags=re.S, count=1)
    # entry point rename (never called by tests)
    src = src.replace("int main(void) {", "int rom_main(void) {", 1)
    # strip remaining file-scope statics for linkage
    src = re.sub(r"^static ", "", src, flags=re.M)
    # host-provided symbols
    src = src.replace(
        "extern const uint8_t font_pic[3072];",
        "extern const uint8_t font_pic[3072];\n"
        "extern uint8_t host_dummy;\n"
        "extern uint16_t hj_pad;\n"
        "extern uint8_t gth;", 1)
    return src


def main() -> None:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(transform(SRC.read_text()))
    print("wrote", OUT)


if __name__ == "__main__":
    main()
