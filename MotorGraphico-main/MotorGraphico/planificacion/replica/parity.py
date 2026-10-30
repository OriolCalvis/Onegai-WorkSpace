#!/usr/bin/env python3
# Adaptado de replica-skill (replica-diff/parity.py), MIT License,
# Copyright (c) 2026 Jake Schincariol. https://github.com/Jakeschincariol/replica-skill
# El texto completo de la licencia está en LICENSE_replica.txt, al lado.
"""Feature parity score for replica-diff. Standard library only.

Reads the feature matrix that replica-recon wrote and you have been filling in
while you build, and tells you how much of the original your clone does, what
is missing, ordered by priority and credit. Plan dependencies remain separate.

    python3 parity.py replica/features.csv
    python3 parity.py replica/features.csv --visual diffs/*.json
    python3 parity.py replica/features.csv --markdown > replica/parity.md
    python3 parity.py replica/features.csv --fail-under 80

The CSV has these columns (extra columns are ignored):

    feature    what it does, in plain words ("reschedule a booking")
    area       the screen or flow it belongs to ("booking page")
    priority   must | should | could   (P0 | P1 | P2 also work)
    original   yes | no     does the original app have it
    clone      yes | partial | no | skip
    notes      anything. For skip, say why ("licensed content, out of scope")

Weights: must 3, should 2, could 1. yes counts 1, partial 0.5, no 0.
skip and rows where original is "no" (features you added) are left out of the
score and listed separately, so the number only ever measures parity.

With --visual, the layout scores from imgdiff.py --json are averaged and
reported next to the feature score, and the overall score is
80% features + 20% layout. Features are what users pay for.
"""

import argparse
import csv
import json
import math
import sys

WEIGHT = {"must": 3, "should": 2, "could": 1, "p0": 3, "p1": 2, "p2": 1}
CREDIT = {"yes": 1.0, "done": 1.0, "partial": 0.5, "no": 0.0, "": 0.0, "todo": 0.0}
PRIORITY_NAME = {3: "must", 2: "should", 1: "could"}


class MatrixError(Exception):
    pass


def load(path):
    with open(path, newline="", encoding="utf-8-sig") as fh:
        reader = csv.DictReader(fh)
        if reader.fieldnames is None:
            raise MatrixError("%s is empty" % path)
        fields = [f.strip().lower() for f in reader.fieldnames]
        if len(set(fields)) != len(fields) or "" in fields:
            raise MatrixError("%s has empty or duplicate columns" % path)
        for need in ("feature", "priority", "clone"):
            if need not in fields:
                raise MatrixError("%s has no '%s' column. Columns needed: "
                                  "feature, area, priority, original, clone, notes"
                                  % (path, need))
        rows = []
        for i, raw in enumerate(reader, start=2):
            if None in raw or any(v is None for v in raw.values()):
                raise MatrixError("%s line %d: wrong number of CSV fields" % (path, i))
            row = {(k or "").strip().lower(): (v or "").strip() for k, v in raw.items()}
            if not row.get("feature"):
                continue
            row["line"] = i
            rows.append(row)
    if not rows:
        raise MatrixError("%s has no features" % path)
    return rows


def score(rows):
    counted = []
    skipped = []
    extras = []
    problems = []
    for row in rows:
        original = row.get("original", "yes").lower() or "yes"
        clone = row.get("clone", "").lower()
        prio = row.get("priority", "").lower()
        if original not in ("yes", "y", "true", "no", "n", "false"):
            raise MatrixError("line %d: invalid original '%s'" % (row["line"], original))
        if prio not in WEIGHT:
            raise MatrixError("line %d: invalid priority '%s'" % (row["line"], prio))
        if clone not in CREDIT and clone != "skip":
            raise MatrixError("line %d: invalid clone '%s'" % (row["line"], clone))
        if clone == "skip" and not row.get("notes", ""):
            raise MatrixError("line %d: skip requires a reason in notes" % row["line"])
        if original in ("no", "n", "false"):
            extras.append(row)
            continue
        if clone == "skip":
            skipped.append(row)
            continue
        w = WEIGHT.get(prio, 1)
        c = CREDIT.get(clone, 0.0)
        counted.append(dict(row, weight=w, credit=c))

    total = sum(r["weight"] for r in counted)
    got = sum(r["weight"] * r["credit"] for r in counted)
    feature_score = 100.0 * got / total if total else 0.0

    areas = {}
    for r in counted:
        a = r.get("area") or "(no area)"
        t = areas.setdefault(a, [0.0, 0.0, 0])
        t[0] += r["weight"] * r["credit"]
        t[1] += r["weight"]
        t[2] += 1
    by_area = sorted(
        ({"area": a, "score": round(100.0 * g / t, 1) if t else 0.0, "features": n}
         for a, (g, t, n) in areas.items()),
        key=lambda d: d["score"])

    missing = [r for r in counted if r["credit"] < 1.0]
    missing.sort(key=lambda r: (-r["weight"], r["credit"], r.get("area", ""), r["feature"]))

    def brief(r):
        return {"feature": r["feature"], "area": r.get("area", ""),
                "priority": PRIORITY_NAME.get(r.get("weight", WEIGHT.get(
                    r.get("priority", "").lower(), 1)), "could"),
                "clone": r.get("clone", "") or "no", "notes": r.get("notes", "")}

    musts = [r for r in counted if r["weight"] == 3]
    return {
        "feature_score": round(feature_score, 1),
        "feature_score_raw": feature_score,
        "counted": len(counted),
        "must_have_done": sum(1 for r in musts if r["credit"] == 1.0),
        "must_have_total": len(musts),
        "by_area": by_area,
        "missing": [brief(r) for r in missing],
        "skipped": [brief(r) for r in skipped],
        "extras": [brief(r) for r in extras],
        "problems": problems,
    }


