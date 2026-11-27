"""Compare matching gameplay windows in native X4 logs."""

from argparse import ArgumentParser, ArgumentTypeError
from dataclasses import asdict, dataclass, field
from hashlib import sha256
import json
from pathlib import Path
import re
from statistics import mean, median


FIELDS = re.compile(r"\b(\w+)=([^\s;]+)")


def nonnegative(fields, name):
    value = fields.get(name)
    return int(value) if value and value.isascii() and value.isdecimal() else None


@dataclass
class Window:
    state: dict
    session: int
    route: tuple
    has_begin: bool
    profile: dict = field(default_factory=dict)
    timing: dict = field(default_factory=dict)
    log_io: dict = field(default_factory=dict)
    boss: dict = field(default_factory=dict)
    drops: int | None = None


def read_log(path):
    data = path.read_bytes()
    builds, hardware = set(), set()
    windows = []
    current = None
    session = 0
    has_begin = False
    route = (None, None)
    for line in data.decode("utf-8-sig", errors="replace").splitlines():
        line = line.strip()
        fields = dict(FIELDS.findall(line))
        if "; native ARM9;" in line:
            if fields.get("id"):
                builds.add(fields["id"])
            session += 1
            current, has_begin, route = None, False, (None, None)
        elif line.startswith("PROFILE_HW "):
            mhz = nonnegative(fields, "arm9_mhz")
            if fields.get("mode") in ("NTR", "TWL") and mhz:
                hardware.add((fields["mode"], mhz))
        elif line.startswith("BEGIN "):
            session += 1
            route = (nonnegative(fields, "campaign"), nonnegative(fields, "debug_loadout"))
            current, has_begin = None, True
        elif line.startswith("frame="):
            current = Window(fields, session, route, has_begin)
            windows.append(current)
        elif current:
            if line.startswith("PROFILE "):
                current.profile = fields
            elif line.startswith("FRAME_TIME "):
                current.timing = fields
            elif line.startswith("LOG_IO "):
                current.log_io = fields
            elif line.startswith("BOSS "):
                current.boss = fields

    if len(builds) > 1 or len(hardware) > 1:
        raise ValueError(f"{path}: use um arquivo por compilação e perfil de hardware")

    last_session = None
    previous = None
    for window in windows:
        if window.session != last_session:
            previous = 0 if window.has_begin else None
            last_session = window.session
        count = nonnegative(window.profile, "dropped_ticks")
        if previous is not None and count is not None and count >= previous:
            window.drops = count - previous
        # A missing sample or a reset ends the interval for attribution.
        previous = count

    profile = next(iter(hardware), None)
    info = {"file": str(path), "sha256": sha256(data).hexdigest(),
            "build": next(iter(builds), None),
            "hardware": {"mode": profile[0], "arm9_mhz": profile[1]} if profile else None}
    return info, windows


@dataclass(frozen=True)
class Scenario:
    stage: int
    area: int
    character: int
    checkpoint: int | None
    debug_loadout: int | None
    boss_active: bool = False

    def matches(self, window, vblanks):
        state = window.state
        if state.get("engine") != "6/0" or state.get("locked") != "0" or not nonnegative(state, "hp"):
            return False
        if any(nonnegative(state, key) != target for key, target in
               (("stage", self.stage), ("area", self.area), ("char", self.character))):
            return False
        if self.checkpoint is not None and nonnegative(state, "checkpoint") != self.checkpoint:
            return False
        if self.debug_loadout is not None and window.route[1] != self.debug_loadout:
            return False
        if self.boss_active and (window.boss.get("enabled") != "1" or not nonnegative(window.boss, "hp")):
            return False
        tick = nonnegative(state, "frame")
        return vblanks is None or (tick is not None and vblanks[0] <= tick <= vblanks[1])


def values(windows, section, name):
    return [value for window in windows if (value := nonnegative(getattr(window, section), name)) is not None]


