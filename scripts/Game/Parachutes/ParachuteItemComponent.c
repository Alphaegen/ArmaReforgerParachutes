class ParachuteItemComponentClass: ScriptComponentClass {}
class ParachuteItemComponent: ScriptComponent {
		
	[Attribute("", UIWidgets.ResourceNamePicker, "Parachute Prefab", "et")]
	protected ResourceName m_ParachutePrefab;
	protected bool m_ParachuteUsed;
	
	void SetParachuteUsed() {
		m_ParachuteUsed = true;
	}
	
	bool GetParachuteUsed() {
		return m_ParachuteUsed;
	}

	ResourceName GetParachutePrefab() { return m_ParachutePrefab; }

	override void OnPostInit(IEntity owner) {
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}

	// parachutePrefab should always be set for parachute to work
	override void EOnInit(IEntity owner) {
		if (m_ParachutePrefab == "") {
			Print("ERROR: ParachuteItemComponent is missing its prefab! Skipping parachute registration.");
			return;
		}
		
		InventoryItemComponent m_item = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (!m_item)
			return;

		m_item.m_OnParentSlotChangedInvoker.Insert(OnParentSlotChanged);
	}
	
	// If player removes item or changes it to another slot, stop any parachute logic
	void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot) {
		if (oldSlot) {
			ParachuteComponent parachuteComponent = ParachuteHelperFunctions.GetPlayerControllerComponentFromSlotOwner(oldSlot);
			if (parachuteComponent) {
				parachuteComponent.ClearParachuteItem();
			}
		}
		
		if (newSlot) {
			ParachuteComponent parachuteComponent = ParachuteHelperFunctions.GetPlayerControllerComponentFromSlotOwner(newSlot);
			if (parachuteComponent) {
				parachuteComponent.SetParachuteItem(this);
			}
		}
	}
}