"""Exercise the real verification runner's failure boundaries without a C++ build.

Run with Python 3 and either powershell.exe or pwsh on PATH. Fake native commands
are PowerShell scripts, so the tests are portable and never fetch dependencies.
"""

import json
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SHELL = shutil.which("pwsh") or shutil.which("powershell")


@unittest.skipUnless(SHELL, "PowerShell is required")
class VerificationRunnerTests(unittest.TestCase):
    def run_fixture(self, *, cmake_exit=0, tests=1, ctest_exit=0, skipped=False,
                    missing=False, export=False, provenance=False,
                    empty_report=False, report_failure=False, build_exit=0,
                    repetitions=1, report_name='fixture', launcher=None, sanitizers=False):
        with tempfile.TemporaryDirectory(prefix="egalis-verify-") as folder:
            fixture = Path(folder)
            subprocess.run(["git", "init", "-q", str(fixture)], check=True)
            subprocess.run(
                ["git", "-C", str(fixture), "-c", "user.name=fixture", "-c",
                 "user.email=fixture@example.invalid", "commit", "--allow-empty", "-qm", "fixture"],
                check=True,
            )
            cmake = fixture / "cmake.ps1"
            cmake.write_text(f"if ($args[0] -eq '--version') {{ 'cmake version 3.24'; exit 0 }}\nif ($args[0] -eq '--build') {{ exit {build_exit} }}\nexit {cmake_exit}\n")
            ctest = fixture / "ctest.ps1"
            xml = f'<testsuite><testcase name="{report_name}">' + ('<skipped />' if skipped else '') + ('<failure />' if report_failure else '') + '</testcase></testsuite>'
            if empty_report:
                xml = '<testsuite />'
            ctest.write_text(
                "if ($args[0] -eq '--version') { 'ctest version 3.24'; exit 0 }\n"
                "if ($args -contains '--show-only=json-v1') {\n"
                + ("'{\"tests\":[{\"name\":\"fixture\"}]}'\n" if tests else "'{\"tests\":[]}'\n")
                + "exit 0 }\n"
                + f"if ({ctest_exit} -ne 0) {{ exit {ctest_exit} }}\n"
                + "$path=$args[([array]::IndexOf($args,'--output-junit')+1)]\n"
                + f"[IO.File]::WriteAllText($path, '{xml}')\nexit 0\n"
            )
            source = fixture
            if export:
                source = fixture / 'build' / 'source-export'
                source.mkdir(parents=True)
            extra = []
            if provenance:
                extra = ['-SourceRevision', '1' * 40, '-SourceTree', '2' * 40]
            command = [SHELL, "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(ROOT / "scripts/verify.ps1"),
                 "-SourceDir", str(source), "-BuildRoot", str(fixture / "build"),
                 "-CMake", str(fixture / 'missing-executable' if missing else cmake), "-CTest", str(ctest)] + extra
            if sanitizers:
                command += ['-Sanitizers']
            if launcher:
                extra_launcher = sys.executable if launcher == "existing" else str(fixture / "missing-launcher")
                if launcher == 'mutable':
                    extra_launcher = str(fixture / ('launcher' + Path(sys.executable).suffix))
                    shutil.copy2(sys.executable, extra_launcher)
                command += ["-CompilerLauncher", extra_launcher]
            self.launcher_digests = []
            for iteration in range(repetitions):
                if launcher == 'mutable':
                    if iteration:
                        with open(extra_launcher, 'ab') as stream:
                            stream.write(b'changed-launcher-content')
                    self.launcher_digests.append(hashlib.sha256(Path(extra_launcher).read_bytes()).hexdigest())
                proc = subprocess.run(command, capture_output=True, text=True)
            evidence_paths = sorted((fixture / "build").glob("evidence-*/verification.json"))
            self.assertEqual(len(evidence_paths), repetitions, proc.stdout + proc.stderr)
            self.completed_runs = [json.loads(path.read_text(encoding='utf-8')) for path in evidence_paths]
            return proc, self.completed_runs[-1]

    def test_success_verifies_both_configurations(self):
        proc, evidence = self.run_fixture()
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertTrue(evidence["success"])
        self.assertEqual([x["configuration"] for x in evidence["configurations"]], ["Debug", "Release"])

    def test_failed_configure_stops_before_test_execution(self):
        proc, evidence = self.run_fixture(cmake_exit=17)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence["success"])
        self.assertEqual(evidence["commands"][-1]["exitCode"], 17)
        self.assertEqual(evidence["configurations"], [])

    def test_failure_and_resolution_have_complete_timing(self):
        for options in ({"cmake_exit": 17}, {"missing": True}):
            with self.subTest(options=options):
                _, evidence = self.run_fixture(**options)
                self.assertGreaterEqual(evidence['elapsedSeconds'], 0)
                for entry in evidence['commands']:
                    self.assertTrue(entry['startedUtc'])
                    self.assertTrue(entry['completedUtc'])
                    self.assertGreaterEqual(entry['elapsedSeconds'], 0)
                    self.assertTrue(entry['phase'])
                    self.assertIn('configuration', entry)

    def test_launcher_is_resolved_and_passed_to_each_fresh_configure(self):
        proc, evidence = self.run_fixture(launcher='existing')
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertTrue(evidence['compilerLauncher']['resolved'])
        configs = [x for x in evidence['commands'] if x['phase'] == 'configure']
        self.assertEqual([x['configuration'] for x in configs], ['Debug', 'Release'])
        self.assertTrue(all(any(a.startswith('-DCMAKE_CXX_COMPILER_LAUNCHER=') for a in x['arguments']) for x in configs))

    def test_launcher_identity_changes_when_same_path_contents_change(self):
        proc, _ = self.run_fixture(launcher='mutable', repetitions=2)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        launchers = [run['compilerLauncher'] for run in self.completed_runs]
        self.assertEqual(launchers[0]['resolved'], launchers[1]['resolved'])
        self.assertNotEqual(self.launcher_digests[0], self.launcher_digests[1])
        self.assertEqual([entry['sha256'].lower() for entry in launchers], self.launcher_digests)
        self.assertTrue(all(entry['version'] == 'unknown' for entry in launchers))
        self.assertTrue(all(run['success'] for run in self.completed_runs))

    def test_configuration_provenance_and_sanitizers_are_explicit(self):
        proc, evidence = self.run_fixture(sanitizers=True)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertEqual(evidence['settings']['configurations'], ['Debug', 'Release'])
        self.assertEqual(evidence['settings']['generator'], 'Ninja')
        self.assertTrue(evidence['settings']['sanitizers'])
        self.assertEqual(evidence['compilerLauncher']['cacheState'], 'unknown')
        configs = [x for x in evidence['commands'] if x['phase'] == 'configure']
        self.assertTrue(all('-DEGALIS_ENABLE_SANITIZERS=ON' in x['arguments'] for x in configs))
        self.assertTrue(all('-DCMAKE_COMPILE_WARNING_AS_ERROR=ON' in x['arguments'] for x in configs))

    def test_missing_launcher_fails_before_configure(self):
        proc, evidence = self.run_fixture(launcher='missing')
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertFalse(any('-S' in x['arguments'] for x in evidence['commands']))
        self.assertIn('compilerLauncher', evidence)

    def test_zero_tests_is_not_a_success(self):
        proc, evidence = self.run_fixture(tests=0)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence["success"])
        self.assertIn("No tests discovered", evidence["error"])

    def test_failed_tests_are_not_a_success(self):
        proc, evidence = self.run_fixture(ctest_exit=8)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence["success"])
        self.assertEqual(evidence["commands"][-1]["exitCode"], 8)

    def test_missing_executable_cannot_reuse_a_successful_exit_code(self):
        proc, evidence = self.run_fixture(missing=True)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertIsNone(evidence['commands'][-1]['exitCode'])

    def test_skipped_tests_are_not_full_verification(self):
        proc, evidence = self.run_fixture(skipped=True)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertIn('Skipped tests', evidence['error'])

    def test_nested_export_requires_explicit_provenance(self):
        proc, evidence = self.run_fixture(export=True)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertIsNone(evidence['head'])
        self.assertIn('require both SourceRevision and SourceTree', evidence['error'])

    def test_export_reports_supplied_tree_not_containing_repo_head(self):
        proc, evidence = self.run_fixture(export=True, provenance=True)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertEqual(evidence['sourceKind'], 'staged-export')
        self.assertEqual(evidence['sourceRevision'], '1' * 40)
        self.assertEqual(evidence['sourceTree'], '2' * 40)
        self.assertIsNone(evidence['head'])
        self.assertEqual(evidence['dirtyPaths'], [])

    def test_incomplete_report_cannot_claim_full_suite(self):
        proc, evidence = self.run_fixture(empty_report=True)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertIn('JUnit count does not match', evidence['error'])

    def test_junit_failure_is_rejected_even_with_zero_exit_code(self):
        proc, evidence = self.run_fixture(report_failure=True)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertIn('JUnit reports failures', evidence['error'])

    def test_failed_build_stops_before_test_execution(self):
        proc, evidence = self.run_fixture(build_exit=23)
        self.assertNotEqual(proc.returncode, 0)
        self.assertFalse(evidence['success'])
        self.assertEqual(evidence['commands'][-1]['exitCode'], 23)
        self.assertEqual(evidence['commands'][-1]['arguments'][0], '--build')
        self.assertEqual(evidence['configurations'], [])

    def test_repeated_invocations_never_share_build_directories(self):
        proc, _ = self.run_fixture(repetitions=2)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        builds = [config['build'] for run in self.completed_runs for config in run['configurations']]
        self.assertEqual(len(builds), 4)
        self.assertEqual(len(set(builds)), 4)
        self.assertTrue(all(run['success'] for run in self.completed_runs))

    def test_normal_configure_explicitly_disables_sanitizers(self):
        proc, evidence = self.run_fixture()
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        configure_commands = [entry for entry in evidence['commands'] if '-S' in entry['arguments']]
        self.assertEqual(len(configure_commands), 2)
        self.assertTrue(all('-DEGALIS_ENABLE_SANITIZERS=OFF' in entry['arguments'] for entry in configure_commands))

    def test_equal_count_substituted_test_identity_is_rejected(self):
        for name in ('different-fixture', 'Fixture'):
            with self.subTest(name=name):
                proc, evidence = self.run_fixture(report_name=name)
                self.assertNotEqual(proc.returncode, 0)
                self.assertFalse(evidence['success'])
                self.assertIn('JUnit identities do not match', evidence['error'])


if __name__ == "__main__":
    unittest.main()
