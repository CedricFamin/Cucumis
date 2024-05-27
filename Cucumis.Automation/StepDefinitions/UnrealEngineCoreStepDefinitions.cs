using Cucumis.Automation.Drivers;

namespace Cucumis.Automation.StepDefinitions
{
    [Binding]
    public sealed class UnrealEngineCoreStepDefinitions
    {
        private readonly UnrealInstancesDriver _unrealInstances;

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