def summarize(path, scenario, vblanks=None):
    info, windows = read_log(path)
    chosen = [window for window in windows if scenario.matches(window, vblanks)]
    routes = {window.route for window in chosen}
    fps = values(chosen, "state", "fps")
    intervals = []
    for window in chosen:
        samples = nonnegative(window.timing, "samples")
        late = nonnegative(window.timing, "over_two_fields")
        if samples and late is not None and late <= samples:
            intervals.append((window, samples, late))
    measured = [window for window, _, _ in intervals]
    p95 = values(measured, "timing", "p95_upper_us")
    diagnostics = values(chosen, "timing", "diagnostic_previous_us")
    sync = values(chosen, "log_io", "previous_sync_us")
    drops = [window.drops for window in chosen if window.drops is not None]
    frames = sum(count for _, count, _ in intervals)
    delayed = sum(late for _, _, late in intervals)
    return {
        **info, "vblank_range": vblanks,
        "routes": [{"campaign": campaign, "debug_loadout": debug} for campaign, debug in sorted(routes, key=repr)],
        "windows": len(chosen), "timed_windows": len(measured),
        "window_fps_min": min(fps, default=None), "window_fps_mean": mean(fps) if fps else None,
        "windows_below_30_fps": sum(rate < 30 for rate in fps) if fps else None,
        "presentation_intervals": frames if measured else None,
        "intervals_over_two_vblanks": delayed if measured else None,
        "windows_with_late_intervals": sum(late > 0 for _, _, late in intervals) if measured else None,
        "late_rate_pct": 100 * delayed / frames if frames else None,
        "p95_upper_median_us": median(p95) if p95 else None,
        "p95_upper_max_us": max(p95, default=None),
        "frame_interval_max_us": max(values(measured, "timing", "max_us"), default=None),
        "dropped_ticks": sum(drops) if drops else None,
        "drop_delta_windows": len(drops),
        "dropped_ticks_per_window": mean(drops) if drops else None,
        "diagnostic_previous_mean_us": mean(diagnostics) if diagnostics else None,
        "diagnostic_previous_max_us": max(diagnostics, default=None),
        "diagnostic_windows": len(diagnostics),
        "log_sync_previous_mean_us": mean(sync) if sync else None,
        "log_sync_previous_max_us": max(sync, default=None),
        "log_sync_windows": len(sync),
    }


DELTAS = ("late_rate_pct", "p95_upper_median_us", "p95_upper_max_us", "frame_interval_max_us",
          "dropped_ticks_per_window", "diagnostic_previous_mean_us", "log_sync_previous_mean_us")


def compare(before, after):
    reasons = []
    if not before["windows"] or not after["windows"]:
        reasons.append("recorte vazio em pelo menos um log")
    if not before["build"] or not after["build"]:
        reasons.append("identificador da compilação ausente")
    if not before["hardware"] or not after["hardware"]:
        reasons.append("perfil de hardware ausente")
    elif before["hardware"] != after["hardware"]:
        reasons.append("perfis de hardware diferentes")
    if (len(before["routes"]) != 1 or before["routes"] != after["routes"]
            or None in before["routes"][0].values()):
        reasons.append("modo de campanha ou equipamento de debug diferente ou indefinido")
    delta = None if reasons else {key: after[key] - before[key] if None not in (before[key], after[key]) else None
                                  for key in DELTAS}
    return {"same_build": bool(before["build"] and before["build"] == after["build"]),
            "same_capture": before["sha256"] == after["sha256"],
            "different_context": reasons, "candidate_minus_baseline": delta}


def vblank_range(value):
    try:
        start, end = map(int, value.split(":"))
        if 0 <= start <= end:
            return start, end
    except ValueError:
        pass
    raise ArgumentTypeError("use INICIO:FIM com inteiros não negativos")


def show(value):
    if value is None:
        return "indisponível"
    return f"{value:.2f}" if isinstance(value, float) else str(value)


