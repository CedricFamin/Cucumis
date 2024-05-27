using System.Configuration;
using System.Reflection;
using EpicGame;
using Xunit.Abstractions;
using Xunit.Runners;
using TestMethodDisplay = Xunit.TestMethodDisplay;
using TestMethodDisplayOptions = Xunit.TestMethodDisplayOptions;

namespace Gauntlet.UnrealTest
{
	public class CucumisTestTestConfig : EpicGameTestConfig
	{
		[AutoParamWithNames("Client", "Clients")] public int NumberOfClient = 1;
		[AutoParamWithNames("Category", "Categories")] public string Categories = "";
		[AutoParamWithNames("Feature", "Features")] public string Features = "";
		[AutoParamWithNames("CucumisTest", "CucumisTests")] public string CucumisTests = "";
	}

	public class RunCucumisTest : EpicGameTestNode<CucumisTestTestConfig>
	{
		public static bool IsFromGauntlet = false;
		public HashSet<string> IncludeCategories;
		public HashSet<string> ExcludeCategories;
		public HashSet<string> Features;
		public HashSet<string> CucumisTests;

		private bool TestCaseFilter(ITestCase testCase)
		{
			HashSet<string> testFeature = testCase.Traits.GetValueOrDefault("FeatureTitle", new List<string>()).ToHashSet();
			HashSet<string> testCategories = testCase.Traits.GetValueOrDefault("Category", new List<string>()).ToHashSet();
			
			if (testCategories.Intersect(new HashSet<string> { "2_Players", "3_Players", "4_players" }).Any())
			{
				testCategories.Add("Multiplayer");
			}

			if (CucumisTests.Count > 0)
			{
				if (!CucumisTests.Contains(testCase.DisplayName))
				{
					Console.WriteLine($"[Cucumis] Skipping {testCase.DisplayName}: Invalid Test name.");
					return false;
				}
			}
			
			if (Features.Count > 0)
			{
				if (!testFeature.Intersect(Features).Any())
				{
					Console.WriteLine($"[Cucumis] Skipping {testCase.DisplayName}: Invalid Feature Title.");
					return false;
				}
			}
			
			if (testCategories.Intersect(ExcludeCategories).Any())
			{
				Console.WriteLine($"[Cucumis] Skipping {testCase.DisplayName}: Invalid ExcludeCategories.");
				return false;
			}
			
			if (IncludeCategories.Count > 0)
			{
				if (IncludeCategories.Except(IncludeCategories).Union(IncludeCategories.Except(testCategories)).Any())
				{
					Console.WriteLine($"[Cucumis] Skipping {testCase.DisplayName}: Invalid IncludeCategories.");
					return false;
				}
			}
			
			return true;
		}
		
		public RunCucumisTest(UnrealTestContext InContext) : base(InContext)
		{
			IsFromGauntlet = true;
		}

		public override CucumisTestTestConfig GetConfiguration()
		{
			CucumisTestTestConfig Config = base.GetConfiguration();
			
			IncludeCategories = CachedConfig.Categories.Split(',', StringSplitOptions.RemoveEmptyEntries).ToHashSet();
			ExcludeCategories = new HashSet<string>();
			Features = CachedConfig.Features.Split(',', StringSplitOptions.RemoveEmptyEntries).ToHashSet();
			CucumisTests = CachedConfig.CucumisTests.Split(',', StringSplitOptions.RemoveEmptyEntries).ToHashSet();
			
			if (CachedConfig.NumberOfClient > 1)
			{
				IncludeCategories.Add("Multiplayer");
				IncludeCategories.Add($"{CachedConfig.NumberOfClient}_Players");
			}
			else
			{
				ExcludeCategories.Add("Multiplayer");
			}
			
			UnrealTestRole[] Roles = Config.RequireRoles(UnrealTargetRole.Client, Config.NumberOfClient).ToArray();
			for (int i = 0; i < Config.NumberOfClient; ++i)
			{
				UnrealTestRole Client = Roles[i];
				Client.Controllers.Add("Cucumis");
				Client.CommandLineParams.AddUnique("CucumisPort", 9876 + i);
			}
			
			Config.RequiresLogin = false;
			Config.PreAssignAccount = false;
			
			return Config;
		}

