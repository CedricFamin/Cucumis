using Cucumis.Automation.Drivers;

namespace Cucumis.Automation.StepDefinitions
{
    [Binding]
    public sealed class UnrealEngineCoreStepDefinitions
    {
        private readonly UnrealInstancesDriver _unrealInstances;

        [BeforeScenario]
        public void CreateUnrealIntances(ScenarioContext scenarioContext)
        {
	        int nbInstanceToCreate = 9;
	        while (nbInstanceToCreate > 1)
	        {
		        if (scenarioContext.ScenarioInfo.Tags.Contains($"{nbInstanceToCreate}_Players"))
		        {
			        break;
		        }
		        --nbInstanceToCreate;
	        }

	        while (nbInstanceToCreate > 0)
	        {
		        UnrealInstanceDriver unrealInstance = _unrealInstances.CreateNewInstance();
		        unrealInstance.StartProcess();
		        unrealInstance.WaitIsReady();
		        unrealInstance.SendSimpleCommand("LoadBlueprintSteps");
		        --nbInstanceToCreate;
	        }
        }
        
        [AfterScenario]
        public void ResetUnrealIntances()
        {
	        _unrealInstances.Reset();
        }
        
        UnrealEngineCoreStepDefinitions(UnrealInstancesDriver driver)
        {
            _unrealInstances = driver;
        }

        [Given("start an unreal editor instance")]
        public void StartAnUnrealEditorInstance()
        {
            UnrealInstanceDriver unrealInstance = _unrealInstances.CreateNewInstance();
            unrealInstance.StartProcess();
            unrealInstance.WaitIsReady();
            unrealInstance.SendSimpleCommand("LoadBlueprintSteps");
        }

        [Given("wait (.*) seconds")]
        public void WaitXSeconds(int seconds)
        {
            Task.Delay(seconds * 1000).Wait();
        }

        [When("(.*)execute the unreal command \"(.*)\"")]
        public void ExecuteTheUnrealCommand(string accessor, string command)
        {
            _unrealInstances.Instances(accessor).SendCommand("ExecuteUnrealCommand", new Dictionary<string, string>
            {
                {"Command", command}
            });
        }

        [Then("(.*)travel to (.*)")]
        public void TravelTo(string accessor, string mapName)
        {
            _unrealInstances.Instances(accessor).SendSimpleCommand("IsCucumisPluginInitialized", new Dictionary<string, string>
            {
                {"MapName", mapName}
            });
        }

        [When(@"(.*)Load Start Game")]
        public void WhenLoadStartGame(string accessor)
        {
	        _unrealInstances.Instances(accessor).SendSimpleCommand("LoadStartGame");
        }

        [Given(@"(.*)wait until main menu is shown")]
        public void GivenWaitUntilMainMenuIsShown(string accessor)
        {
	        _unrealInstances.Instances(accessor).SendSimpleCommand("WaitUntilMainMenuReady");
	        
        }
    }
}
