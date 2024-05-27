namespace Cucumis.Automation.Drivers;

public interface IUnrealInstanceDriverInterface
{
	public UnrealApiResponse SendCommand(string cucumisStep, Dictionary<string, string> content = null);
	public void SendSimpleCommand(string command, Dictionary<string, string> content = null);
}