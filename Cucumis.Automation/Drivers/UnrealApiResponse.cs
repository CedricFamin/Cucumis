namespace Cucumis.Automation.Drivers;

public class UnrealApiResponse
{
	public string State { get; set; } = "Failed";
	public Dictionary<string, string> Data { get; set; } = new();
}