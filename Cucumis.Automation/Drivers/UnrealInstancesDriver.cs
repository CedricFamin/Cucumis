using System.Text.RegularExpressions;

namespace Cucumis.Automation.Drivers;

public class UnrealInstancesDriver
{
    public class UnrealInstancesAccessor : IUnrealInstanceDriverInterface
    {
        public List<UnrealInstanceDriver> Instances { get; set; } = new();

        public UnrealApiResponse SendCommand(string cucumisStep, Dictionary<string, string> content = null)
        {
            UnrealApiResponse commonResponse = new UnrealApiResponse();
            commonResponse.State = "Succeeded";
            int instanceId = 0;
            foreach (UnrealInstanceDriver instance in Instances)
            {
	            UnrealApiResponse response = instance.SendCommand(cucumisStep, content);
	            if (response.State == "Failed")
	            {
		            commonResponse.State = "Failed";
	            }

	            foreach (KeyValuePair<string,string> data in commonResponse.Data)
	            {
		            commonResponse.Data.Add($"{data.Key}_{instanceId}", data.Value);
	            }

	            instanceId += 1;
            }
            return commonResponse;
        }
        
        public void WaitIsReady()
        {
            foreach (UnrealInstanceDriver instance in Instances)
            {
                instance.WaitIsReady();
            }
        }

        public void SendSimpleCommand(string command, Dictionary<string, string> content = null)
        {
            foreach (UnrealInstanceDriver instance in Instances)
            {
                instance.SendSimpleCommand(command, content);
            }
        }
    }
    
    public const int BaseHttpPort = 9876;
    private List<UnrealInstanceDriver> _instances { get; set; } = new();

    public UnrealInstanceDriver CreateNewInstance()
    {
        UnrealInstanceDriver instance = new UnrealInstanceDriver
        {
            HttpPort = BaseHttpPort + _instances.Count
        };
        _instances.Add(instance);
        return instance;
    }

    public UnrealInstancesAccessor Server()
    {
        // Convention, instance 0 is the main server
        return new UnrealInstancesAccessor
        {
            Instances = new List<UnrealInstanceDriver> { _instances[0] }
        };
    }

    public UnrealInstancesAccessor Instances(List<int> indexes)
    {
        UnrealInstancesAccessor accessor = new UnrealInstancesAccessor();
        foreach (int index in indexes)
        {
            accessor.Instances.Add(_instances[index]);
        }
        
        return accessor;
    }
    
    public UnrealInstancesAccessor All()
    {
        return new UnrealInstancesAccessor
        {
            Instances = _instances
        };
    }
    
    public UnrealInstancesAccessor Instances(string accessPattern)
    {
        return accessPattern switch
        {
            "" => All(),
            "all " => All(),
            "server " => Server(),
            null => All(),
            _ => DecodeAccessPattern(accessPattern)
        };
    }

    private UnrealInstancesAccessor DecodeAccessPattern(string accessPattern)
    {
        const string pattern = "^instances? (?:([0-9]),?)+ $";

        UnrealInstancesAccessor accessor = new UnrealInstancesAccessor();
        Match matchList = Regex.Match(accessPattern, pattern);
        if (matchList.Success)
        {
            foreach (var index in matchList.Groups[1].Captures)
            {
                accessor.Instances.Add(_instances[int.Parse(index?.ToString() ?? "")]);
            }
        }
        return accessor;
    }
}