		private AssemblyRunner _xunitRunner = null;
		
		public override bool StartTest(int Pass, int InNumPasses)
		{
			if (!base.StartTest(Pass, InNumPasses))
				return false;
			
			Assembly currentAssembly = Assembly.GetExecutingAssembly();
			_xunitRunner = AssemblyRunner.WithoutAppDomain(currentAssembly.Location);
			_xunitRunner.OnDiscoveryComplete = OnDiscoveryComplete;
			_xunitRunner.OnExecutionComplete = OnExecutionComplete;
			_xunitRunner.OnTestFailed = OnTestFailed;
			_xunitRunner.OnTestSkipped = OnTestSkipped;
			_xunitRunner.TestCaseFilter = TestCaseFilter;
			_xunitRunner.OnDiagnosticMessage = OnDiagnosticMessage;
			_xunitRunner.OnErrorMessage = OnErrorMessage;
			_xunitRunner.OnTestFinished = OnTestFinished;
			_xunitRunner.OnTestOutput = OnTestOutput;
			_xunitRunner.OnTestPassed = OnTestPassed;
			_xunitRunner.OnTestStarting = OnTestStarting;
			
			Console.WriteLine(@"Discovering...");
			
			_xunitRunner.Start(
				diagnosticMessages: true
				, methodDisplay: TestMethodDisplay.ClassAndMethod
				, methodDisplayOptions: TestMethodDisplayOptions.None
				, internalDiagnosticMessages: true
				, parallel: false
				, parallelAlgorithm: ParallelAlgorithm.Conservative);

			return true;
		}

		private void OnTestStarting(TestStartingInfo info)
		{
			Console.WriteLine($"[Cucumis] Starting test {info.TestDisplayName}.");
		}

		private void OnTestPassed(TestPassedInfo info)
		{
			Console.WriteLine($"[Cucumis] Succeed test {info.TestDisplayName}.");
		}

		private void OnTestOutput(TestOutputInfo info)
		{
			Console.Write($"[Cucumis] {info.Output}");
		}

		private void OnTestFinished(TestFinishedInfo info)
		{
			Console.WriteLine($"[Cucumis] Test finished {info.TestDisplayName}");
		}

		private void OnErrorMessage(ErrorMessageInfo info)
		{
			ReportError($"[Cucumis] Error during test: {info.MesssageType} - {info.ExceptionType}");
			ReportError($"[Cucumis] {info.ExceptionMessage}");
			ReportError($"[Cucumis] {info.ExceptionStackTrace}");
		}

		public override void CleanupTest()
		{
			_xunitRunner.Dispose();
		}
		public override void TickTest()
		{
			base.TickTest();
		}

		private void OnTestSkipped(TestSkippedInfo info)
		{
			ReportWarning("[Cucumis][SKIP] {0}: {1}", info.TestDisplayName, info.SkipReason);
		}

		private void OnTestFailed(TestFailedInfo info)
		{
			ReportError("[Cucumis][FAIL] {0}: {1}", info.TestDisplayName, info.ExceptionMessage);
			if (info.ExceptionStackTrace != null)
			{
				ReportError(info.ExceptionStackTrace);
			}
		}

		private void OnExecutionComplete(ExecutionCompleteInfo info)
		{
			Console.WriteLine($"[Cucumis] Finished: {info.TotalTests} tests in {Math.Round(info.ExecutionTime, 3)}s ({info.TestsFailed} failed, {info.TestsSkipped} skipped)");
			StopTest(StopReason.Completed);
		}

		private void OnDiscoveryComplete(DiscoveryCompleteInfo info)
		{
			Console.WriteLine($"[Cucumis] Running {info.TestCasesToRun} of {info.TestCasesDiscovered} tests...");
		}
		
		private void OnDiagnosticMessage(DiagnosticMessageInfo info)
		{
			Console.WriteLine($@"[Cucumis] {info.Message}");
		}
	}
}

