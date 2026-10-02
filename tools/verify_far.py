"""Far-data layout guard: parses src/far_data.s exactly like the host
transform (tests/host/transform.py::parse_far_bytes) and asserts the
documented M8 field layout. Hand-counted .byte padding once silently
shifted every M8 field by 2 bytes (all far reads returned wrong data
while pure-state logic stayed green); this gate fails the build if any
field drifts from its contract offset again.

Usage: python tools/verify_far.py  (exit 0 = layout exact)
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FAR = ROOT / "src" / "far_data.s"

# (offset from FARRODATA start, expected leading bytes)
FIELDS = [
    (0, b"\x03\x04"),                    # tour data starts (scf[0..1])
    (580, None),                         # tour4 base (size-checked only)
    (792 + 0, b"ASS TRA\x00"),
    (792 + 8, b"BAT CRU\x00"),
    (792 + 16, b"DREADNT\x00"),
    (792 + 24, b"TRIBUTE PAID 500CR\x00"),
    (792 + 48, b"TRIBUTE UNPAID: FIGHT!\x00"),
    (792 + 72, b"FERRENGI STAND DOWN\x00"),
    (792 + 96, b"FERRENGAL DEFENDED\x00"),
    (792 + 120, b"RETURN WITH 3000 FTRS\x00"),
    (792 + 144, b"FERRENGAL\x00"),
    (792 + 156, b"VORLON\x00"),
    (792 + 168, b"ASGARD\x00"),
    (792 + 180, b"ELDARI\x00"),
    (792 + 192, b"KRULL\x00"),
    (792 + 204, b"MORGU\x00"),
    (792 + 216, b"XARTH\x00"),
    (792 + 228, bytes([118, 12])),
    (792 + 240, b"ENGAGING ENEMY!\x00"),
    (792 + 264, b"PREPARE FOR BATTLE\x00"),
    (1080 + 0, b"SCOUT\x00"),
    (1080 + 12, b"MERCHANT\x00"),
    (1080 + 24, b"FREIGHTER\x00"),
    (1080 + 36, b"DREAD\x00"),
    (1080 + 48, bytes([10, 15, 0, 0])),
    (1080 + 52, bytes([20, 30, 136, 19])),
    (1080 + 56, bytes([40, 50, 136, 19])),
    (1080 + 60, bytes([60, 80, 48, 117])),
    (1080 + 64, b"CORP CHARTER 5000\x00"),
    (1080 + 88, b"EXCHANGE SHIP\x00"),
    (1080 + 112, b"BUY PROBE 500\x00"),
    (1080 + 136, b"BUY BEACON 100\x00"),
    (1080 + 160, b"BUY GENESIS 5000\x00"),
    (1080 + 184, b"BUY PHOTON 500\x00"),
    (1080 + 208, b"CORP CHARTERED!\x00"),
    (1080 + 232, b"NEED 5000 CREDITS\x00"),
    (1080 + 256, b"CORP FLEET ACTIVE\x00"),
    (1080 + 280, b"HULL SWAPPED\x00"),
    (1080 + 304, b"CANT AFFORD HULL\x00"),
    (1080 + 328, b"CARGO WONT FIT\x00"),
    (1080 + 352, b"OVERFLOW TO POOL\x00"),
    (1080 + 376, b"POOL TAPPED\x00"),
]
TOTAL = 792 + 288 + 400 + 88 + 112  # + tour8 (56 frames + 56 pads)


def parse_far_bytes(s: str) -> list:
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == '"':
            j = s.index('"', i + 1)
            out += [ord(ch) for ch in s[i + 1:j]]
            i = j + 1
        elif c in ", \t":
            i += 1
        else:
            m = re.match(r"(-?\d+)", s[i:])
            out.append(int(m.group(1)) & 0xFF)
            i += m.end()
    return out


def main() -> int:
    vals = []
    for line in FAR.read_text().splitlines():
        m = re.match(r"\s*\.byte\s+(.*)", line)
        if m:
            vals += parse_far_bytes(m.group(1))
    img = bytes(vals)
    fails = []
    if len(img) != TOTAL:
        fails.append("total %d != %d" % (len(img), TOTAL))
    for off, want in FIELDS:
        if want is None:
            continue
        got = img[off:off + len(want)]
        if got != want:
            fails.append("offset %d: got %r want %r" % (off, got, want))
    if fails:
        for f in fails:
            print("verify_far FAIL:", f)
        return 1
    print("verify_far: far layout exact (%d bytes, %d fields)"
          % (len(img), len(FIELDS)))
    return 0


def check_farcode_calls() -> int:
    """Every call in bank-1 C must stay in bank 1.

    A bank-1 jsr to a bank-0 address (none.lib helpers like shrax /
    mulax / incaxy, or the bare farbyt leaf) lands in bank-1 zeros
    ($01E8xx = BRK-sled) instead of bank-0 code: wander, garbage
    reads, occasional hangs (root-caused 2026-09-28, M8). So m8.c may
    only call its own functions + farbyt1 (bank-1 twin leaf).
    Checked on compiler output (exact, no byte-guessing).
    """
    import subprocess
    import tempfile
    cc65 = Path("C:/cc65/bin/cc65.exe")
    with tempfile.TemporaryDirectory() as td:
        out = str(Path(td) / "m8check.s")
        r = subprocess.run(
            [str(cc65), "-t", "none", "--cpu", "65816",
             "-Isrc", "-O",
             "--code-name", "FARCODE", "--rodata-name", "FARRODATA",
             "-o", out, "src/m8.c"],
            capture_output=True, text=True)
        if r.returncode != 0:
            print("verify_far FAIL: m8.c does not compile")
            print(r.stderr[-2000:])
            return 1
        bad = []
        for line in Path(out).read_text().splitlines():
            m = re.match(r"\s*jsr\s+(\S+)", line)
            if not m:
                continue
            tgt = m.group(1)
            if tgt.startswith("_m8") or tgt.startswith("_m9") or tgt in ("_farbyt1",):
                continue
            bad.append(tgt)
    if bad:
        for b in sorted(set(bad)):
            print("verify_far FAIL: bank-1 call to bank-0 symbol:", b)
        return 1
    print("verify_far: m8.c calls all same-bank")
    return 0


if __name__ == "__main__":
    rc = main()
    if rc == 0:
        rc = check_farcode_calls()
    sys.exit(rc)
