# Keylogger
> By **Oorth**
<pre align="center">

88  dP 888888 Yb  dP 88      dP"Yb   dP""b8  dP""b8 888888 88""Yb 
88odP  88__    YbdP  88     dP   Yb dP   `" dP   `" 88__   88__dP 
88"Yb  88""     8P   88  .o Yb   dP Yb  "88 Yb  "88 88""   88"Yb  
88  Yb 888888  dP    88ood8  YbodP   YboodP  YboodP 888888 88  Yb 
</pre>


## !! Disclaimer !!
This Keylogger is intended solely for ethical security research and educational purposes. Its use is restricted to systems you own or have explicit authorization to test. Deploying this keylogger on unauthorized systems is illegal and carries severe legal and ethical consequences and this is currently under development too.
>If you break the law, that's your problem, not mine. But if you manage to take down a major corporation, I expect a cut of the profits.

## Overview
Hi, This is a highly advanced keylogger, with very complex structure and evasion techniques which makes it undetectable.
This keylogger provides 3 types of information from the target machine ->
   1) Keystrokes
   2) Active Window
   3) Mouse click co-ordinates

## Working
This keylogger is staged in nature :)
soo it downloads each of the above mentioned features as 3 different .dlls from a server straight to your " memory " not the disk :)

Yes, the malicious .dlls never touches the disk and gets loaded and executed directly from the memory !!
This instantly bypasses any disk scans
> Neat right?
```markdown
This Keylogger uses techniques such as ->
>   1) Dynamic function linking               [To avoid entries in the import tables]
>   2) Various Obsfuscation techniques
>   3) Loading dlls from memory               [as discussed earlier]
>   4) Anti-debugging techniques              [Under development]
>   5) Custom Networking protocol
>   6) Remote command and control
```

This keylogger is controled via a C2 server so it never makes any direct connection between the target and the attacker.
To command the keylogger you will need the console which can be found here https://github.com/Oorth/Remote-Access-Trojan-Windows

The keylogger itself doesnt contain any networking code, so it requires the network_lib.dll which is bundled together.
> ok soo this keylogger was designed as a part of a RAT which has a custom payload delivery mechanism, thats why it uses a .dll for networking. If you want to use it standalone you will also need to put the network_lib.dll via your delivery mechanism.
>
> ~~Good luck with that~~

This keylogger currently lacks Debugger evasion abilities, BUT I am working on it here -> https://github.com/Oorth/Debugger_Evasion
once that is done, Debugger evasion abilities will be implemented here.
> working hard on it (⌐■_■)

## Requirements:
### Target: 
  network_lib.dll (which is bundled together)
### Attacker:
  console.exe (can be found here -> https://github.com/Oorth/Remote-Access-Trojan-Windows)

## Usage:
you can just use the pre compiled .exe but if you want to compile it then
```markdown
  cl /EHsc .\keylogger.cpp .\MemoryModule.c /link ws2_32.lib user32.lib /OUT:keylogger.exe
```
and each individual module can be compiled by
```markdown
  cl /EHsc /LD .\keylog_k_lib.cpp /link User32.lib                 !! Replace the "k" with appropriate alphabet !!
```

## The End
So people have fun stay safe, If you have further ideas or suggestions go on I am all ears. Also at this point just use the whole RAT, link provided everywhere in this readme.
