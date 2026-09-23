"""Host test runner: loads tests/host/out/test.bin into py65, resets,
steps until trep[15] (done) or the cycle budget expires (hang = FAIL),
then asserts the recorded scenario results.

Usage: python tests/host/run_tests.py [--cap N] [--smoke]
  --smoke: run 300k cycles and report gsfr (liveness check only)
"""
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
OUT = HERE / "out"

sys.path.insert(0, str(HERE))


def sym_addr(name):
    lines = (OUT / "labels.txt").read_text().splitlines()
    for line in lines:
        parts = line.split()
        if len(parts) >= 3 and parts[2] == "_" + name:
            return int(parts[1], 16)
    for line in lines:
        parts = line.split()
        if len(parts) >= 3 and parts[2] == "._" + name:
            return int(parts[1], 16)
    raise KeyError("symbol not found: " + name)


def main():
    from py65.devices.mpu6502 import MPU
    args = sys.argv[1:]
    cap = 60_000_000
    if "--cap" in args:
        cap = int(args[args.index("--cap") + 1])
    smoke = "--smoke" in args

    blob = (OUT / "test.bin").read_bytes()
    assert len(blob) == 61440, len(blob)
    mpu = MPU()
    mpu.memory[0x1000:0x1000 + len(blob)] = blob
    mpu.reset()
    # py65 reset() leaves PC at $0000; fetch the reset vector manually
    mpu.pc = mpu.memory[0xFFFC] | (mpu.memory[0xFFFD] << 8)
    trep = sym_addr("trep")
    gsfr = sym_addr("gsfr")
    steps = 0
    batch = 20000
    while steps < cap:
        for _ in range(batch):
            mpu.step()
        steps += batch
        if smoke and steps >= 300000:
            break
        if mpu.memory[trep + 15]:
            break
    lo = mpu.memory[gsfr]
    hi = mpu.memory[gsfr + 1]
    print("cycles=%d gsfr=%d done=%d" % (steps, lo + (hi << 8),
                                         mpu.memory[trep + 15]))
    if smoke:
        return 0
    if not mpu.memory[trep + 15]:
        print("FAIL: done flag never set (hang suspected)")
        return 1
    rep = [mpu.memory[trep + i] for i in range(16)]
    print("trep=", rep)
    print("tail(st,sel)@3904,3968,4032=",
          [(mpu.memory[trep + 80 + i * 2],
            mpu.memory[trep + 81 + i * 2]) for i in range(3)])
    traj = [mpu.memory[trep + 16 + i] for i in range(64)]
    print("traj(st,dept,sel,gsi)*16=")
    for i in range(0, 64, 4):
        print("  gsfr~%d: %s" % (2880 + (i // 4) * 64,
                                 (traj[i], traj[i + 1], traj[i + 2],
                                  traj[i + 3])))
    fails = []
    exp = {
        0: ("T1 M4 tour reaches loadret", 1),
        1: ("T1 gstate ST_LOADRET", 11),
        2: ("T1 gsec", 3),
        3: ("T1 gturns low byte (498&255)", 242),
        6: ("T1 gore", 4),
        7: ("T1 gorg", 3),
        8: ("T2 police commission-denial sticky", 1),
        9: ("T2 gstate ST_DOCK", 13),
        10: ("T2 gdockmsg static", 0),
        13: ("T2 gprobes", 1),
        14: ("T2 gholdmax", 25),
    }
    for idx, (label, want) in exp.items():
        got = rep[idx]
        ok = (got == want)
        print(("PASS " if ok else "FAIL ") + label + ": got %d want %d"
              % (got, want))
        if not ok:
            fails.append(label)
    # 16-bit checks
    cr = rep[4] + (rep[5] << 8)
    print(("PASS " if cr == 10006 else "FAIL ") + "T1 credits: got %d"
          % cr)
    if cr != 10006:
        fails.append("T1 credits")
    cr2 = rep[11] + (rep[12] << 8)
    print(("PASS " if cr2 == 4500 else "FAIL ") + "T2 credits: got %d"
          % cr2)
    if cr2 != 4500:
        fails.append("T2 credits")
    t3 = [mpu.memory[trep + 88 + i] for i in range(4)]
    bk = t3[1] + (t3[2] << 8)
    ok3 = (t3[0] == 1 and bk == 1165 and t3[3] == 5)
    print(("PASS " if ok3 else "FAIL ") +
          "T3 interest: flag=%d bank=%d bankday=%d" % (t3[0], bk, t3[3]))
    if not ok3:
        fails.append("T3 interest")
    t4 = [mpu.memory[trep + 96 + i] for i in range(11)]
    exp4 = [(0, 1, "flag"), (1, 9, "gstate"), (2, 3, "gsec"),
            (3, 494 - 256, "gturns"), (4, 1, "gcitadel"),
            (5, 10, "gcolship"), (6, 20, "gqsec"), (7, 413 - 256,
                                                   "gfuel"),
            (8, 11, "gpftrs"), (9, 15, "gholds"), (10, 25, "gxp")]
    for idx, want, label in exp4:
        ok = (t4[idx] == want)
        print(("PASS " if ok else "FAIL ") + "T4 %s: got %d want %d"
              % (label, t4[idx], want))
        if not ok:
            fails.append("T4 " + label)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
