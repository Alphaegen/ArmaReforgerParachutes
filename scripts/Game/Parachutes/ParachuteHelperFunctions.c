class ParachuteHelperFunctions {
	static ParachuteComponent GetPlayerControllerComponentFromSlotOwner(InventoryStorageSlot slot) {
		if (!slot)
			return null;
		
		if (!SCR_ChimeraCharacter.Cast(slot.GetStorage().GetOwner()))
			return null;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(slot.GetStorage().GetOwner());
		if (playerId < 0)
			return null;
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return null;
		
		return ParachuteComponent.Cast(pc.FindComponent(ParachuteComponent));
	}
}