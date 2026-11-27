from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from compare_frame_logs import Scenario, compare, read_log, summarize


BEGIN = "BEGIN campaign=0 character=1 stage=12 area=1 debug_loadout=1\n"
SCENARIO = Scenario(stage=12, area=1, character=1, checkpoint=0, debug_loadout=1)


def window(frame, drops, *, stage=12, player=18, boss=50, late=3, p95=34000,
           samples=50, diagnostic=9000, sync=5000):
    return (f"frame={frame} ticks={frame} char=1 stage={stage} area=1 fps=50 hp={player} "
            "checkpoint=0 engine=6/0 locked=0\n"
            + (f"PROFILE dropped_ticks={drops}\n" if drops is not None else "")
            + f"FRAME_TIME samples={samples} max_us=50000 p95_upper_us={p95} "
              f"over_two_fields={late} diagnostic_previous_us={diagnostic}\n"
              f"LOG_IO previous_sync_us={sync}\n"
              f"BOSS enabled={int(boss > 0)} hp={boss} state=0/3/0\n")


class FrameLogTests(unittest.TestCase):
    def setUp(self):
        temp = TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.directory = Path(temp.name)

    def log(self, name, text, *, mode="TWL", mhz=134, build="baseline"):
        path = self.directory / (name + ".log")
        header = f"BETA 9; native ARM9; id={build}; build=Nov 27 2026 00:00:00\n"
        if mode:
            header += f"PROFILE_HW mode={mode} arm9_mhz={mhz}\n"
        path.write_bytes((header + text).replace("\n", "\r\n").encode("utf-8-sig"))
        return path

    def test_dropped_ticks_across_unselected_windows_and_session_reset(self):
        path = self.log("a", BEGIN + window(60, 2) + window(120, 7, stage=11) + window(180, 8)
                        + BEGIN + window(240, 2))
        result = summarize(path, SCENARIO)
        self.assertEqual(result["windows"], 3)
        self.assertEqual(result["dropped_ticks"], 5)
        self.assertEqual(result["drop_delta_windows"], 3)
        self.assertEqual(summarize(path, SCENARIO, (180, 180))["dropped_ticks"], 1)

    def test_a_missing_counter_breaks_delta_attribution(self):
        path = self.log("a", "".join(window(60 * index, drops)
                                     for index, drops in enumerate((5, None, 10, 2, 3), 1)))
        _, parsed = read_log(path)
        self.assertEqual([record.drops for record in parsed], [None, None, None, None, 1])

    def test_player_and_boss_health_are_distinct(self):
        path = self.log("a", BEGIN + window(60, 0, player=0) + window(120, 0, boss=0) + window(180, 1))
        self.assertEqual(summarize(path, SCENARIO)["windows"], 2)
        combat = Scenario(12, 1, 1, 0, 1, boss_active=True)
        self.assertEqual(summarize(path, combat)["windows"], 1)
        self.assertEqual(summarize(path, combat)["dropped_ticks"], 1)

    def test_matched_ab_reports_deltas_with_coverage(self):
        a = self.log("a", BEGIN + window(60, 2) + window(120, 3, late=2))
        b = self.log("b", BEGIN + window(60, 0, late=1, p95=17000, diagnostic=3000, sync=2000), build="candidate")
        before, after = summarize(a, SCENARIO), summarize(b, SCENARIO)
        self.assertEqual(before["presentation_intervals"], 100)
        self.assertEqual(before["intervals_over_two_vblanks"], 5)
        self.assertEqual(before["windows_with_late_intervals"], 2)
        comparison = compare(before, after)
        self.assertFalse(comparison["same_build"])
        self.assertEqual(comparison["different_context"], [])
        delta = comparison["candidate_minus_baseline"]
        self.assertEqual(delta["late_rate_pct"], -3)
        self.assertEqual(delta["p95_upper_max_us"], -17000)
        self.assertEqual(delta["diagnostic_previous_mean_us"], -6000)
        self.assertEqual(delta["log_sync_previous_mean_us"], -3000)
        self.assertEqual(delta["dropped_ticks_per_window"], -1.5)
        self.assertTrue(compare(before, before)["same_capture"])

    def test_incompatible_profiles_keep_raw_metrics(self):
        a = self.log("a", BEGIN + window(60, 0))
        b = self.log("b", BEGIN + window(60, 0), mode="NTR", mhz=67, build="candidate")
        before, after = summarize(a, SCENARIO), summarize(b, SCENARIO)
        self.assertEqual(after["presentation_intervals"], 50)
        comparison = compare(before, after)
        self.assertIn("perfis de hardware diferentes", comparison["different_context"])
        self.assertIsNone(comparison["candidate_minus_baseline"])

    def test_invalid_intervals_and_absent_log_io_stay_unknown(self):
        truncated = window(60, 0, late=60, samples=50).split("LOG_IO ", 1)[0]
        report = summarize(self.log("a", BEGIN + truncated), SCENARIO)
        self.assertEqual(report["windows"], 1)
        self.assertEqual(report["timed_windows"], 0)
        self.assertIsNone(report["late_rate_pct"])
        self.assertIsNone(report["p95_upper_max_us"])
        self.assertIsNone(report["log_sync_previous_mean_us"])
        self.assertEqual(report["dropped_ticks"], 0)


if __name__ == "__main__":
    unittest.main()
