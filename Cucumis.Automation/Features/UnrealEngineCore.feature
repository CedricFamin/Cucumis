Feature: Map Loading

	Scenario: Load Game
		Given start an unreal editor instance
		And wait until main menu is shown
		When Load Start Game
		Given wait 60 seconds
	
	@Multiplayer @2_Players
	Scenario: Load Multiple Game
		Given start an unreal editor instance
		And start an unreal editor instance
		And wait until main menu is shown
		When instance 0 Load Start Game
		Given wait 10 seconds
		When instance 1 Load Start Game
		Given wait 10 seconds
		
	
	Scenario Outline: Load all maps
		Given start an unreal editor instance
		Then travel to <map>
		Then server travel to <map>
		Then instance 0 travel to <map>
		Then all travel to <map>

		Examples:
		  | map            |
		  | VerticalSlice  |
		  | TechPlayground |