"""Deterministic recorder tests: synthetic clocks, no sleeps."""
import importlib.util
import json
from pathlib import Path
import tempfile
import subprocess
import sys
import unittest
from concurrent.futures import ThreadPoolExecutor

SCRIPT = Path(__file__).resolve().parents[2] / 'scripts/workflow_timing.py'

class TimingTests(unittest.TestCase):
    def setUp(self):
        self.assertTrue(SCRIPT.exists(), 'phase recorder is missing')
        spec = importlib.util.spec_from_file_location('workflow_timing', SCRIPT)
        self.api = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.api)
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def start(self, activity, phase='implementation', at=10, **kw):
        return self.api.start(self.root, activity, phase, utc=f'2026-11-25T00:00:{at:02d}+00:00', monotonic=at, clock='test-boot', **kw)

    def finish(self, activity, at=20, code=0, **kw):
        return self.api.finish(self.root, activity, code, utc=f'2026-11-25T00:00:{at:02d}+00:00', monotonic=at, clock='test-boot', **kw)

    def test_overlaps_count_union_and_inclusive_phase_time(self):
        self.start('a', at=10)
        self.start('b', at=15)
        self.finish('a', at=20)
        self.finish('b', at=25, code=7)
        summary = self.api.summary(self.root)
        self.assertEqual(summary['elapsedUnionSeconds'], 15)
        self.assertEqual(summary['phases']['implementation']['inclusiveSeconds'], 20)
        self.assertEqual(summary['failed'], 1)
        self.assertEqual(summary['completed'], 2)
        self.assertEqual(json.loads((self.root/'b.json').read_text())['exitCode'], 7)

    def test_concurrent_ids_have_separate_files_and_duplicate_rejected(self):
        with ThreadPoolExecutor() as pool:
            list(pool.map(lambda n: self.start(f'a{n}'), range(8)))
        self.assertEqual(len(list(self.root.glob('*.json'))), 8)
        with self.assertRaises(ValueError): self.start('a0')
        with self.assertRaises(ValueError): self.api.summary(self.root)

    def test_invalid_transitions_and_clocks_retain_unfinished_record(self):
        with self.assertRaises(ValueError): self.finish('absent')
        self.start('a', parent='parent', revision='abc', tree='def')
        for kwargs in ({'at': 9}, {'at': 20, 'clock': 'other'}):
            with self.subTest(kwargs=kwargs):
                if 'clock' in kwargs:
                    with self.assertRaises(ValueError):
                        self.api.finish(self.root, 'a', 0, utc='2026-11-25T00:00:20+00:00', monotonic=20, clock='other')
                else:
                    with self.assertRaises(ValueError): self.finish('a', **kwargs)
        self.assertEqual(json.loads((self.root/'a.json').read_text())['status'], 'running')
        self.finish('a')
        with self.assertRaises(ValueError): self.finish('a')

    def test_cli_retains_failed_activity_and_summary(self):
        base = [sys.executable, str(SCRIPT), '--directory', str(self.root)]
        for args in (['start', 'cli-1', 'tests'], ['finish', 'cli-1', '--exit-code', '9']):
            proc = subprocess.run(base + args, text=True, capture_output=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
        proc = subprocess.run(base + ['summary'], text=True, capture_output=True)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertEqual(json.loads(proc.stdout)['failed'], 1)
        record = json.loads((self.root / 'cli-1.json').read_text())
        self.assertEqual(record['status'], 'failed')
        self.assertNotIn('command', record)

    def test_summary_rejects_corrupt_completed_record(self):
        self.start('a')
        self.finish('a')
        path = self.root / 'a.json'
        record = json.loads(path.read_text())
        record['elapsedSeconds'] = -1
        path.write_text(json.dumps(record))
        with self.assertRaises(ValueError): self.api.summary(self.root)

    def test_summary_rejects_missing_completed_clock_fields(self):
        self.start('a')
        self.finish('a')
        path = self.root / 'a.json'
        original = json.loads(path.read_text())
        for field in ('completedUtc', 'completedMonotonic', 'clock'):
            with self.subTest(field=field):
                record = dict(original)
                record[field] = None
                path.write_text(json.dumps(record))
                with self.assertRaises(ValueError): self.api.summary(self.root)

    def test_reject_path_ids_bad_phase_and_backwards_utc(self):
        for activity, phase in (('../escape','spec'), ('a','unknown')):
            with self.assertRaises(ValueError): self.start(activity, phase)
        self.start('a')
        with self.assertRaises(ValueError):
            self.api.finish(self.root, 'a', 0, utc='2026-11-24T00:00:20+00:00', monotonic=20, clock='test-boot')

if __name__ == '__main__': unittest.main()