def visual_scores(paths):
    scores = []
    for p in paths:
        with open(p, encoding="utf-8") as fh:
            data = json.load(fh)
        if "score" not in data:
            raise MatrixError("%s is not imgdiff.py --json output" % p)
        name = data.get("files", {}).get("clone", p)
        value = float(data["score"])
        if not math.isfinite(value) or not 0 <= value <= 100:
            raise MatrixError("%s: visual score must be between 0 and 100" % p)
        scores.append({"file": name, "score": value,
                       "mode": data.get("mode", "layout")})
    return scores


def combine(result, visuals):
    if visuals:
        layout = sum(v["score"] for v in visuals) / len(visuals)
        result["visual"] = visuals
        result["layout_score"] = round(layout, 1)
        result["overall_raw"] = 0.8 * result["feature_score_raw"] + 0.2 * layout
        result["overall"] = round(result["overall_raw"], 1)
    else:
        result["overall"] = result["feature_score"]
        result["overall_raw"] = result["feature_score_raw"]
    return result


def render(result, markdown=False):
    out = []
    h = "## " if markdown else ""
    out.append("%sParity: %.1f / 100" % (h, result["overall"]))
    out.append("")
    out.append("features %.1f  (%d counted, must-haves %d of %d done)" % (
        result["feature_score"], result["counted"], result["must_have_done"],
        result["must_have_total"]))
    if "layout_score" in result:
        out.append("layout   %.1f  (%d screens compared)" % (
            result["layout_score"], len(result["visual"])))
    if result["must_have_done"] < result["must_have_total"]:
        out.append("")
        out.append("Not shippable yet: %d must-have features are not done."
                   % (result["must_have_total"] - result["must_have_done"]))
    out.append("")
    out.append("%sBy area, weakest first" % h)
    for a in result["by_area"]:
        out.append("- %-28s %5.1f  (%d features)" % (a["area"], a["score"], a["features"]))
    out.append("")
    out.append("%sMissing, by priority and credit (check plan dependencies)" % h)
    if not result["missing"]:
        out.append("- nothing. Every counted feature is done.")
    for m in result["missing"]:
        note = ("  (%s)" % m["notes"]) if m["notes"] else ""
        out.append("- [%s] %s: %s, %s%s" % (m["priority"], m["area"] or "-",
                                             m["feature"], m["clone"], note))
    if result["skipped"]:
        out.append("")
        out.append("%sLeft out on purpose (not scored)" % h)
        for m in result["skipped"]:
            out.append("- %s: %s" % (m["feature"], m["notes"] or "no reason given, add one"))
    if result["extras"]:
        out.append("")
        out.append("%sYours, not in the original (not scored)" % h)
        for m in result["extras"]:
            out.append("- %s" % m["feature"])
    if result.get("visual"):
        out.append("")
        out.append("%sScreens" % h)
        for v in sorted(result["visual"], key=lambda d: d["score"]):
            out.append("- %-40s %5.1f" % (v["file"], v["score"]))
    if result["problems"]:
        out.append("")
        out.append("%sFix in the matrix" % h)
        for p in result["problems"]:
            out.append("- %s" % p)
    return "\n".join(out)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("matrix", help="the feature matrix CSV")
    ap.add_argument("--visual", nargs="*", default=[],
                    help="imgdiff.py --json outputs to fold in")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--markdown", action="store_true")
    ap.add_argument("--fail-under", type=float)
    args = ap.parse_args(argv)
    if args.fail_under is not None and (not math.isfinite(args.fail_under)
                                       or not 0 <= args.fail_under <= 100):
        ap.error("--fail-under must be between 0 and 100")
    try:
        result = combine(score(load(args.matrix)), visual_scores(args.visual))
    except (MatrixError, OSError, ValueError, csv.Error, TypeError, AttributeError) as exc:
        print("parity: %s" % exc, file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(result, indent=2, ensure_ascii=False))
    else:
        print(render(result, markdown=args.markdown))
    if args.fail_under is not None and result["overall_raw"] < args.fail_under:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
