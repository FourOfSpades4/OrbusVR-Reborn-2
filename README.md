# OrbusVR Reborn^2

This is a Mod for [OrbusVR](https://orbusvr.com/), a VR game that ended developer support in May 2023. It adds a new mode for one of the 2 raids, Guild City Expert Mode. 
</br>

### New Mechanics
Boss 1: Clockwork Hunter
- The Laser Beam now damages **ALL** players inside it for 20% of their max health per second (excluding damage resistance)
- During Lightning Strike, all players who are not inside the Laser will instantly die after Clockwork Hunter's shield is destroyed.

Boss 2: Broken Knight
- Broken Knight now deals 3x as much damage, spread amongst the Tank and 2 closest players to the Tank.
  - If there are no players within 10m of the Tank, the tank will take the full 3x damage.  
- This effect also applies to the Circle Spark mechanic.

Boss 3: Pot Tank
- During the Wave Phase, Pot Tank will gain a shield that blocks 500 hits of damage.  
  - If the shield is not destroyed before the phase ends, he will explode, killing the party.

Boss 4: Seamstress
- During Main Phase, all players will take 10,000 damage per second.  
- During Add Phase, all healing done to players is turned into damage.

Boss 5: Valusia Warrior
- Every 20 seconds, 2 players are outlined Blue and 2 players are outlined Purple.
  - After 20 seconds, the Blue Players must be within 5m of each other, and not within 5m of the Purple Players.
  - The same applies for the Purple Players
  - If players fail, they will die after 20 seconds. Otherwise, they will lose their colored outline and another set will be chosen.

### How does it work?
There are two types of clients: the Master Client and the regular Clients. The Master Client is a single designated player who is going to inform the custom server about important party-wide events, such as damage being recieved or dealt, entering combat, exiting combat, ect. The regular Clients are designed to update the server with their specific actions that cannot be seen by the Master Client and display events when told to by the Master Client.   
</br>
The project uses [MinHook](https://github.com/TsudaKageyu/minhook) a simply hooking library in order to execute OrbusVR's functions at any time in order to add functionality such as changing health bar values, colors, spawning objects, and more. It also can detect when the same functions are called to update the server when events such as using Paladin's Shield to tell the Server that they should have damage resistance.  
</br>

### Is it good?
No, but it was a ton of fun. It was my first experience in coding server communcations, and honestly, it's pretty terrible. If I were to remake this, I would use [BepInEx](https://github.com/BepInEx/BepInEx) to make it easier to install, better Server / Client communication, and make the Server do most of the calculations instead of the clients. I learned a ton during this project, and will probably do a project attempting to create a multiplayer game myself now that I know way more about how computers communicate.   
Edit: [I'm now working on it.](https://github.com/FourOfSpades4/MUD)