DISPLAY = {
    "windows": "Janelas selecionadas", "timed_windows": "Janelas com medição de intervalos",
    "window_fps_min": "FPS mínimo por janela", "window_fps_mean": "FPS médio por janela",
    "windows_below_30_fps": "Janelas abaixo de 30 FPS",
    "presentation_intervals": "Intervalos de apresentação medidos",
    "intervals_over_two_vblanks": "Intervalos acima de dois VBlanks",
    "windows_with_late_intervals": "Janelas com um ou mais atrasos",
    "late_rate_pct": "Intervalos atrasados (%)", "p95_upper_median_us": "Mediana dos limites de p95 (µs)",
    "p95_upper_max_us": "Maior limite de p95 (µs)", "frame_interval_max_us": "Maior intervalo (µs)",
    "dropped_ticks": "Ticks de simulação descartados", "drop_delta_windows": "Janelas com delta de ticks",
    "dropped_ticks_per_window": "Ticks descartados por janela medida",
    "diagnostic_previous_mean_us": "Diagnóstico anterior médio (µs)",
    "diagnostic_previous_max_us": "Diagnóstico anterior máximo (µs)",
    "diagnostic_windows": "Janelas com custo de diagnóstico",
    "log_sync_previous_mean_us": "Sincronização anterior do log média (µs)",
    "log_sync_previous_max_us": "Sincronização anterior do log máxima (µs)",
    "log_sync_windows": "Janelas com custo de sincronização",
}


def print_report(report):
    scenario = report["scenario"]
    print(f"Fase {scenario['stage']}, área {scenario['area']} (índice do log), "
          f"personagem {('X', 'Zero')[scenario['character']]}")
    if scenario["boss_active"]:
        print("Apenas janelas com chefe ativo e HP positivo.")
    trials = [("A", report["baseline"])]
    if "candidate" in report:
        trials.append(("B", report["candidate"]))
    for label, trial in trials:
        hardware = trial["hardware"]
        profile = f"{hardware['mode']} a {hardware['arm9_mhz']} MHz" if hardware else "perfil indisponível"
        print(f"\n{label}  {trial['file']}  |  compilação {show(trial['build'])}  |  {profile}")
        for key, title in DISPLAY.items():
            print(f"  {title:44} {show(trial[key])}")
    if "comparison" in report:
        comparison = report["comparison"]
        if comparison["same_capture"]:
            print("\nA e B usam a mesma captura.")
        elif comparison["same_build"]:
            print("\nA e B usam a mesma compilação.")
        if comparison["different_context"]:
            print("\nComparação entre contextos indisponível. " + ", ".join(comparison["different_context"]) + ".")
        else:
            print("\nDiferença B menos A")
            for key, value in comparison["candidate_minus_baseline"].items():
                title = "Intervalos atrasados (pontos percentuais)" if key == "late_rate_pct" else DISPLAY[key]
                print(f"  {title:44} {show(value)}")
    print("\nO p95 representa limites por janela. O estado no fim da janela define o recorte.")


def main():
    parser = ArgumentParser(description="Compara janelas de gameplay nos logs nativos do X4.")
    parser.add_argument("baseline", type=Path)
    parser.add_argument("candidate", type=Path, nargs="?")
    parser.add_argument("--stage", type=int, required=True)
    parser.add_argument("--area", type=int, required=True, help="índice area= no log, começando em zero")
    parser.add_argument("--character", choices=("x", "zero"), required=True)
    parser.add_argument("--checkpoint", type=int)
    parser.add_argument("--debug-loadout", type=int, choices=(0, 1))
    parser.add_argument("--boss-active", action="store_true", help="exige chefe ativo com HP positivo no fim da janela")
    parser.add_argument("--before-vblanks", type=vblank_range, metavar="INICIO:FIM")
    parser.add_argument("--after-vblanks", type=vblank_range, metavar="INICIO:FIM")
    parser.add_argument("--json", action="store_true", help="grava o relatório JSON na saída padrão")
    args = parser.parse_args()
    if args.after_vblanks and not args.candidate:
        parser.error("--after-vblanks exige o segundo log")
    scenario = Scenario(args.stage, args.area, int(args.character == "zero"), args.checkpoint,
                        args.debug_loadout, args.boss_active)
    try:
        before = summarize(args.baseline, scenario, args.before_vblanks)
        report = {"scenario": asdict(scenario), "baseline": before}
        if args.candidate:
            after = summarize(args.candidate, scenario, args.after_vblanks)
            report.update(candidate=after, comparison=compare(before, after))
    except (OSError, ValueError) as error:
        parser.error(str(error))
    if args.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print_report(report)


if __name__ == "__main__":
    main()
