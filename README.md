# Arma Reforger Parachute Framework

This project adds fully controllable parachutes to Arma Reforger in a way that makes it easy to add new/different ones. It only contains a single type of parachute at the moment, which is a glider, but the dynamic values give server owners the freedom to create/modify parachutes to their liking. You can find the mod [here](https://reforger.armaplatform.com/workshop/65930CB4CD0237B2-ParachuteFramework).  

This mod is **open source** and welcomes community improvements through pull requests and issues. The mod, especially the code, is definitely not anywhere close to being perfect so feel free to give feedback or help out by improving it yourself.  

How you implement this mod in your own server depends on how you want to use it. Adding the parachute item to the default loadout is the most easy way to do it.  

---

## Quick Start

```bash
git clone https://github.com/Alphaegen/ArmaReforgerParachutes.git
````

---

## How to contribute

1. Fork the repository on GitHub
2. Create a branch such as `feature‑lift‑tweak`
3. Commit changes in small, logical steps with clear messages
4. Push and open a **Pull request** against `main`
5. A maintainer (probably me) will review and may request changes
6. When the checks pass your patch is merged

Please open an **Issue** first if you plan a large change so we can discuss direction.

### Guidelines

* Keep line endings and indentation consistent with the existing code
* Document new public properties in the header comment
* Test in multiplayer with at least one remote client
* Reference the issue you are fixing in the pull request description

---

## Code overview

### `ParachuteComponent` (player side)

* Lives on each `PlayerController`
* Listens for the **Jump** key and decides whether deployment is allowed
* Spawns `ParachuteDeployedEntity`, moves the character into its compartment, then hands ownership to the player
* Tracks landing speed to trigger leg break or fatal impact on the server
* Cleans up the parachute entity after exit or death
* Has component settings you can change on the `PlayerController` like fatal velocity

### `ParachuteDeployedEntity` (in‑world vehicle)

* Provides physics based flight

  * Pitch and roll torques, bank‑to‑turn logic, glide and flare handling, wind impulse
  * Auto‑level PD controller keeps the canopy upright
* Sends movement snapshots at a tunable rate with delta gating and distance culling

  * Non owners blend packets for smooth motion and hard snap when error grows large
* Optional debug overlay that shows velocity, forces and control input
* Can be destroyed by damage which forces the occupant to exit
* Exposes many sliders in the editor so mission makers can tune performance, flight model and network cost without touching code
* Has component settings you can change on the prefab like physics values or debugging

---

## Licence

Released under the GNU General Public License v3.0 licence.

## Happy jumping!