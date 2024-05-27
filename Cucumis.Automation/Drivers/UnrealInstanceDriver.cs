using System.Diagnostics;
using System.Net.Http.Json;
using System.Text;
using Gauntlet.UnrealTest;
using SpecFlow.Internal.Json;

namespace Cucumis.Automation.Drivers;

public class UnrealInstanceDriver : IUnrealInstanceDriverInterface
{
    private readonly HttpClient _client = new();
    public  int HttpPort { get; set; } = 9876;
    private const string Host = "localhost";
    private const string Protocol = "http";

    public void StartProcess()
    {
	    RunCucumisTest.IsFromGauntlet.Should().BeTrue("Cucumis test should only be run from Gauntlet.");
        _client.Timeout = new TimeSpan(0, 1, 0);
        _client.BaseAddress = new Uri($"{Protocol}://{Host}:{HttpPort}/Cucumis/Steps/");
    }

    public async Task<UnrealApiResponse> SendCommandAsync(string cucumisStep, Dictionary<string, string> contentDict = null)
    {
        JsonContent content = JsonContent.Create(contentDict ?? new Dictionary<string, string>());
        Task<string> json = content.ReadAsStringAsync();
        json.Wait();
        
        var queryContent = new StringContent(json.Result, Encoding.ASCII, "application/json");
        queryContent.Headers.ContentLength = json.Result.Length;
        HttpResponseMessage response = await _client.PostAsync(cucumisStep, queryContent);
        response.EnsureSuccessStatusCode();
        UnrealApiResponse result = await response.Content.ReadFromJsonAsync<UnrealApiResponse>();
        return result ?? new UnrealApiResponse();
    }

    public UnrealApiResponse SendCommand(string cucumisStep, Dictionary<string, string> content = null)
    {
        Task<UnrealApiResponse> task = SendCommandAsync(cucumisStep, content);
        task.Wait();
        return task.Result;
    }

    public void WaitIsReady()
    {
        bool succeed = false;
        int wait = 0;
        int retry = 5;
        while (!succeed)
        {
            Task.Delay(wait * 1000).Wait();
            try
            {
                UnrealApiResponse resultJson = SendCommand("IsCucumisPluginInitialized");
                succeed = resultJson.State == "Succeeded";
            }
            catch (Exception)
            {
	            retry.Should().BeGreaterOrEqualTo(0, $@"Can't connected to Cucumis API. Stopping...");
                Console.WriteLine($@"Can't connected to Cucumis API, retrying in {wait} seconds...");
            }
            wait = (int)(wait * 1.3f + 1);
            retry -= 1;
        }
    }

    public void SendSimpleCommand(string command, Dictionary<string, string> content = null)
    {
        UnrealApiResponse resultJson = SendCommand(command);
        resultJson.State.Should().Be("Succeeded", $"Command {command} failed, Reason: {resultJson.Data.GetValueOrDefault("Error")}.");
    }
}