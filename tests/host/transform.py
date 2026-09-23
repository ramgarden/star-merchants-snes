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


def transform(src: str) -> str:
    # read_pads hardware branch -> injected pad word (BEFORE REG rewrite)
    src = re.sub(
        r"    while \(!\(REG_HVBJOY & 0x01u\)\) \{\n    \}\n"
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
        r"    while \(!\(REG_HVBJOY & 0x01u\)\) \{\n    \}\n"
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
